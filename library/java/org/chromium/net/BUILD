# Cronet API fork

load("@build_bazel_rules_android//android:rules.bzl", "android_library")
load("@rules_jvm_external//:defs.bzl", "artifact")

# Copy of the original Cronet API
android_library(
    name = "net",
    srcs = glob(["*.java"]),
    manifest = "ChromiumNetManifest.xml",
    visibility = ["//visibility:public"],
    deps = [
        artifact("androidx.annotation:annotation"),
    ],
)
