#!/usr/bin/python

import json
import os
import sys


def _main():
    # type: () -> None
    _touch_deps_files(sys.argv)
    _touch_swiftmodule(sys.argv)


def _touch_deps_files(args):
    # type: (str) -> None
    "Touch the Xcode required .d files"
    flag = args.index("-output-file-map")
    output_file_map_path = args[flag + 1]

    with open(output_file_map_path) as f:
        output_file_map = json.load(f)

    for entry in output_file_map.values():
        deps_file = entry.get("dependencies")
        if deps_file:
            _touch(deps_file)


def _touch_swiftmodule(args):
    # type: (str) -> None
    "Touch the Xcode required .swiftmodule and .swiftdoc files"
    flag = args.index("-emit-module-path")
    swiftmodule_path = args[flag + 1]
    module_root, _ = os.path.splitext(swiftmodule_path)
    swiftdoc_path = "{}.swiftdoc".format(module_root)

    _touch(swiftmodule_path)
    _touch(swiftdoc_path)


def _touch(path):
    # type: (str) -> None
    open(path, "w")


if __name__ == "__main__":
    _main()
