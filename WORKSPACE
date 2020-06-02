load("//bazel:envoy_mobile_repo.bzl", "envoy_mobile_repos")
envoy_mobile_repos()

local_repository(
    name = "envoy",
    path = "envoy",
)

local_repository(
    name = "envoy_build_config",
    path = "envoy_build_config",
)

load("@envoy//bazel:api_binding.bzl", "envoy_api_binding")
envoy_api_binding()

load("@envoy//bazel:api_repositories.bzl", "envoy_api_dependencies")
envoy_api_dependencies()

load("@envoy//bazel:repositories.bzl", "envoy_dependencies")
envoy_dependencies()

load("@envoy//bazel:repositories_extra.bzl", "envoy_dependencies_extra")
envoy_dependencies_extra()

load("@envoy//bazel:dependency_imports.bzl", "envoy_dependency_imports")
envoy_dependency_imports()

load("//bazel:envoy_mobile_swift_bazel_support.bzl", "swift_support")
swift_support()

load("//bazel:envoy_mobile_dependencies.bzl", "envoy_mobile_dependencies")
envoy_mobile_dependencies()

load("//bazel:envoy_mobile_toolchains.bzl", "envoy_mobile_toolchains")
envoy_mobile_toolchains()

android_sdk_repository(name = "androidsdk")

android_ndk_repository(name = "androidndk")

load("@rules_jvm_external//:defs.bzl", "maven_install")
maven_install(
    artifacts = [
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

