local_repository(
    name = "envoy",
    path = "envoy",
)

# Envoy Mobile dependencies. Contains overrides for upstream Envoy.
# Note: This needs to be called before Upstream Envoy dependencies
load("//bazel:envoy_mobile_dependencies.bzl", "envoy_mobile_dependencies")
envoy_mobile_dependencies()

load("//bazel:envoy_mobile_initialize.bzl", "envoy_mobile_initialize")
envoy_mobile_initialize()

# Upstream Envoy
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

# From envoy_dependencies()
load("@rules_foreign_cc//:workspace_definitions.bzl", "rules_foreign_cc_dependencies")
rules_foreign_cc_dependencies()

android_sdk_repository(name = "androidsdk")
android_ndk_repository(name = "androidndk")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
# TODO: Detekt issue
#       The rules_detekt_dependencies initializes the dependencies via http archives
#       the issue here is that @rules_detekt//detekt:toolchains.bzl loads these dependencies.
#       This means that we need to separate the two method calls into different files.
rules_detekt_version = "0.3.0"
rules_detekt_sha = "b1b4c8a3228f880a169ab60a817619bc4cf254443196e7e108ece411cb9c580e"
http_archive(
    name = "rules_detekt",
    sha256 = rules_detekt_sha,
    strip_prefix = "bazel_rules_detekt-{v}".format(v = rules_detekt_version),
    url = "https://github.com/buildfoundation/bazel_rules_detekt/archive/v{v}.tar.gz".format(v = rules_detekt_version),
)
load("@rules_detekt//detekt:dependencies.bzl", "rules_detekt_dependencies")
rules_detekt_dependencies()
load("@rules_detekt//detekt:toolchains.bzl", "rules_detekt_toolchains")
rules_detekt_toolchains(detekt_version = "1.8.0")

