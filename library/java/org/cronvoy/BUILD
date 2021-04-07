# Android libraries for cronvoy

load("@build_bazel_rules_android//android:rules.bzl", "android_library")
load("@rules_jvm_external//:defs.bzl", "artifact")

# A raw http client backed by Cronet.
android_library(
    name = "cronvoy",
    srcs = [
        "BidirectionalStreamBuilderImpl.java",
        "CallbackExceptionImpl.java",
        "CronetEngineBase.java",
        "CronetEngineBuilderImpl.java",
        "CronetExceptionImpl.java",
        "CronetMetrics.java",
        "CronvoyEngine.java",
        "CronvoyEngineBuilderImpl.java",
        "InputStreamChannel.java",
        "JavaCronetProvider.java",
        "JavaUploadDataSinkBase.java",
        "JavaUrlRequest.java",
        "JavaUrlRequestUtils.java",
        "NetworkExceptionImpl.java",
        "Preconditions.java",
        "QuicExceptionImpl.java",
        "RequestFinishedInfoImpl.java",
        "ThreadStatsUid.java",
        "UrlRequestBase.java",
        "UrlRequestBuilderImpl.java",
        "UrlResponseInfoImpl.java",
        "UserAgent.java",
        "VersionSafeCallbacks.java",
    ],
    manifest = "CronvoyManifest.xml",
    visibility = ["//visibility:public"],
    deps = [
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_base_engine_lib",
        "//library/java/io/envoyproxy/envoymobile/engine:envoy_engine_lib",
        "//library/java/io/envoyproxy/envoymobile/engine/types:envoy_c_types_lib",
        "//library/java/org/chromium/net",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_interfaces_lib",
        "//library/kotlin/io/envoyproxy/envoymobile:envoy_lib",
        artifact("androidx.annotation:annotation"),
    ],
)
