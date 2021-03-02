load(":configure_header.bzl", "configure_header_toolchain")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "configure_header",
    srcs = ["configure_header.cpp"],
)

configure_header_toolchain(
    name = "corrade_configure_header",
    executable = ":configure_header",
)

toolchain_type(name = "configure_header_toolchain_type")

toolchain(
    name = "configure_header_toolchain",
    toolchain = ":corrade_configure_header",
    toolchain_type = ":configure_header_toolchain_type"
)


