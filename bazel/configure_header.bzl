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

def _configure_header_impl(ctx):
    in_file = ctx.file.src
    out_file = ctx.actions.declare_file(ctx.attr.output)

    args = ctx.actions.args()
    args.add_all([in_file.path, out_file.path])
    args.add_all([
        # ctx.expand_location below resolves $(location //pkg:target) strings
        "-D{}={}".format(k, ctx.expand_location(v))
        for k, v in ctx.attr.defines.items()
    ])

    # NOTE: it would be amazing to use ctx.actions.expand_template instead
    # but it does not support wildcards and we do need them for stripping
    ctx.actions.run(
        mnemonic = "CorradeConfigureHeader",
        progress_message = "Configuring %s" % ctx.attr.output,
        inputs = depset([in_file]),
        outputs = [out_file],
        executable = ctx.executable._tool,
        arguments = [args],
        execution_requirements = {"block-network": ""},
    )

    return [
        DefaultInfo(
            files = depset([out_file]),
            runfiles = ctx.runfiles(transitive_files = depset([out_file]))
        ),
        CcInfo(
            compilation_context = cc_common.create_compilation_context(
                headers = depset([out_file]),
                system_includes = depset(["%s" % out_file.root.path]),
                quote_includes = (
                    depset(["%s" % out_file.dirname]) if ctx.attr.local else None
                ),
            ),
        )
    ]

configure_header = rule(
    doc = (
        "Rule for configuring .h.cmake headers\n" +
        "WARNING! Experimental, will change without notice\n" +
        "WARNING! Generated files have tricky lookup semantics in bazel " +
        "so look up `local` attribute for in-place include, and look up " +
        "@corrade//:Main for example of correct pathing for system includes."
    ),
    attrs = {
        "src": attr.label(
            mandatory = True,
            allow_single_file = True,
            doc = ("Source .h.cmake file to configure"),
        ),
        # NOTE: we can infer this from rule name instead
        # but this replicates CMake command better.
        "output": attr.string(
            mandatory = True,
            doc = ("Output .h file to produce"),
        ),
        "defines": attr.string_dict(
            mandatory = True,
            doc = (
                "Dict of defines key value pairs {\"key\": \"value\"} " +
                "the executable will look for ${key} matches to transform " +
                "into values. " +
                "It also will accept $(location //:target) patterns to expand " +
                "as per bazel rules, which works on files and directories too" +
                ", but for that you must also put said target in deps."
            ),
        ),
        "deps": attr.label_list(
            allow_files = True,
            doc = ("Dependencies for $(location dep) expansion"),
        ),
        "local": attr.bool(
            default = False,
            doc = (
                "Controls whether the target is findable locally with " +
                "#include \"file.h\" or not.\n" +
                "WARNING: This makes the header available globally " +
                "whether dependants would-be in the same folder or not.\n" +
                "Default is False.\n" +
                "This one is tricky as bazel plays difficult with file " +
                "locations for generated files, as they are not placed in " +
                "repo root, hence the need for additional custom lookup.\n"
            ),
        ),
        "_tool": attr.label(
            doc = ("Private, touch at your own risk."),
            executable = True,
            cfg = "host",
            allow_single_file = True,
            default = Label("@corrade//bazel:configure_header"),
        )
    },
    implementation = _configure_header_impl,
)
