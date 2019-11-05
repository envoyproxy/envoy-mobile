licenses(["notice"])  # Apache 2

filegroup(
    name = "proguard_rules",
    srcs = ["proguard.txt"],
    visibility = ["//visibility:public"],
)

genrule(
    name = "javadocs",
    srcs = [],
    outs = ["envoy-javadoc.jar"],
    cmd = """
curl -L -o dokka.jar "https://github.com/Kotlin/dokka/releases/download/{version}/dokka-fatjar-{version}.jar"
echo "{sha256} dokka.jar" | sha256sum --check
origdir=$$PWD
tmp_dir=$$(mktemp -d)
java -jar $$origdir/dokka.jar $$origdir/library/java/src/ -format javadoc -output $$tmp_dir
java -jar $$origdir/dokka.jar $$origdir/library/kotlin/src/ -format javadoc -output $$tmp_dir
cd $$tmp_dir
zip -r $$origdir/$@ .
    """.format(
        sha256 = "4c73eee92dd652ea8e2afd7b20732cf863d4938a30f634d12c88fe64def89fd8",
        version = "0.9.18",
    ),
    visibility = ["//visibility:public"],
)
