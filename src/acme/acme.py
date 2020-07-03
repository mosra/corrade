#!/usr/bin/env python

#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020 Vladimír Vondruš <mosra@centrum.cz>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included
#   in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.
#

import argparse
import codecs
import re
import os
import logging
import subprocess

from typing import List, Tuple

alone_in_parentheses_rx = re.compile(r'\((?P<inside>0|1|!?defined\((?P<name>[^)]+)\))\)')
zero_and_something_rx = re.compile(r'0 && (0|1|!?defined\([^)]+\)|!?\()')
something_and_zero_rx = re.compile(r'(0|1|!?defined\([^)]+\)|\)) && 0')
one_or_something_rx = re.compile(r'1 \|\| (0|1|!?defined\([^)]+\)|!?\()')
something_or_one_rx = re.compile(r'(0|1|!?defined\([^)]+\)|\)) \|\| 1')
alone_defined_rx = re.compile(r'^(?P<not>!)?defined\((?P<name>[^)]+)\)$')

def normalize_expression(expression) -> Tuple[str, str]:
    match = alone_defined_rx.match(expression)
    if match:
        return 'ifndef' if match.group('not') else 'ifdef', match.group('name')
    return 'if', expression

def simplify_expression(what, expression, forced_defines = {}):
    assert what in ['if', 'ifdef', 'ifndef', 'elif']

    # Denormalize the shorthand expression
    if what == 'ifdef':
        what, expression = 'if', 'defined({})'.format(expression)
    if what == 'ifndef':
        what, expression = 'if', '!defined({})'.format(expression)

    # Go through all forced defines and replace them with their values
    for name, enabled in forced_defines.items():
        if name not in expression: continue

        expression = expression.replace('defined({})'.format(name), '1' if enabled else '0')

    # Now comes the ugly part, simplify as long as there's something
    modified = True
    while modified:
        modified = False # Nothing done in this round yet

        # Basic replacements
        for find, replace in [('!0', '1'),
                              ('!1', '0'),
                              # All these have to be after `!0` / `!1` so e.g.
                              # `!0 || b` doesn't get # simplified to `!b`
                              ('0 || ', ''),
                              (' || 0', ''),
                              ('1 && ', ''),
                              (' && 1', '')]:
            if expression.find(find) != -1:
                expression = expression.replace(find, replace)
                modified = True

        # defined() / 0 / 1 alone in parentheses, remove parentheses
        match = alone_in_parentheses_rx.search(expression)
        if match:
            expression = expression[:match.start()] + match.group('inside') + expression[match.end():]
            modified = True

        # If the above replacements modified something, restart to avoid e.g.
        # `!0 && (something)` getting reduced by the "zero and something" rule
        # below
        if modified: continue

        # 0 && something or something && 0, replace with just 0
        # 1 || something or something || 1, replace with just 1
        for rexp, repl in [(zero_and_something_rx, '0'), (something_and_zero_rx, '0'),
                           (one_or_something_rx, '1'), (something_or_one_rx, '1')]:
            match = rexp.search(expression)
            if match:
                # The "something" is a parenthesised expression after, go to
                # the right and eat everything in matching parentheses
                if match.group(1) in ['(', '!(']:
                    level = 1
                    for i in range(match.end(), len(expression)):
                        c = expression[i]
                        if c == '(': level = level + 1
                        elif c == ')':
                            level = level - 1
                            assert level >= 0
                            if level == 0:
                                expression = expression[:match.start()] + repl + expression[i + 1:]
                                break

                # The "something" is a parenthesised expression before, go to
                # the left and eat everything in matching parentheses
                elif match.group(1) == ')':
                    level = 1
                    for i in range(match.start(), 0, -1):
                        c = expression[i - 1]
                        if c == ')': level = level + 1
                        elif c == '(':
                            level = level - 1
                            assert level >= 0
                            if level == 0:
                                # Eat also the negation, if there
                                if i > 1 and expression[i - 2] == '!':
                                    i = i - 1
                                expression = expression[:i - 1] + repl + expression[match.end():]
                                break

                # Simple version
                else:
                    expression = expression[:match.start()] + repl + expression[match.end():]

                modified = True

    if expression == '0':
        result = False
    elif expression == '1':
        result = True
    else:
        result = None # keep it

    # Renormalize back to ifdef if the expression is simple enough
    if what == 'if': what, expression = normalize_expression(expression)

    return result, what, expression

def sort_includes(includes: List[str]) -> List[str]:
    system, local = [], []
    for i in sorted(includes):
        if i.startswith('#include <'): system += [i]
        elif i.startswith('#include "'): local += [i]
        else: assert False # pragma: no cover
    return (system + ["\n"] + local) if local and system else (system + local)

copyright_email_rx = re.compile(r'<[^@]+@[^>]+>')
copyright_year_rx = re.compile(r'\d\d\d\d')

def sort_copyrights(copyrights: List[str]) -> List[str]:
    email_dict = {}
    output = []

    # Go through all copyright, the oldest first
    for copyright in sorted(copyrights):
        match = copyright_email_rx.search(copyright)
        assert match
        email = match.group(0)

        # If the e-mail is not in the dict yet, add it there with all years it
        # covers
        years = set([int(i) for i in copyright_year_rx.findall(copyright)])
        if email not in email_dict:
            email_dict[email] = (years, len(output))
            output += [copyright]

        # If given mail is in the dict already, check that all years are there
        # as well
        else:
            # If all years are there, nothing left to do
            if years <= email_dict[email][0]: continue

            # If we have a superset of the years, use this copyright line
            # instead
            if years > email_dict[email][0]:
                output[email_dict[email][1]] = copyright
                email_dict[email] = (years, email_dict[email][1])
                continue

            # Otherwise complain
            raise ValueError("First copyright found for {} is missing years {}, supply an explicit combined copyright instead".format(email, repr(years - email_dict[email][0])))

    return output

include_rx = re.compile(r'^(?P<include>#include (?P<quote>["<])(?P<file>[^">]+)[">]).*?$')
preprocessor_rx = re.compile(r'^(?P<indent>\s*)#(?P<what>ifdef|ifndef|if|else|elif|endif)\s*(?P<value>[^\n]*?[^\s]?)(?P<comment>\s*/[/*].*)?$')
define_rx = re.compile(r'\s*#(?P<what>define|undef) (?P<name>[^\s]+)\s*$')
linecomment_rx = re.compile(r'^\s*(/\*.*\*/|//.*)?\s*$')
copyright_rc = re.compile(r'^\s+Copyright © \d{4}.+$')
blockcomment_start_rx = re.compile(r'^\s*/\*.*\s*$')
blockcomment_end_rx = re.compile(r'^\s*.*\*/\s*$')
acme_pragma_rx = re.compile(r'^#pragma\s+ACME\s+(?P<what>[^\s]+)\s*(?P<value>[^\s]?.*)\s*$')

def acme(toplevel_file, output) -> List[str]:
    base_directory = os.path.dirname(toplevel_file)

    write_comments = True
    paths = []
    local_include_prefixes = []
    local_includes_noexpand = set()
    all_includes = set()
    new_includes = []
    copyrights = set()
    parsed_files = set()
    forced_defines = {}
    revision_commands = {}
    stats_commands = {}
    def parse(file, level):
        nonlocal write_comments, paths, local_include_prefixes, all_includes, new_includes, copyrights, parsed_files, forced_defines, revision_commands, stats_commands

        logging.info("%sParsing file %s...", ' '*level, file)

        # Mark the file as parsed. Doing it at the beginning so accidental
        # circular includes are caught.
        parsed_files.add(file)

        # Output line buffer + line buffer for includes (which need to be
        # inserted before the actual output, but with their order preserved)
        includes_out = []
        out = []
        in_comment = False
        comment_buffer = []
        multiline_copyright: str = None
        # Preprocessor branch stack. First element of each item is True if
        # this is a real node, False if this is a dummy for an #elif. The leaf
        # node is never False, all False nodes get popped at an #endif. Second
        # element is True if contents of current branch should be printed,
        # False if not and None if the preprocessor branching should be kept
        # verbatim. There's always at least one element, using a two-element
        # list instead of a tuple so I can modify the entries
        branch_stack = [[True, True, 0]]

        bline: bytes
        with open(file, 'rb') as f:
            # We don't handle BOMs, but let the user know that clearly
            bom = f.read(len(codecs.BOM_UTF8))
            if bom == codecs.BOM_UTF8:
                raise ValueError("{} contains a BOM, this is ABSOLUTELY unacceptable".format(file))
            f.seek(0)

            for bline in f:
                line: str = bline.decode('utf-8')

                # Buffer the comments and put them in the output at the end
                if in_comment:
                    # End of the comment, write the buffered contents if they
                    # were not thrown away and if we're not in a disabled
                    # preprocessor branch
                    if blockcomment_end_rx.match(line):
                        assert not multiline_copyright

                        in_comment = False
                        if branch_stack[-1][1] is not False and comment_buffer:
                            out += comment_buffer
                            out += [line]
                            comment_buffer = []

                    # Process the comments further only if we're not in a
                    # disabled preprocessor branch
                    elif branch_stack[-1][1] is not False:
                        # Continuation of a multiline copyright
                        if multiline_copyright:
                            multiline_copyright += line

                            # Final line of a multiline copyright, add it
                            if line.rstrip().endswith('>'):
                                copyrights.add(multiline_copyright)
                                multiline_copyright = None

                        # License block -- extract just copyrights
                        elif copyright_rc.match(line):
                            comment_buffer = []

                            # Add a complete copyright line to the global list
                            if line.rstrip().endswith('>'): copyrights.add(line)

                            # If a multi-line copyright (ending either with a
                            # `,` or a year), wait for next time
                            else: multiline_copyright = line

                        # Otherwise add to the buffer. The buffer is always at least
                        # one line (added when comment start matches), if not then it
                        # means we don't want it (license block, for example)
                        elif comment_buffer: comment_buffer += [line]

                    continue

                # Single-line comment or an empty line
                if linecomment_rx.match(line):
                    # If this is an {{include}} placeholder, finalize the
                    # previous set of includes and open a new one so the new
                    # includes are added to the new placeholder
                    if line.strip() == '// {{includes}}':
                        new_includes += [set()]

                    # Add it only if we're not in a disabled preprocessor
                    # branch and it's either a non-empty line (and comments are
                    # not disabled) or an empty line that's not first in the
                    # file and there's not more than one following each other
                    if branch_stack[-1][1] is not False and ((line.strip() and write_comments) or line.strip() == '// {{includes}}' or (not line.strip() and out and out[-1].strip())):
                        out += [line]
                    continue

                # Start of a multi-line comment
                match = blockcomment_start_rx.match(line)
                if match and '*/' not in line:
                    assert not in_comment
                    assert not comment_buffer

                    in_comment = True

                    # Add the comment only if we're not in a disabled
                    # preprocessor branch and comments are enabled
                    if branch_stack[-1][1] is not False and write_comments:
                        comment_buffer = [line]
                    continue

                # Preprocessor branch
                match = preprocessor_rx.match(line)
                if match:
                    # Unmatched group is None, so use a string instead. Because
                    # we strip the value, we need to put back a space before
                    # the comment.
                    indent = match.group('indent') or ''
                    what = match.group('what')
                    value = match.group('value').strip()
                    comment = match.group('comment') or ''
                    if comment and not comment[0].isspace(): comment = ' ' + comment

                    # Leaf node should always be a real node
                    assert len(branch_stack) >= 1
                    assert branch_stack[-1][0] == True

                    if what in ['if', 'ifdef', 'ifndef', 'elif']:
                        push_value, what, value = simplify_expression(what, value, forced_defines)

                    if what in ['if', 'ifdef', 'ifndef']:
                        # If the parent node disabled visibility, this should
                        # not enable it back
                        if branch_stack[-1][1] is False: push_value = False
                        # Push a new node on the stack
                        branch_stack += [[True, push_value, len(out)]]
                        # If the new node doesn't affect visibility, print it
                        if branch_stack[-1][1] is None:
                            out += ['{}#{} {}{}\n'.format(indent, what, value, comment)]

                    elif what == 'elif':
                        assert len(branch_stack) >= 2
                        # Flip the condition for the else block, if the outer
                        # condition affected visibility, but only if the whole
                        # node isn't already disabled
                        if branch_stack[-1][1] is not None and branch_stack[-2][1] is not False:
                            branch_stack[-1][1] = not branch_stack[-1][1]
                        # If the parent node disabled visibility, this should
                        # not enable it back
                        if branch_stack[-1][1] is False: push_value = False

                        # If the outer branch didn't affect visibility and this
                        # one isn't either, put the processed elif to output.
                        # The outer branch then doesn't need to have its #endif
                        # written, so setting it to True.
                        if branch_stack[-1][1] is None and push_value is None:
                            branch_stack[-1][1] = True # TODO: false? does it matter?
                            out += ['{}#elif {}{}\n'.format(indent, value, comment)]
                        # If the outer branch didn't affect visibility and this
                        # does, put just else to output. That also means we
                        # need to print #endif after, so keep it at None.
                        elif branch_stack[-1][1] is None and push_value is not None:
                            out += ['{}#else{}\n'.format(indent, comment)]
                        # If the outer branch affected visibility and this
                        # does not, put just if (normalized) to output
                        elif branch_stack[-1][1] is not None and push_value is None:
                            out += ['{}#{} {}{}\n'.format(indent, *normalize_expression(value), comment)]
                        # Otherwise not printing anything, meaning both the
                        # outer node and the inner affect visibility
                        else:
                            assert branch_stack[-1][1] is not None and push_value is not None

                        # Turn the parent node into a #elif dummy and add a new
                        # node to handle this new if. We need to do it this way
                        # in order to remember the visibility status of the
                        # else block
                        branch_stack[-1][0] = False
                        branch_stack += [[True, push_value, branch_stack[-1][2]]]
                    elif what == 'else':
                        assert len(branch_stack) >= 2
                        # Put the line to output if the branch didn't affect
                        # visibility. If it did, flip the condition.
                        if branch_stack[-1][1] is None:
                            out += ['{}#else{}\n'.format(indent, comment)]
                        else:
                            branch_stack[-1][1] = not branch_stack[-1][1]
                        # But if the parent node disabled visibility, this
                        # should not enable it back
                        if branch_stack[-2][1] is False:
                            branch_stack[-1][1] = False
                    else:
                        assert what == 'endif'
                        assert len(branch_stack) >= 2
                        # Put the line to output if the branch didn't affect
                        # visibility
                        endif_written = False
                        if branch_stack[-1][1] is None:
                            endif_written = True
                            out += ['{}#endif{}\n'.format(indent, comment)]

                        # Remember the line no. of the `if` statement, so we
                        # can remove the whole thing below if it turns out to
                        # be empty
                        if_lineno = branch_stack[-1][2]
                        branch_stack.pop()

                        # There might be dummy #elif nodes above, drop all of
                        # them until there is a real one. Put an extra endif
                        # in all cases where needed.
                        while branch_stack[-1][0] is False:
                            if branch_stack[-1][1] is None:
                                out += ['{}#endif{}\n'.format(indent, comment)]
                            if_lineno = branch_stack[-1][2]
                            branch_stack.pop()
                        assert len(branch_stack) >= 1

                        # If the endif was written and there's nothing between
                        # the if and the endif, remove the whole thing.
                        if endif_written and if_lineno + 2 == len(out):
                            out.pop()
                            out.pop()

                    continue

                # We're in a preprocessor branch with disabled visibility,
                # ignore anything else that could be inside
                # TODO: still need to handle raw strings, ugh
                if branch_stack[-1][1] is False:
                    continue

                # Include
                match = include_rx.match(line)
                if match:
                    include:str = match.group('file')
                    is_local = match.group('quote') == '"'
                    # Local includes or includes from dependent projects, recurse
                    if (is_local or include.partition('/')[0] in local_include_prefixes) and not include in local_includes_noexpand:
                        # A header corresponding to an implementation file
                        if is_local and '/' not in include:
                            absolute_include = os.path.join(os.path.dirname(file), include)
                            if not os.path.exists(absolute_include): # pragma: no cover
                                logging.fatal("Can't find %s in %s", include, os.path.dirname(file))
                                assert False
                        # Something else
                        else:
                            for path in paths:
                                absolute_include = os.path.join(path, include)
                                if os.path.exists(absolute_include):
                                    break
                            else: # pragma: no cover
                                logging.fatal("Can't find %s in any of %s", include, paths)
                                assert False

                        # If given include file is not parsed yet, parse it
                        if absolute_include not in parsed_files:
                            # If this is the top-level parsed file, add the
                            # contents in-order. Otherwise prepend the include
                            # before this file contents to avoid nested include
                            # guards.
                            parsed_file = parse(absolute_include, level + 1)
                            if toplevel_file == file:
                                out += parsed_file
                            else:
                                includes_out += parsed_file

                    # System or local noexpand include.
                    else:
                        includeline = match.group('include') + '\n'

                        # Already spotted, don't do anything
                        if includeline in all_includes: continue

                        # If the include is wrapped in a preprocessor branch
                        # other than the include guard (i.e., it starts at a
                        # line > 0) and the branch is not disabled/enabled,
                        # then we need to keep it in place -- it might be a
                        # platform-specific thing.
                        if len(branch_stack) > 1 and branch_stack[-1][2] != 0 and branch_stack[-1][1] is None:
                            out += [includeline]

                        # Otherwise add it to the set of not-yet-written
                        # includes, it'll get written to the nearest preceding
                        # {{includes}} placeholder.
                        else:
                            if not new_includes:
                                logging.warning("Includes found before an {{includes}} placeholder, the resulting file will have them on the top")
                                new_includes += [set()]
                            new_includes[-1].add(includeline)

                        # In both cases, add it to the list of global includes
                        # to avoid including it more than once. This is done
                        # with the assumption that platform-specific includes
                        # always get wrapped in the preprocessor branch the
                        # same way.
                        # TODO: this might cause problems?
                        all_includes.add(includeline)
                    continue

                # Pragma
                match = acme_pragma_rx.match(line)
                if match:
                    what = match.group('what')
                    value = match.group('value')
                    if what == 'enable':
                        forced_defines[value] = True
                    elif what == 'disable':
                        forced_defines[value] = False
                    elif what == 'path':
                        paths += [os.path.join(base_directory, value)]
                    elif what == 'local':
                        local_include_prefixes += [value]
                    elif what == 'comments':
                        write_comments = value == 'on'
                    elif what == 'revision':
                        path, _, command = value.partition(' ')
                        revision_commands[path] = command.strip()
                    elif what == 'stats':
                        id, _, command = value.partition(' ')
                        stats_commands[id] = command.strip()
                    elif what == 'noexpand':
                        local_includes_noexpand.add(value)
                    else:
                        logging.warning("Unknown #pragma ACME %s %s", what, value)

                    continue

                # Preprocessor define -- if it's among the forced ones, ignore
                match = define_rx.match(line)
                if match and match.group('name') in forced_defines:
                    continue

                # Something else, copy verbatim to the output. Strip the
                # trailing comment, if requested
                if line.rstrip().endswith('*/'):
                    out += [line[:line.rindex('/*')].rstrip() + '\n']
                else:
                    out += [line]

        # Assert that the branch stack is correct
        assert len(branch_stack) == 1

        # Drop empty liness off the end
        while out and not out[-1].strip(): out.pop()

        # Return parsed lines, stuff from includes in front
        return includes_out + out

    lines = parse(toplevel_file, 0)

    # For each include placeholder put the corresponding includes there. If
    # there's none, put them on the top.
    if new_includes:
        i = 0
        while i != len(lines):
            if lines[i].strip() == '// {{includes}}':
                sorted_includes = sort_includes(new_includes[0])
                lines = lines[:i] + sorted_includes + lines[i + 1:]
                new_includes.pop(0)
                if not new_includes: break
                i = i + len(sorted_includes)
            else:
                i = i + 1
        else:
            # Warning already printed when new_includes was discovered to be empty
            lines = sort_includes(new_includes[0]) + lines

    # Find a copyright placeholder and put the copyrights there
    if copyrights:
        for i, line in enumerate(lines):
            if line.strip() == '{{copyright}}':
                lines = lines[:i] + sort_copyrights(copyrights) + lines[i + 1:]
                break
        else:
            logging.warning(" No {{copyrights}} placeholder found, ignoring found copyright statements")

    # If no custom revision fetch command was provided, add a default one. Then
    # for each revision path find a corresponding placeholder and replace it.
    if '*' not in revision_commands:
        revision_commands['*'] = 'git describe --dirty --always'
    for path, command in revision_commands.items():
        placeholder = '{{{{revision{}}}}}'.format('' if path == '*' else ':' + path)
        revision = None
        for i, line in enumerate(lines):
            if not placeholder in line: continue
            if not revision:
                # Find the file where the revision should be fetched
                if path == '*': cwd = os.path.dirname(os.path.realpath(toplevel_file))
                else:
                    for file in parsed_files:
                        realfile = os.path.realpath(file)
                        if path in realfile:
                            cwd = os.path.dirname(realfile)
                            break
                    else: # pragma: no cover
                        logging.fatal("No matching file found for expanding %s", placeholder)
                        assert False

                revision = subprocess.check_output(command, cwd=cwd, shell=True).decode('utf-8').strip()
            lines[i] = line.replace(placeholder, revision)

    # Create the output directory
    output_dir = os.path.dirname(output)
    if not os.path.exists(output_dir): os.makedirs(output_dir)

    # Perform some stats on file contents, passing them to stdin, running in
    # the (freshly created) output directory
    for id, command in stats_commands.items():
        placeholder = '{{{{stats:{}}}}}'.format(id)
        stats = None
        for i, line in enumerate(lines):
            if not placeholder in line: continue
            if not stats:
                stats = subprocess.check_output(command, cwd=output_dir, input=''.join(lines).encode('utf-8'), shell=True).decode('utf-8').strip()
            lines[i] = line.replace(placeholder, stats)

    logging.info('Writing %i lines to %s', len(lines), output)
    with open(output, 'w') as of:
        for line in lines:
            of.write(line)

if __name__ == '__main__': # pragma: no cover
    parser = argparse.ArgumentParser(
        description=r"""Creates single-header libraries from given top-level input file.""",
        epilog="""If output exists and is a directory, the file is saved inside with the same name as the input.""")
    parser.add_argument('file', help='top-level file')
    parser.add_argument('-o', '--output', help="output file or directory", default='output')
    parser.add_argument('--debug', help="verbose debug output", action='store_true')
    args = parser.parse_args()

    if args.debug:
        logging.basicConfig(level=logging.DEBUG)
    else:
        logging.basicConfig(level=logging.INFO)

    output = os.path.join(os.path.dirname(args.file), args.output)

    # If a directory, save the file inside
    if os.path.exists(output) and os.path.isdir(output):
        output = os.path.join(output, os.path.basename(args.file))

    # Otherwise assume it's a filename, create all directories above it if they
    # don't exist
    else: os.makedirs(os.path.dirname(output), exist_ok=True)

    acme(args.file, output)
