"""
This rules creates a fat static framework that can be included later with
static_framework_import
"""

load("@build_bazel_rules_swift//swift:swift.bzl", "SwiftInfo", "swift_library")

MINIMUM_IOS_VERSION = "10.0"

_PLATFORM_TO_SWIFTMODULE = {
    "ios_armv7": "arm",
    "ios_arm64": "arm64",
    "ios_i386": "i386",
    "ios_x86_64": "x86_64",
}

def _zip_binary_arg(module_name, input_file):
    return "{module_name}.framework/{module_name}={file_path}".format(
        module_name = module_name,
        file_path = input_file.path,
    )

def _zip_swift_arg(module_name, swift_identifier, input_file):
    return "{module_name}.framework/Modules/{module_name}.swiftmodule/{swift_identifier}.{ext}={file_path}".format(
        module_name = module_name,
        swift_identifier = swift_identifier,
        ext = input_file.extension,
        file_path = input_file.path,
    )

def _swift_static_framework_impl(ctx):
    module_name = ctx.attr.framework_name
    fat_file = ctx.outputs.fat_file

    input_archives = []
    input_modules_docs = []
    zip_args = [_zip_binary_arg(module_name, fat_file)]

    for platform, archive in ctx.split_attr.archive.items():
        swiftmodule_identifier = _PLATFORM_TO_SWIFTMODULE[platform]
        if not swiftmodule_identifier:
            fail("Unhandled platform '{}'".format(platform))
        #library = archive[CcInfo].linking_context.libraries_to_link[0].pic_static_library
        swift_info = archive[SwiftInfo]
        swiftdoc = swift_info.direct_swiftdocs[0]
        swiftmodule = swift_info.direct_swiftmodules[0]

        libraries = archive[CcInfo].linking_context.libraries_to_link
        archives = []
        for library in libraries:
          archive = library.pic_static_library or library.static_library
          if archive:
            archives.append(archive)

        #archives = [a for a in (x.pic_static_library or x.static_library for x in libraries) if a]

        #for library in archive[CcInfo].linking_context.libraries_to_link:
        #  if library.pic_static_library:
        #    input_archives.append(library.pic_static_library)

        platform_archive = ctx.actions.declare_file("{}.{}.a".format(module_name, platform))
        #libtool_args = ["--mode=link", "cc", "-static", "-o", platform_archive.path] + \
        #    [x.path for x in archives]
        libtool_args = ["-no_warning_for_no_symbols", "-static", "-arch_only", platform, "-syslibroot", "__BAZEL_XCODE_SDKROOT__", "-o", platform_archive.path] + [x.path for x in archives]
        ctx.actions.run(
            inputs = archives,
            outputs = [platform_archive],
            mnemonic = "LibtoolLinkedLibraries",
            progress_message = "Combining libraries for {} on {}".format(module_name, platform),
            executable = ctx.executable._libtool,
            arguments = libtool_args,
        )

        input_archives.append(platform_archive)

        input_modules_docs += [swiftdoc, swiftmodule]
        zip_args += [
            _zip_swift_arg(module_name, swiftmodule_identifier, swiftdoc),
            _zip_swift_arg(module_name, swiftmodule_identifier, swiftmodule),
        ]

    ctx.actions.run(
        inputs = input_archives,
        outputs = [fat_file],
        mnemonic = "LipoPlatformLibraries",
        progress_message = "Creating fat library for {}".format(module_name),
        executable = "lipo",
        arguments = ["-create", "-output", fat_file.path] + [x.path for x in input_archives],
    )

    output_file = ctx.outputs.output_file
    ctx.actions.run(
        inputs = input_modules_docs + [fat_file],
        outputs = [output_file],
        mnemonic = "CreateFrameworkZip",
        progress_message = "Creating framework zip for {}".format(module_name),
        executable = ctx.executable._zipper,
        arguments = ["c", output_file.path] + zip_args,
    )

    return [
        DefaultInfo(
            files = depset([output_file]),
        ),
    ]

_swift_static_framework = rule(
    implementation = _swift_static_framework_impl,
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
        _libtool = attr.label(
            default = "@bazel_tools//tools/objc:libtool",
            cfg = "host",
            executable = True,
        ),
        _zipper = attr.label(
            default = "@bazel_tools//tools/zip:zipper",
            cfg = "host",
            executable = True,
        ),
    ),
    outputs = {
        "fat_file": "%{framework_name}.fat",
        "output_file": "%{framework_name}.zip",
    },
)

def swift_static_framework(
        name,
        module_name = None,
        srcs = [],
        copts = [],
        deps = [],
        visibility = []):
    """Create a static library, and static framework target for a swift module

    Args:
        name: The name of the module, the framework's name will be this name
            appending Framework so you can depend on this from other modules
        srcs: Custom source paths for the swift files
        copts: Any custom swiftc opts passed through to the swift_library
        deps: Any deps the swift_library requires
    """
    archive_name = name + "_archive"
    module_name = module_name or name + "_framework"
    swift_library(
        name = archive_name,
        module_name = module_name,
        srcs = srcs,
        copts = copts,
        deps = deps,
        visibility = ["//visibility:public"],
    )

    _swift_static_framework(
        name = name,
        framework_name = module_name,
        archive = archive_name,
        visibility = visibility,
    )
