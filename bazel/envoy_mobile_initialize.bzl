load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file", "http_jar")
load("@build_bazel_apple_support//lib:repositories.bzl", "apple_support_dependencies")
load("@build_bazel_rules_apple//apple:repositories.bzl", "apple_rules_dependencies")
load("@build_bazel_rules_swift//swift:repositories.bzl", "swift_rules_dependencies")
load("@rules_jvm_external//:defs.bzl", "maven_install")
load("@io_bazel_rules_kotlin//kotlin:kotlin.bzl", "kotlin_repositories", "kt_register_toolchains")
load("@io_grpc_grpc_java//:repositories.bzl", "grpc_java_repositories")
load("@rules_proto_grpc//protobuf:repositories.bzl", "protobuf_repos")
load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")
load("@rules_proto_grpc//java:repositories.bzl", rules_proto_grpc_java_repos = "java_repos")

def envoy_mobile_initialize():
    _swift_ios_dependencies()
    _kotlin_android_dependencies()


def _swift_ios_dependencies():

    apple_support_dependencies()
    apple_rules_dependencies(ignore_version_differences = True)
    swift_rules_dependencies()


def _kotlin_android_dependencies():
    maven_install(
        artifacts = [
            # Kotlin
            "org.jetbrains.kotlin:kotlin-stdlib-jdk8:1.3.11",

            # Test artifacts
            "org.assertj:assertj-core:3.9.0",
            "junit:junit:4.12",
            "org.mockito:mockito-inline:2.28.2",
            "org.mockito:mockito-core:2.28.2",
        ],
        repositories = [
            "https://repo1.maven.org/maven2",
            "https://jcenter.bintray.com/",
        ],
    )
    kotlin_repositories()
    kt_register_toolchains()

    grpc_java_repositories(
        omit_bazel_skylib = True,
        omit_com_google_protobuf = True,
        omit_com_google_protobuf_javalite = True,
        omit_net_zlib = True,
    )
    protobuf_repos()
    rules_proto_grpc_toolchains()
    rules_proto_grpc_java_repos()
