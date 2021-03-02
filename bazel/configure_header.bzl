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

    return [
        DefaultInfo(
            files = depset([out_file]),
            runfiles = ctx.runfiles(transitive_files = depset([out_file]))
        )
    ]

_configure_header = rule(
    attrs = {
        "src": attr.label(mandatory = True, allow_single_file=True),
        "output": attr.string(mandatory = True),
        "defines": attr.string_dict(mandatory = True),
        "deps": attr.label_list(allow_files=True),
    },
    implementation = _configure_header_impl,
    toolchains = [
        "@corrade//bazel:configure_header_toolchain_type",
    ],
)

def configure_header(name, src, output, defines, deps):
    n = "__{}_h".format(name);
    _configure_header(
        name = n,
        src = src,
        output = output,
        defines = defines,
        deps = deps,
    )

    # Note: this should instead be provided with CcInfo
    native.cc_library(
        name = name,
        hdrs = [":{}".format(n)],
        include_prefix = ".",
        visibility = ["//visibility:public"],
    )
