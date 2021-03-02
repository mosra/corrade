workspace(name = "corrade")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Toolchains for docker-sandbox build
http_archive(
    name = "bazel_toolchains",
    sha256 = "1adf5db506a7e3c465a26988514cfc3971af6d5b3c2218925cd6e71ee443fc3f",
    strip_prefix = "bazel-toolchains-4.0.0",
    urls = [
        "https://github.com/bazelbuild/bazel-toolchains/releases/download/4.0.0/bazel-toolchains-4.0.0.tar.gz",
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-toolchains/releases/download/4.0.0/bazel-toolchains-4.0.0.tar.gz",
    ],
)

load("@bazel_toolchains//rules:rbe_repo.bzl", "rbe_autoconfig")
rbe_autoconfig(
    name = "rbe_ubuntu1804",
    env = {
        "ABI_LIBC_VERSION": "glibc_2.27",
        "ABI_VERSION": "clang",
        "BAZEL_COMPILER": "clang",
        "BAZEL_HOST_SYSTEM": "i686-unknown-linux-gnu",
        "BAZEL_TARGET_CPU": "k8",
        "BAZEL_TARGET_LIBC": "glibc_2.27",
        "BAZEL_TARGET_SYSTEM": "x86_64-unknown-linux-gnu",
        "CC": "clang",
        "CXX": "clang++",
        "CC_TOOLCHAIN_NAME": "linux_gnu_x86",
    },
    registry = "l.gcr.io",
    repository = "google/rbe-ubuntu18-04",
    digest = "sha256:48b67b41118dbcdfc265e7335f454fbefa62681ab8d47200971fc7a52fb32054",
)

# TODO: move to defs of some sort
register_toolchains("//bazel:configure_file_toolchain")
