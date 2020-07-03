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

import os
import shutil
import unittest

from acme import simplify_expression, sort_includes, sort_copyrights, acme

class ParseExpression(unittest.TestCase):
    def test_basic(self):
        self.assertEqual(simplify_expression('if', '0'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '1'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', 'SOMETHING'),
            (None, 'if', 'SOMETHING'))

    def test_normalization(self):
        self.assertEqual(simplify_expression('ifdef', 'FOO'),
            (None, 'ifdef', 'FOO'))
        self.assertEqual(simplify_expression('ifndef', 'FOO'),
            (None, 'ifndef', 'FOO'))
        self.assertEqual(simplify_expression('if', 'defined(FOO)'),
            (None, 'ifdef', 'FOO'))
        self.assertEqual(simplify_expression('elif', 'defined(FOO)'),
            (None, 'elif', 'defined(FOO)'))

    def test_basic_simplify(self):
        self.assertEqual(simplify_expression('if', '0 || 0'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '1 && 0'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '1 && 1'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '0 || 1'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '(1)'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '(!defined(FOO))'),
            (None, 'ifndef', 'FOO'))
        self.assertEqual(simplify_expression('elif', '!(!!0)'),
            (True, 'elif', '1'))

    def test_negation_precedence(self):
        self.assertEqual(simplify_expression('if', '!0 || defined(FOO)'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '!0 && defined(FOO)'),
            (None, 'ifdef', 'FOO'))

    def test_simplify(self):
        self.assertEqual(simplify_expression('if', '0 || (!defined(FOO) && 1)'),
            (None, 'ifndef', 'FOO'))
        self.assertEqual(simplify_expression('elif', '!(0 || defined(A)) && (defined(B) && 1)'),
            (None, 'elif', '!defined(A) && defined(B)'))
        self.assertEqual(simplify_expression('elif', '!(0 && defined(A)) && (defined(B) || 1)'),
            (True, 'elif', '1'))

    def test_simplify_nested_parentheses(self):
        self.assertEqual(simplify_expression('if', '0 && (defined(FOO) || (defined(BAR) && !defined(BAZ)))'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '(defined(FOO) || (defined(BAR) && !defined(BAZ))) && 0'),
            (False, 'if', '0'))
        self.assertEqual(simplify_expression('if', '1 || !(defined(FOO) || (defined(BAR) && !defined(BAZ)))'),
            (True, 'if', '1'))
        self.assertEqual(simplify_expression('if', '!(defined(FOO) || (defined(BAR) && !defined(BAZ))) || 1'),
            (True, 'if', '1'))

    def test_forced_defines(self):
        self.assertEqual(simplify_expression('elif', '!(0 || defined(A)) && (defined(B) && 1)', {'A': False}),
            (None, 'elif', 'defined(B)'))
        self.assertEqual(simplify_expression('elif', '!(0 || defined(A)) && (defined(B) && 1)', {'B': True}),
            (None, 'elif', '!defined(A)'))
        self.assertEqual(simplify_expression('elif', 'defined(CORRADE_NO_DEBUG) || defined(NDEBUG)', {'DOXYGEN_GENERATING_OUTPUT': False, 'CORRADE_NO_DEBUG': True}),
            (True, 'elif', '1'))
        # Crazy thing, combining all the awful corner cases from above
        self.assertEqual(simplify_expression('if', '!defined(CORRADE_NO_TWEAKABLE) && (defined(DOXYGEN_GENERATING_OUTPUT) || defined(CORRADE_TARGET_UNIX) || (defined(CORRADE_TARGET_WINDOWS) && !defined(CORRADE_TARGET_WINDOWS_RT)) || defined(CORRADE_TARGET_EMSCRIPTEN))', {'DOXYGEN_GENERATING_OUTPUT': False, 'CORRADE_NO_TWEAKABLE': True}),
            (False, 'if', '0'))

class SortIncludes(unittest.TestCase):
    def test_system(self):
        self.assertEqual(sort_includes([
            "#include <thread>\n",
            "#include <cstring>\n",
            "#include <string>\n"
        ]), [
            "#include <cstring>\n",
            "#include <string>\n",
            "#include <thread>\n"
        ])

    def test_local(self):
        self.assertEqual(sort_includes([
            "#include \"world.h\"\n",
            "#include \"hello.h\"\n"
        ]), [
            "#include \"hello.h\"\n",
            "#include \"world.h\"\n"
        ])

    def test_mixed(self):
        self.assertEqual(sort_includes([
            "#include <string>\n",
            "#include \"noexpand.h\"\n",
            "#include \"neither.h\"\n",
            "#include <cstring>\n"
        ]), [
            "#include <cstring>\n",
            "#include <string>\n",
            "\n",
            "#include \"neither.h\"\n",
            "#include \"noexpand.h\"\n"
        ])

class SortCopyrights(unittest.TestCase):
    def test_sort(self):
        self.assertEqual(sort_copyrights([
            "Copyright © 2015, 2016 John Doe <foo@bar>",
            "Copyright © 1997, 2003 Boo Bee <bar@bar>",
            "Copyright © 2018 Ho <hu@he>"
        ]), [
            "Copyright © 1997, 2003 Boo Bee <bar@bar>",
            "Copyright © 2015, 2016 John Doe <foo@bar>",
            "Copyright © 2018 Ho <hu@he>"
        ])

    def test_pick_earlier(self):
        self.assertEqual(sort_copyrights([
            "Copyright © 2015, 2016, 2018 John Doe <foo@bar>",
            "Copyright © 2018 John Doe <foo@bar>",
            "Copyright © 1997, 2003 Boo Bee <bar@bar>",
            "Copyright © 2016 John Doe <foo@bar>"
        ]), [
            "Copyright © 1997, 2003 Boo Bee <bar@bar>",
            "Copyright © 2015, 2016, 2018 John Doe <foo@bar>"
        ])

    def test_pick_longer(self):
        self.assertEqual(sort_copyrights([
            "Copyright © 2015 John Doe <foo@bar>",
            "Copyright © 2015, 2016, 2018 John Doe <foo@bar>"
        ]), [
            "Copyright © 2015, 2016, 2018 John Doe <foo@bar>"
        ])

    def test_extra_year(self):
        with self.assertRaises(ValueError) as context:
            sort_copyrights([
                "Copyright © 2015, 2016, 2018 John Doe <foo@bar>",
                "Copyright © 2016 John Doe <foo@bar>",
                "Copyright © 2016, 2017, 2019 John Doe <foo@bar>"
            ])

        self.assertIn('First copyright found for <foo@bar> is missing years {2017, 2019}', context.exception.args[0])

    def test_extra_year2(self):
        with self.assertRaises(ValueError) as context:
            sort_copyrights([
                "Copyright © 2016 John Doe <foo@bar>",
                # After this entry the set needs to get updated, otherwise
                # 2018 will get lost
                "Copyright © 2016, 2018 John Doe <foo@bar>",
                "Copyright © 2016, 2020 John Doe <foo@bar>"
            ])

        self.assertIn('First copyright found for <foo@bar> is missing years {2020}', context.exception.args[0])

class ParseFile(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        self.path = os.path.dirname(os.path.realpath(__file__))

        # Display ALL THE DIFFS
        self.maxDiff = None

    def setUp(self):
        if os.path.exists(os.path.join(self.path, 'output')): shutil.rmtree(os.path.join(self.path, 'output'))

    def run_acme(self, path, output_dir = None):
        test_dir = os.path.join(self.path, path)
        output_dir = output_dir or test_dir
        acme(os.path.join(test_dir, 'input.h'), os.path.join(output_dir, 'actual.h'))

        with open(os.path.join(test_dir, 'expected.h')) as f:
            expected_contents = f.read()
        with open(os.path.join(output_dir, 'actual.h')) as f:
            actual_contents = f.read()
        return actual_contents, expected_contents

    def test_bom(self):
        with self.assertRaises(ValueError) as context:
            self.run_acme('bom')
        self.assertIn('contains a BOM', context.exception.args[0])

    def test_comments(self):
        self.assertEqual(*self.run_acme('comments'))

    def test_includes(self):
        self.assertEqual(*self.run_acme('includes'))

    def test_includes_no_placeholder(self):
        self.assertEqual(*self.run_acme('no_copyright_include_placeholders'))

    def test_preprocessor(self):
        self.assertEqual(*self.run_acme('preprocessor'))

    def test_pragmas(self):
        self.assertEqual(*self.run_acme('pragmas'))

    def test_revision_stats(self):
        self.assertEqual(*self.run_acme('revision_stats', os.path.join(self.path, 'revision_stats/output')))
