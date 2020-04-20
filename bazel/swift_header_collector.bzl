"""
Propagate the generated Swift header from a swift_library target

This exists to work around https://github.com/bazelbuild/rules_swift/issues/291
"""

def _swift_header_collector(ctx):
    dep = ctx.attr.deps[0]
    return [
        DefaultInfo(
            files = dep[CcInfo].compilation_context.headers,
        ),
    ]

swift_header_collector = rule(
    attrs = dict(
        deps = attr.label_list(
            mandatory = True,
            allow_empty = False,
        ),
    ),
    implementation = _swift_header_collector,
)
