load("@envoy//bazel:envoy_build_system.bzl", "envoy_cc_library", "envoy_package")

licenses(["notice"])  # Apache 2

envoy_package()

envoy_cc_library(
    name = "connectivity_manager_lib",
    srcs = [
        "connectivity_manager.cc",
        "android.cc",
    ] + select({
        "//bazel:include_ifaddrs": [
            "//third_party:android/ifaddrs-android.h",
            "//third_party:android/LocalArray.h",
            "//third_party:android/ScopedFd.h",
        ],
        "//conditions:default": [],
    }),
    hdrs = [
        "android.h",
        "connectivity_manager.h",
        "proxy_settings.h",
    ],
    copts = select({
        "//bazel:include_ifaddrs": ["-DINCLUDE_IFADDRS"],
        "//conditions:default": [],
    }),
    repository = "@envoy",
    deps = [
        "//library/common/network:src_addr_socket_option_lib",
        "//library/common/types:c_types_lib",
        "@envoy//envoy/network:socket_interface",
        "@envoy//envoy/singleton:manager_interface",
        "@envoy//source/common/common:assert_lib",
        "@envoy//source/common/common:scalar_to_byte_vector_lib",
        "@envoy//source/common/common:utility_lib",
        "@envoy//source/common/network:addr_family_aware_socket_option_lib",
        "@envoy//source/common/network:socket_option_lib",
        "@envoy//source/extensions/common/dynamic_forward_proxy:dns_cache_manager_impl",
    ],
)

envoy_cc_library(
    name = "src_addr_socket_option_lib",
    srcs = ["src_addr_socket_option_impl.cc"],
    hdrs = ["src_addr_socket_option_impl.h"],
    repository = "@envoy",
    deps = [
        "@envoy//envoy/network:address_interface",
        "@envoy//source/common/network:socket_option_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
    ],
)

envoy_cc_library(
    name = "socket_tag_socket_option_lib",
    srcs = ["socket_tag_socket_option_impl.cc"],
    hdrs = ["socket_tag_socket_option_impl.h"],
    repository = "@envoy",
    deps = [
        "//library/common/jni:android_jni_utility_lib",
        "@envoy//envoy/network:address_interface",
        "@envoy//source/common/common:scalar_to_byte_vector_lib",
        "@envoy//source/common/network:socket_option_lib",
        "@envoy_api//envoy/config/core/v3:pkg_cc_proto",
    ],
)

envoy_cc_library(
    name = "synthetic_address_lib",
    hdrs = ["synthetic_address_impl.h"],
    repository = "@envoy",
    deps = [
        "@envoy//envoy/network:address_interface",
        "@envoy//source/common/network:socket_interface_lib",
    ],
)

cc_library(
    name = "apple_platform_cert_verifier",
    srcs = select({
        "@envoy//bazel:apple": ["apple_platform_cert_verifier.cc"],
        "//conditions:default": [],
    }),
    hdrs = select({
        "@envoy//bazel:apple": ["apple_platform_cert_verifier.h"],
        "//conditions:default": [],
    }),
    deps = select({
        "@envoy//bazel:apple": [
            "//library/common/extensions/cert_validator/platform_bridge:c_types_lib",
            "//library/common:envoy_main_interface_lib",
            "@envoy//bazel:boringssl",
        ],
        "//conditions:default": [],
    }),
)
