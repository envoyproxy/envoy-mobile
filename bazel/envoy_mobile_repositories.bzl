load("@bazel_gazelle//:deps.bzl", "go_repository")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file", "http_jar")

def envoy_mobile_repositories():
    http_archive(
        name = "google_bazel_common",
        sha256 = "d8c9586b24ce4a5513d972668f94b62eb7d705b92405d4bc102131f294751f1d",
        strip_prefix = "bazel-common-413b433b91f26dbe39cdbc20f742ad6555dd1e27",
        urls = ["https://github.com/google/bazel-common/archive/413b433b91f26dbe39cdbc20f742ad6555dd1e27.zip"],
    )

    http_archive(
        name = "swift_flatbuffers",
        sha256 = "ffd68aebdfb300c9e82582ea38bf4aa9ce65c77344c94d5047f3be754cc756ea",
        build_file = "@envoy_mobile//bazel:flatbuffers.BUILD",
        strip_prefix = "flatbuffers-2.0.0",
        urls = ["https://github.com/google/flatbuffers/archive/refs/tags/v2.0.0.zip"],
    )

    upstream_envoy_overrides()
    swift_repos()
    kotlin_repos()
    android_repos()
    python_repos()

def upstream_envoy_overrides():
    # Workaround due to a Detekt version compatibility with protobuf: https://github.com/envoyproxy/envoy-mobile/issues/1869
    http_archive(
        name = "com_google_protobuf",
        patch_args = ["-p1"],
        patches = [
            "@envoy_mobile//bazel:protobuf.patch",
        ],
        sha256 = "d7371dc2d46fddac1af8cb27c0394554b068768fc79ecaf5be1a1863e8ff3392",
        strip_prefix = "protobuf-3.16.0",
        urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.16.0/protobuf-all-3.16.0.tar.gz"],
    )

    # Workaround old NDK version breakages https://github.com/envoyproxy/envoy-mobile/issues/934
    http_archive(
        name = "com_github_libevent_libevent",
        urls = ["https://github.com/libevent/libevent/archive/0d7d85c2083f7a4c9efe01c061486f332b576d28.tar.gz"],
        strip_prefix = "libevent-0d7d85c2083f7a4c9efe01c061486f332b576d28",
        sha256 = "549d34065eb2485dfad6c8de638caaa6616ed130eec36dd978f73b6bdd5af113",
        build_file_content = """filegroup(name = "all", srcs = glob(["**"]), visibility = ["//visibility:public"])""",
    )

    # Patch upstream Abseil to prevent Foundation dependency from leaking into Android builds.
    # Workaround for https://github.com/abseil/abseil-cpp/issues/326.
    # TODO: Should be removed in https://github.com/envoyproxy/envoy-mobile/issues/136 once rules_android
    # supports platform toolchains.
    http_archive(
        name = "com_google_absl",
        patches = ["@envoy_mobile//bazel:abseil.patch"],
        sha256 = "2e4ace2ed32a4ccfd29e856ad72b4fd1eae2ec060d3ba8646857fa170d6e8269",
        strip_prefix = "abseil-cpp-17c954d90d5661e27db8fc5f086085690a8372d9",
        # 2021-06-03
        urls = ["https://github.com/abseil/abseil-cpp/archive/17c954d90d5661e27db8fc5f086085690a8372d9.tar.gz"],
    )

    # This should be kept in sync with Envoy itself, we just need to apply this patch
    # Remove this once https://boringssl-review.googlesource.com/c/boringssl/+/37804 is in master-with-bazel
    http_archive(
        name = "boringssl",
        patches = ["@envoy_mobile//bazel:boringssl.patch"],
        sha256 = "579cb415458e9f3642da0a39a72f79fdfe6dc9c1713b3a823f1e276681b9703e",
        strip_prefix = "boringssl-648cbaf033401b7fe7acdce02f275b06a88aab5c",
        urls = ["https://github.com/google/boringssl/archive/648cbaf033401b7fe7acdce02f275b06a88aab5c.tar.gz"],
    )

def swift_repos():
    http_archive(
        name = "build_bazel_rules_apple",
        sha256 = "12865e5944f09d16364aa78050366aca9dc35a32a018fa35f5950238b08bf744",
        url = "https://github.com/bazelbuild/rules_apple/releases/download/0.34.2/rules_apple.0.34.2.tar.gz",
    )

    http_archive(
        name = "build_bazel_rules_swift",
        sha256 = "a2fd565e527f83fb3f9eb07eb9737240e668c9242d3bc318712efa54a7deda97",
        url = "https://github.com/bazelbuild/rules_swift/releases/download/0.27.0/rules_swift.0.27.0.tar.gz",
    )

    http_archive(
        name = "DrString",
        build_file_content = """exports_files(["drstring"])""",
        sha256 = "860788450cf9900613454a51276366ea324d5bfe71d1844106e9c1f1d7dfd82b",
        url = "https://github.com/dduan/DrString/releases/download/0.5.2/drstring-x86_64-apple-darwin.tar.gz",
    )

    http_archive(
        name = "SwiftLint",
        build_file_content = """exports_files(["swiftlint"])""",
        sha256 = "61d335766a39ba8fa499017a560950bd9fa0b0e5bc318559a9c1c7f4da679256",
        url = "https://github.com/realm/SwiftLint/releases/download/0.47.1/portable_swiftlint.zip",
    )

def kotlin_repos():
    http_archive(
        name = "rules_java",
        sha256 = "8c376f1e4ab7d7d8b1880e4ef8fc170862be91b7c683af97ca2768df546bb073",
        url = "https://github.com/bazelbuild/rules_java/releases/download/5.0.0/rules_java-5.0.0.1.tar.gz",
    )

    http_archive(
        name = "rules_jvm_external",
        sha256 = "cd1a77b7b02e8e008439ca76fd34f5b07aecb8c752961f9640dea15e9e5ba1ca",
        strip_prefix = "rules_jvm_external-4.2",
        url = "https://github.com/bazelbuild/rules_jvm_external/archive/4.2.zip",
    )

    http_archive(
        name = "io_bazel_rules_kotlin",
        sha256 = "12d22a3d9cbcf00f2e2d8f0683ba87d3823cb8c7f6837568dd7e48846e023307",
        url = "https://github.com/bazelbuild/rules_kotlin/releases/download/v1.5.0/rules_kotlin_release.tgz",
    )

    http_archive(
        name = "rules_detekt",
        sha256 = "44912c74dc2e164227b1102ef36227d0e78fdbd7c7359868ae13424eb4f0d5c2",
        strip_prefix = "bazel_rules_detekt-0.6.0",
        url = "https://github.com/buildfoundation/bazel_rules_detekt/archive/v0.6.0.tar.gz",
    )

    # gRPC java for @rules_proto_grpc
    # The current 0.2.0 uses v1.23.0 of gRPC java which has a buggy version of the grpc_java_repositories
    # where it tries to bind the zlib and errors out
    # The fix went in on this commit:
    # https://github.com/grpc/grpc-java/commit/57e7bd394e92015d2891adc74af0eaf9cd347ea8#diff-515bc54a0cbb4b12fb4a7c465758b011L128-L131
    http_archive(
        name = "io_grpc_grpc_java",
        sha256 = "8b495f58aaf75138b24775600a062bbdaa754d85f7ab2a47b2c9ecb432836dd1",
        strip_prefix = "grpc-java-1.24.0",
        urls = ["https://github.com/grpc/grpc-java/archive/v1.24.0.tar.gz"],
    )

    http_archive(
        name = "rules_proto_grpc",
        sha256 = "1e08cd6c61f893417b14930ca342950f5f22f71f929a38a8c4bbfeae2a80d03e",
        strip_prefix = "rules_proto_grpc-0.2.0",
        urls = ["https://github.com/rules-proto-grpc/rules_proto_grpc/archive/0.2.0.tar.gz"],
    )

    http_file(
        name = "kotlin_formatter",
        executable = 1,
        sha256 = "115d4c5cb3421eae732c42c137f5db8881ff9cc1ef180a01e638283f3ccbae44",
        urls = ["https://github.com/pinterest/ktlint/releases/download/0.37.1/ktlint"],
    )

    http_archive(
        name = "robolectric",
        sha256 = "d4f2eb078a51f4e534ebf5e18b6cd4646d05eae9b362ac40b93831bdf46112c7",
        urls = ["https://github.com/robolectric/robolectric-bazel/archive/4.4.tar.gz"],
        strip_prefix = "robolectric-bazel-4.4",
    )

def android_repos():
    http_archive(
        name = "build_bazel_rules_android",
        urls = ["https://github.com/bazelbuild/rules_android/archive/refs/tags/v0.1.1.zip"],
        sha256 = "cd06d15dd8bb59926e4d65f9003bfc20f9da4b2519985c27e190cddc8b7a7806",
        strip_prefix = "rules_android-0.1.1",
    )

def python_repos():
    http_archive(
        name = "pybind11_bazel",
        strip_prefix = "pybind11_bazel-26973c0ff320cb4b39e45bc3e4297b82bc3a6c09",
        urls = ["https://github.com/pybind/pybind11_bazel/archive/26973c0ff320cb4b39e45bc3e4297b82bc3a6c09.zip"],
        sha256 = "a5666d950c3344a8b0d3892a88dc6b55c8e0c78764f9294e806d69213c03f19d",
    )
    http_archive(
        name = "pybind11",
        build_file = "@pybind11_bazel//:pybind11.BUILD",
        strip_prefix = "pybind11-2.6.1",
        urls = ["https://github.com/pybind/pybind11/archive/v2.6.1.tar.gz"],
        sha256 = "cdbe326d357f18b83d10322ba202d69f11b2f49e2d87ade0dc2be0c5c34f8e2a",
    )
