licenses(["notice"])  # Apache 2

load("//tools:detekt.bzl", "detekt")
load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kt_android_library")


android_binary(
    name = "hello_envoy_kt",
    custom_package = "io.envoyproxy.envoymobile.helloenvoykotlin",
    manifest = "AndroidManifest.xml",
    deps = [
        "hello_envoy_kt_lib",
    ],
)

kt_android_library(
    name = "hello_envoy_kt_lib",
    srcs = glob([
        "MainActivity.kt",
        "Response.kt",
        "ResponseRecyclerViewAdapter.kt",
        "ResponseViewHolder.kt",
    ]),
    custom_package = "io.envoyproxy.envoymobile.helloenvoykotlin",
    manifest = "AndroidManifest.xml",
    resource_files = glob([
        "res/**/*.xml",
        "res/**/*.yaml",
    ]),
    deps = [
        "//dist:envoy_mobile_android",
        "@androidsdk//com.android.support:appcompat-v7-25.0.0",
        "@androidsdk//com.android.support:recyclerview-v7-25.0.0",
    ],
)

genrule(
    name = "genrule_linter",
        srcs = ["@kotlin_detekt_jar//jar"],
        cmd = "java -jar $(location @kotlin_detekt_jar//jar) --help",
        outs = ["jar-output"],
)

java_binary(
    name = "kotlin_linter",
    deps = [
        "@kotlin_detekt_jar//jar"
    ],
    jvm_flags = ["--help"]
)