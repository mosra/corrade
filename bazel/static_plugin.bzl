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
load(":compile_resource.bzl", "compile_resource")

"""             Macro for declaring corrade static plugin             """
""" Arguments:                                                        """
"""   - name                final library label                       """
"""   - metadata_file       same as in cmake                          """
"""   - resources           labels/files to pass to resource compiler """
"""   - **kwargs            everything else gets passed to resulting  """
"""                         cc_library inside macro                   """
def static_plugin(name, metadata_file = None, resources = [], **kwargs):
    conf = "%s_static_plugin_conf" % name
    resource = "%s_static_plugin_resource" % name

    srcs = kwargs.pop("srcs", default = [])
    copts = kwargs.pop("copts", default = [])
    deps = kwargs.pop("deps", default = [])
    local_defines = kwargs.pop("local_defines", default = [])
    # use alwayslink by default
    alwayslink = kwargs.pop("alwayslink", default = True)

    if kwargs.pop("linkstatic", default = None) != None:
        fail("corrade_static_plugin is static, do not touch this field")

    _write_conf(
        name = conf,
        plugin_name = name,
        metadata = metadata_file,
    )

    rc_deps = []
    rc_deps += resources
    if metadata_file:
        rc_deps += [metadata_file]

    compile_resource(
        name = resource,
        conf = ":%s" % conf,
        deps = rc_deps,
        override_name = name,
    )

    native.cc_library(
        name = name,
        copts = copts + ["-std=c++11"],
        linkstatic = True,
        alwayslink = alwayslink,
        srcs = srcs + [":%s" % resource],
        deps = deps + ["@corrade//src/Corrade/PluginManager:headers"],
        local_defines = local_defines + ["CORRADE_STATIC_PLUGIN"],
        **kwargs
    )

# NOTE: requires bash; here we assume that anyone smart enough to use bazel
# on windows have also read their manual on how to install it with MSYS2
def _write_conf_impl(ctx):
    name = ctx.attr.plugin_name
    conf_content = "group=CorradeStaticPlugin_%s\n" % name

    out = ctx.actions.declare_file(
        "resources_%s.conf" % name,
        sibling = ctx.file.metadata if ctx.attr.metadata else None,
    )
    retract_path = "/".join([".." for s in out.dirname.split("/")])

    if ctx.attr.metadata:
        metadata = ctx.file.metadata
        conf_content += "[file]\n"
        conf_content += "filename=\"%s/%s\"\n" % (retract_path, metadata.path)
        conf_content += "alias=%s.%s" % (name, metadata.extension)

    ctx.actions.run_shell(
        mnemonic = "CorradeWriteConf",
        inputs = [],
        outputs = [out],
        command = "echo -e '{}' > '{}'".format(
            conf_content,
            out.path,
        ),
    )

    return [
        DefaultInfo(
            files = depset([out]),
            runfiles = ctx.runfiles(transitive_files = depset([out]))
        ),
    ]

_write_conf = rule(
    doc = ("Internal rule to write .conf file."),
    attrs = {
        "plugin_name": attr.string(mandatory = True),
        "metadata": attr.label(allow_single_file = True),
    },
    implementation = _write_conf_impl,
)
