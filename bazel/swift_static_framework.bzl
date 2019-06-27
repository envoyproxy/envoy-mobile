"""
This rules creates a fat static framework that can be included later with
static_framework_import
"""

load("@build_bazel_rules_swift//swift:swift.bzl", "SwiftInfo", "swift_library")

MINIMUM_IOS_VERSION = "11.0"

_PLATFORM_TO_SWIFTMODULE = {
    "ios_arm64": "arm64",
    "ios_x86_64": "x86_64",
}

def _tar_binary_arg(module_name, input_file):
    return "{module_name}.framework/{module_name}={file_path}".format(
        module_name = module_name,
        file_path = input_file.path,
    )

def _tar_swift_arg(module_name, swift_identifier, input_file):
    return "{module_name}.framework/Modules/{module_name}.swiftmodule/{swift_identifier}.{ext}={file_path}".format(
        module_name = module_name,
        swift_identifier = swift_identifier,
        ext = input_file.extension,
        file_path = input_file.path,
    )

def _prebuilt_swift_static_framework_impl(ctx):
    module_name = ctx.attr.framework_name
    fat_file = ctx.outputs.fat_file

    input_archives = []
    input_modules_docs = []
    tar_args = [_tar_binary_arg(module_name, fat_file)]

    for platform, archive in ctx.split_attr.archive.items():
        swiftmodule_identifier = _PLATFORM_TO_SWIFTMODULE[platform]
        if not swiftmodule_identifier:
            fail("Unhandled platform '{}'".format(platform))
        library = archive[CcInfo].linking_context.libraries_to_link.to_list()[0].pic_static_library
        swift_info = archive[SwiftInfo]
        swiftdoc = swift_info.direct_swiftdocs[0]
        swiftmodule = swift_info.direct_swiftmodules[0]

        input_archives.append(library)
        input_modules_docs += [swiftdoc, swiftmodule]
        tar_args += [
            _tar_swift_arg(module_name, swiftmodule_identifier, swiftdoc),
            _tar_swift_arg(module_name, swiftmodule_identifier, swiftmodule),
        ]

    ctx.actions.run(
        inputs = input_archives,
        outputs = [fat_file],
        mnemonic = "LipoSwiftLibraries",
        progress_message = "Combining libraries for {}".format(module_name),
        executable = "lipo",
        arguments = ["-create", "-output", fat_file.path] + [x.path for x in input_archives],
    )

    output_file = ctx.outputs.output_file
    ctx.actions.run(
        inputs = input_modules_docs + [fat_file],
        outputs = [output_file],
        mnemonic = "CreateSwiftFrameworkTar",
        progress_message = "Creating framework tar for {}".format(module_name),
        executable = ctx.executable._create_tar,
        arguments = [output_file.path] + tar_args,
    )

    return [
        DefaultInfo(
            files = depset([output_file]),
        ),
    ]

_prebuilt_swift_static_framework = rule(
    implementation = _prebuilt_swift_static_framework_impl,
    attrs = dict(
        archive = attr.label(
            mandatory = True,
            providers = [CcInfo, SwiftInfo],
            cfg = apple_common.multi_arch_split,
        ),
        framework_name = attr.string(mandatory = True),
        minimum_os_version = attr.string(default = MINIMUM_IOS_VERSION),
        platform_type = attr.string(
            default = str(apple_common.platform_type.ios),
        ),
        _create_tar = attr.label(
            default = "//tools:create_tar",
            cfg = "host",
            executable = True,
        ),
    ),
    outputs = {
        "fat_file": "%{framework_name}.fat",
        "output_file": "%{framework_name}.tar.bz2",
    },
)

def prebuilt_swift_static_framework(
        name,
        version = None,
        srcs = [],
        copts = [],
        deps = []):
    """Create a static library, and static framework target for a swift module

    Args:
        name: The name of the module, the framework's name will be this name
            appending Framework so you can depend on this from other modules
        version: The version of the library for use with metadata files
        srcs: Custom source paths for the swift files
        copts: Any custom swiftc opts passed through to the swift_library
        deps: Any deps the swift_library requires
    """
    srcs = srcs or native.glob(["Sources/**/*.swift"])
    swift_library(
        name = name,
        module_name = name,
        srcs = srcs,
        copts = copts,
        deps = deps,
        visibility = ["//visibility:public"],
    )

    framework_name = name + "Framework"
    _prebuilt_swift_static_framework(
        name = framework_name,
        framework_name = name,
        archive = name,
    )
