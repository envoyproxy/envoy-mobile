
def get_transitive_srcs(srcs=[], deps=[]):
    """Obtain the source files for a target and its transitive dependencies.
    Args:
      srcs: a list of source files
      deps: a list of targets that are direct dependencies
    Returns:
      a collection of the transitive sources
    """
    return depset(
        srcs,
        transitive = [dep for dep in deps],
    )

def _impl(ctx):
    print("~~~~~~~~~~~~~~~~~")
    sources = []
    # TODO: do i want attr dep?
    for src in ctx.attr.deps:
        if JavaInfo in src:
            java_info = src[JavaInfo]
            for source_jar in java_info.source_jars:
                print(source_jar.path)
                sources.append(source_jar)
#            transitive_source_jars = java_info.transitive_source_jars
#            for transitive_sources in transitive_source_jars.to_list():
#                sources.append(transitive_sources)
    output_directory = ctx.outputs.sources.dirname
    print(output_directory)
    # Unzip all the sources
    for source in sources:
        zipper_args = ctx.actions.args()
        zipper_args.add("vx", source)
        zipper_args.add("-d", ctx.outputs.sources.dirname)
        print(zipper_args)
        ctx.actions.run(
            inputs = sources,
            outputs = [ctx.outputs.sources],
            executable = ctx.executable.zipper,
            arguments = [zipper_args],
            progress_message = "Creating sources...",
            mnemonic = "sourcesjar",
        )
    print("~~~~~~~~~~~~~~~~~")

sources_jar = rule(
    implementation = _impl,
    attrs = {
        "srcs" : attr.label_list(allow_files = True),
        "deps" : attr.label_list(),
        "zipper": attr.label(default = Label("@bazel_tools//tools/zip:zipper"), cfg = "host", executable=True),
    },
    outputs ={
        "sources": "%{name}-sources.jar"
    }
)