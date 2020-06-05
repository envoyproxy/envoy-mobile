licenses(["notice"])  # Apache 2

filegroup(
    name = "proguard_rules",
    srcs = ["proguard.txt"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "javadocs",
    srcs = ["envoy-sources.jar"],
    outs = ["envoy-javadoc.jar"],
    cmd = """
orig_dir=$$PWD
sources_dir=$$(mktemp -d)
unzip $(SRCS) -d $$sources_dir
tmp_dir=$$(mktemp -d)
java -jar $(location @kotlin_dokka//jar) \
    $$sources_dir \
    -format javadoc \
    -output $$tmp_dir > /dev/null
cd $$tmp_dir
zip -r $$orig_dir/$@ . > /dev/null
    """,
    tools = ["@kotlin_dokka//jar"],
    visibility = ["//visibility:public"],
)
