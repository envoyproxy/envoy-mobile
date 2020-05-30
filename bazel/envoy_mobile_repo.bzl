load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file", "http_jar")

def envoy_mobile_repo():
    _upstream_envoy_overrides()

def _upstream_envoy_overrides():
    # Patch protobuf to prevent duplicate symbols: https://github.com/lyft/envoy-mobile/issues/617
    # More details: https://github.com/protocolbuffers/protobuf/issues/7046
    # TODO: Remove after https://github.com/bazelbuild/bazel/pull/10493 is merged to Bazel
    # Reverts:
    # - https://github.com/protocolbuffers/protobuf/commit/7b28278c7d4f4175e70aef2f89d304696eb85ae3
    # - https://github.com/protocolbuffers/protobuf/commit/a03d332aca5d33c5d4b2cd25037c9e37d57eff02
    http_archive(
        name = "com_google_protobuf",
        patch_args = ["-p1"],
        patches = [
            "@envoy//bazel:protobuf.patch",
            "//bazel:protobuf.patch",
        ],
        sha256 = "d7cfd31620a352b2ee8c1ed883222a0d77e44346643458e062e86b1d069ace3e",
        strip_prefix = "protobuf-3.10.1",
        urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.10.1/protobuf-all-3.10.1.tar.gz"],
    )

    # Patch upstream Abseil to prevent Foundation dependency from leaking into Android builds.
    # Workaround for https://github.com/abseil/abseil-cpp/issues/326.
    # TODO: Should be removed in https://github.com/lyft/envoy-mobile/issues/136 once rules_android
    # supports platform toolchains.
    http_archive(
        name = "com_google_absl",
        patches = ["//bazel:abseil.patch"],
        sha256 = "14ee08e2089c2a9b6bf27e1d10abc5629c69c4d0bab4b78ec5b65a29ea1c2af7",
        strip_prefix = "abseil-cpp-cf3a1998e9d41709d4141e2f13375993cba1130e",
        # 2020-03-05
        urls = ["https://github.com/abseil/abseil-cpp/archive/cf3a1998e9d41709d4141e2f13375993cba1130e.tar.gz"],
    )

    # This should be kept in sync with Envoy itself, we just need to apply this patch
    # Remove this once https://boringssl-review.googlesource.com/c/boringssl/+/37804 is in master-with-bazel
    http_archive(
        name = "boringssl",
        patches = ["//bazel:boringssl.patch"],
        sha256 = "36049e6cd09b353c83878cae0dd84e8b603ba1a40dcd74e44ebad101fc5c672d",
        strip_prefix = "boringssl-37b57ed537987f1b4c60c60fa1aba20f3a0f6d26",
        urls = ["https://github.com/google/boringssl/archive/37b57ed537987f1b4c60c60fa1aba20f3a0f6d26.tar.gz"],
    )
