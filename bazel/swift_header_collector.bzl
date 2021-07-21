"""
Propagate the generated Swift header from a swift_library target
This exists to work around https://github.com/bazelbuild/rules_swift/issues/291
"""

def _swift_header_collector(ctx):
    return [
        DefaultInfo(
            files = ctx.attr.library[CcInfo].compilation_context.headers,
        ),
    ]

swift_header_collector = rule(
    attrs = dict(
        library = attr.label(
            mandatory = True,
            providers = [CcInfo],
        ),
    ),
    implementation = _swift_header_collector,
)
