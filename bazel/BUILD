load(":configure_file.bzl", "configure_file_toolchain")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "configure_file",
    srcs = ["configure_file.cpp"],
)

configure_file_toolchain(
    name = "corrade_configure_file",
    executable = ":configure_file",
)

toolchain_type(name = "configure_file_toolchain_type")

toolchain(
    name = "configure_file_toolchain",
    toolchain = ":corrade_configure_file",
    toolchain_type = ":configure_file_toolchain_type"
)


