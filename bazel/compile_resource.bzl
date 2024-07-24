#
#   This file is part of Corrade.
#
#   Copyright © 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016,
#               2017, 2018, 2019, 2020, 2021
#             Vladimír Vondruš <mosra@centrum.cz>
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

# NOTE: requires bash. here we assume that anyone smart enough to use bazel
# on windows have also read their manual on how to install it with MSYS2
def _compile_resource_impl(ctx):
    conf = ctx.file.conf
    name = ctx.label.name if ctx.attr.override_name == "" else ctx.attr.override_name

    out_depends = ctx.actions.declare_file("resource_%s.depends" % name)
    ctx.actions.run_shell(
        mnemonic = "CorradeCompileDepends",
        inputs = depset([conf]),
        outputs = [out_depends],
        command = "cp '{}' '{}'".format(
            conf.path,
            out_depends.path,
        ),
    )

    out_cpp = ctx.actions.declare_file("resource_%s.cpp" % name)
    ctx.actions.run(
        mnemonic = "CorradeCompileResource",
        inputs = depset(ctx.files.conf + ctx.files.deps),
        outputs = [out_cpp],
        executable = ctx.executable._tool,
        arguments = [
            name,
            conf.path,
            out_cpp.path,
        ],
        execution_requirements = {"block-network": ""},
    )

    outputs = depset([out_cpp, out_depends])

    return [
        DefaultInfo(
            files = outputs,
            runfiles = ctx.runfiles(transitive_files = outputs)
        ),
    ]

compile_resource = rule(
    doc = ("Rule for invoking corrade-rc, similar to corrade_add_resource()"),
    attrs = {
        "conf": attr.label(
            mandatory = True,
            allow_single_file = True,
            doc = ("Source .conf file to pass to corrade-rc"),
        ),
        "deps": attr.label_list(
            mandatory = True,
            allow_files = True,
            doc = (
                "Full list of files required to run this rule.\n" +
                "Note that unlike cmake, bazel requires those to be " +
                "declared explicitly."
            ),
        ),
        "override_name": attr.string(
            doc = (
                "Optional override for name argument of corrade-rc, " +
                "default name is taken from rule name."
            ),
        ),
        "_tool": attr.label(
            doc = ("Private, touch at your own risk."),
            executable = True,
            cfg = "host",
            allow_single_file = True,
            default = Label("@corrade//:corrade-rc"),
        ),
    },
    implementation = _compile_resource_impl,
)
