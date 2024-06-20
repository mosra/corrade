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

"""             Macro for declaring corrade dynamic plugin            """
"""                                                                   """
""" NOTE: currently does not work with static builds correctly as     """
""" shared global symbols are not correctly resolved and each binary  """
""" keeps its own copy internally.                                    """
"""                                                                   """
""" Arguments:                                                        """
"""   - name                final library label                       """
"""   - subdir              similar to cmake install dirs, but with   """
"""                         relative package subpath instead because  """
"""                         bazel does not deal with absolute paths   """
"""                         by design                                 """
"""   - metadata_file       same as in cmake                          """
"""   - override_suffix     overrides default shared object suffix    """
"""                         to your liking                            """
"""   - **kwargs            everything else gets passed to resulting  """
"""                         cc_binary inside macro                    """
def dynamic_plugin(name, subdir, metadata_file = None, override_suffix = "",
                   **kwargs):
    srcs = kwargs.pop("srcs", default = [])
    copts = kwargs.pop("copts", default = [])
    deps = kwargs.pop("deps", default = [])
    local_defines = kwargs.pop("local_defines", default = [])

    if kwargs.pop("hdrs", default = None) != None:
        fail(
            "corrade_dynamic_plugin hdrs field is prohibited " +
            "as it uses cc_binary, put everything in srcs"
        )
    if kwargs.pop("linkshared", default = None) != None:
        fail(
            "corrade_dynamic_plugin is a linkshared cc_binary, " +
            "do not touch this field"
        )

    conf = "%s_dynamic_plugin_conf" % name
    if metadata_file:
        _copy_conf(
            name = conf,
            plugin_name = name,
            metadata = metadata_file,
        )

    lib = "%s_dynamic_plugin" % name
    native.cc_binary(
        name = lib,
        local_defines = local_defines + ["CORRADE_DYNAMIC_PLUGIN"],
        copts = copts + ["-std=c++11"],
        srcs = srcs,
        linkshared = True,
        deps = deps + ["@corrade//src/Corrade/PluginManager:headers"],
        **kwargs,
    )

    _assemble_plugin(
        name = name,
        binary = ":%s" % lib,
        metadata = ":%s" % conf if metadata_file else None,
        subdir = subdir,
        override_suffix = override_suffix,
    )

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
    dir = ctx.attr.subdir
    name = ctx.label.name
    bin = ctx.file.binary
    conf = ctx.file.metadata
    override = ctx.attr.override_suffix
    ext = bin.extension if not override else override

    out_bin = ctx.actions.declare_file("%s/%s.%s" % (dir, name, ext))

    inputs = [(bin, out_bin)]
    outputs = [out_bin]
    if conf:
        out_conf = ctx.actions.declare_file("%s/%s.%s" % (dir, name, conf.extension))
        inputs.append((conf, out_conf))
        outputs.append(out_conf)

    for _in, _out in inputs:
        ctx.actions.run_shell(
            mnemonic = "CorradeAssembleDynamicPlugin",
            inputs = [_in],
            outputs = [_out],
            command = "cp '{}' '{}'".format(
                _in.path,
                _out.path,
            ),
        )

    return [
        DefaultInfo(
            files = depset([out_bin]),
            runfiles = ctx.runfiles(
                transitive_files = depset(outputs),
            )
        ),
    ]

_assemble_plugin = rule(
    doc = (
        "Internal rule to strip automatic lib* prefixes and " +
        "assemble everything together in a separate directory."
    ),
    attrs = {
        "binary": attr.label(mandatory = True, allow_single_file = True),
        "subdir": attr.string(mandatory = True),
        "metadata": attr.label(allow_single_file = True),
        "override_suffix": attr.string(),
    },
    implementation = _assemble_plugin_impl,
)
