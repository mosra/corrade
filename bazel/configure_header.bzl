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

def _configure_header_toolchain_impl(ctx):
    exe = ctx.attr.executable.files.to_list()[0]
    return platform_common.ToolchainInfo(executable = exe)

configure_header_toolchain = rule(
    attrs = {
        "executable": attr.label(
            mandatory=True,
            executable=True,
            cfg="host",
            allow_single_file=True),
    },
    implementation = _configure_header_toolchain_impl,
)

def _configure_header_impl(ctx):
    cfg_file = ctx.toolchains["@corrade//bazel:configure_header_toolchain_type"].executable
    in_file = ctx.attr.src.files.to_list()[0]
    out_file = ctx.actions.declare_file(ctx.attr.output)

    deps = [in_file]
    for target in ctx.attr.deps:
        deps += target.files.to_list()

    args = [
        "{}".format(in_file.path),
        "{}".format(out_file.path),
    ]

    for k, v in ctx.attr.defines.items():
        args.append("-D{}={}".format(k, ctx.expand_location(v)))

    ctx.actions.run(
        mnemonic = "ConfigureFileDotH",
        inputs = depset(deps),
        outputs = [out_file],
        use_default_shell_env = True,
        executable = cfg_file,
        tools = [cfg_file],
        arguments = args,
        execution_requirements = {"block-network": ""},
    )

    compilation_context = cc_common.create_compilation_context(
        headers = depset([out_file]),
        system_includes = depset(
            ["%s" % out_file.root.path] +
            ["{}/{}".format(out_file.root.path, i) for i in ctx.attr.includes]
        ),
        quote_includes = depset(
            (["%s" % out_file.dirname] if ctx.attr.local else []),
        ),
    )
    return [
        DefaultInfo(
            files = depset([out_file]),
            runfiles = ctx.runfiles(transitive_files = depset([out_file]))
        ),
        CcInfo(compilation_context = compilation_context),
    ]

configure_header = rule(
    attrs = {
        "src": attr.label(
            mandatory = True,
            allow_single_file=True,
            doc = ("Source .h.cmake file to configure"),
        ),
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
            allow_files=True,
            doc = ("Dependencies for $(location dep) expansion"),
        ),
        "includes": attr.string_list(
            doc = (
                "List of paths where the output .h file should be foundable. " +
                "This one is tricky as bazel plays difficult with file " +
                "locations, especially so if they are generated.\n" +
                "For generated files, these are different from repo root, " +
                "it will place them relative to execution root, where source " +
                "files will have a hard time looking it up.\n" +
                "Full path as-if it was in repo is set to work, and " +
                "additional values provided here will be appended to full " +
                "path to make it findable at those locations.\n" +
                "Note these are system includes."
            ),
        ),
        "local": attr.bool(
            default=False,
            doc = (
                "Controls whether the target is findable locally with " +
                "#include \"file.h\" or not. Default is off because it " +
                "makes every dependant be able to include it locally whether " +
                "they would be as-if in the same directory or not."
            ),
        ),
    },
    implementation = _configure_header_impl,
    toolchains = [
        "@corrade//bazel:configure_header_toolchain_type",
    ],
)
