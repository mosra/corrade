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

# NOTE: requires bash; here we assume that anyone smart enough to use bazel
# on windows have also read their manual on how to install it with MSYS2
def _copy_conf_impl(ctx):
    name = ctx.attr.plugin_name
    metadata = ctx.file.metadata

    out = ctx.actions.declare_file("%s.%s" % (name, metadata.extension))

    ctx.actions.run_shell(
        mnemonic = "CorradeCopyConf",
        inputs = [metadata],
        outputs = [out],
        command = "cp '{}' '{}'".format(
            metadata.path,
            out.path,
        ),
    )

    return [
        DefaultInfo(
            files = depset([out]),
            runfiles = ctx.runfiles(transitive_files = depset([out]))
        ),
    ]

_copy_conf = rule(
    doc = ("Internal rule to copy .conf file."),
    attrs = {
        "plugin_name": attr.string(mandatory = True),
        "metadata": attr.label(mandatory = True, allow_single_file = True),
    },
    implementation = _copy_conf_impl,
)

def _assemble_plugin_impl(ctx):
    binary = ctx.file.binary

    out = ctx.actions.declare_file("%s.%s" % (ctx.label.name, binary.extension))
    ctx.actions.run_shell(
        mnemonic = "CorradeAssembleDynamicPlugin",
        inputs = [binary],
        outputs = [out],
        command = "cp '{}' '{}'".format(
            binary.path,
            out.path,
        ),
    )

    transitive = [out]
    if ctx.file.metadata:
        transitive.append(ctx.file.metadata)

    return [
        DefaultInfo(
            files = depset([out]),
            runfiles = ctx.runfiles(transitive_files = depset(transitive))
        ),
    ]

_assemble_plugin = rule(
    doc = ("Internal rule to de-prefix automatic lib* prefixes."),
    attrs = {
        "binary": attr.label(mandatory = True, allow_single_file = True),
        "metadata": attr.label(allow_single_file = True),
    },
    implementation = _assemble_plugin_impl,
)

# NOTE: doesn't work.
# There is something in bazel pathing logic / dynamic plugins logic
# that does not go together
def _dynamic_plugin(name, metadata_file = None, srcs = [], hdrs = [],
                   copts = [], deps = [], local_defines = [],
                   **kwargs):
    conf = "%s_dynamic_plugin_conf" % name
    if metadata_file:
        _copy_conf(
            name = conf,
            plugin_name = name,
            metadata = metadata_file,
        )

    native.cc_binary(
        name = "%s_dynamic_plugin" % name,
        local_defines = local_defines + ["CORRADE_DYNAMIC_PLUGIN"],
        copts = copts + ["-std=c++11"],
        srcs = srcs + hdrs,
        linkshared = True,
        linkstatic = False,
        deps = deps + ["@corrade//src/Corrade/PluginManager:headers"],
        **kwargs,
    )

    _assemble_plugin(
        name = name,
        binary = ":%s_dynamic_plugin" % name,
        metadata = ":%s" % conf if metadata_file else None,
    )
