load("@rules_cc//cc:defs.bzl", "cc_library")
load("@build_bazel_rules_android//android:rules.bzl", "android_binary")
load("@google_bazel_common//tools/maven:pom_file.bzl", "pom_file")

# This file is based on https://github.com/aj-michael/aar_with_jni which is
# subject to the following copyright and license:

# MIT License

# Copyright (c) 2019 Adam Michael

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# android_library's implicit aar doesn't flatten its transitive
# dependencies. When using the kotlin rules, the kt_android_library rule
# creates a few underlying libraries, because of this the classes.jar in
# the aar we built was empty. This rule separately builds the underlying
# kt.jar file, and replaces the aar's classes.jar with the kotlin jar
#
# The final resulting artifacts are:
#   {archive_name}.aar
#   {archive_name}_pom.xml
#   {archive_name}-sources.jar
#   {archive_name}-javadoc.jar
def aar_with_jni(name, android_library, manifest, archive_name, native_deps = [], proguard_rules = "", visibility = []):
    manifest_name = name + "_android_manifest"
    android_binary_name = name + "_bin"
    jni_archive_name = archive_name + "_jni"
    cc_lib_name = name + "_jni_interface_lib"

    # Create a dummy manifest file for our android_binary
    native.genrule(
        name = archive_name + "_binary_manifest_generator",
        outs = [archive_name + "_generated_AndroidManifest.xml"],
        cmd = """
cat > $(OUTS) <<EOF
<manifest
  xmlns:android="http://schemas.android.com/apk/res/android"
  package="does.not.matter">
  <uses-sdk android:minSdkVersion="999"/>
</manifest>
EOF
""",
    )

    # We wrap our native so dependencies in a cc_library because android_binaries
    # require a library target as dependencies in order to generate the appropriate
    # architectures in the directory `lib/`
    cc_library(
        name = cc_lib_name,
        srcs = native_deps,
    )

    # This outputs {jni_archive_name}_unsigned.apk which will contain the base files for our aar
    android_binary(
        name = jni_archive_name,
        manifest = archive_name + "_generated_AndroidManifest.xml",
        custom_package = "does.not.matter",
        srcs = [],
        deps = [android_library, cc_lib_name],
    )

    # This creates bazel-bin/library/kotlin/src/io/envoyproxy/envoymobile/{name}_bin_deploy.jar
    # This jar has all the classes needed for our aar and will be our `classes.jar`
    android_binary(
        name = android_binary_name,
        manifest = manifest,
        srcs = [],
        deps = [android_library],
    )

    # This is to generate the envoy mobile aar AndroidManifest.xml
    native.genrule(
        name = manifest_name,
        outs = [manifest_name + ".xml"],
        cmd = """
cat > $(OUTS) <<EOF
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="io.envoyproxy.envoymobile" >

    <application>
    </application>

</manifest>
EOF
""",
    )

    # Internal names for aar companion artifacts:
    # 1. pom.xml generation rule
    # 2. sources jar generation rule (via java_binary)
    # 3. java docs jar generation rule (from sources jar)
    _pom_name = name + "_envoy_pom_xml"
    _sources_name = name + "_android_sources_jar"
    _javadocs_name = name + "_android_javadocs"

    # This is for the pom xml. It has a public visibility since this can be accessed in the root BUILD file
    pom_file(
        name = _pom_name,
        targets = [android_library],
        template_file = "//bazel:pom_template.xml",
    )

    # This implicitly outputs {name}_deploy-src.jar which is the sources jar
    native.java_binary(
        name = _sources_name,
        runtime_deps = [android_library],
        visibility = visibility,
    )

    # This takes all the source files from the source jar and creates a javadoc.jar from it
    native.genrule(
        name = _javadocs_name,
        srcs = [_sources_name + "_deploy-src.jar"],
        outs = [_javadocs_name + ".jar"],
        cmd = """
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
    orig_dir=$$PWD
    sources_dir=$$(mktemp -d)
    echo "sources dir:"
    echo $$sources_dir

    unzip $(SRCS) -d $$sources_dir > /dev/null
    echo "===="
    ls $$sources_dir
    echo "===="
    tmp_dir=$$(mktemp -d)
    echo "tmp dir:"
    echo $$tmp_dir
    java -jar $(location @kotlin_dokka//jar) \
        $$sources_dir \
        -format javadoc \
        -output $$tmp_dir > /dev/null
    cd $$tmp_dir
    echo "#################"
    ls
    echo "#################"
    zip -r $$orig_dir/$@ . > /dev/null
    echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
        """,
        tools = ["@kotlin_dokka//jar"],
    )

    # This gen rule does the following:
    # 1. Unzips the apk file generated by the `jni_archive_name` into a temporary directory
    # 2. Renames the `lib` directory to `jni` directory since the aar requires the so files
    #    to be in the `jni` directory
    # 3. Copy the android binary `jar` output from the `android_binary_name` as `classes.jar`
    # 4. Copy the proguard rules specified in the macro parameters
    # 5. Override the apk's aar with a generated one
    # 6. Zip everything in the temporary directory into the output
    native.genrule(
        name = name,
        srcs = [
            android_binary_name + "_deploy.jar",
            jni_archive_name + "_unsigned.apk",
            manifest_name,
            _pom_name,
            _sources_name + "_deploy-src.jar",
            _javadocs_name,
            proguard_rules,
        ],
        outs = [
            archive_name + ".aar",
            archive_name + "_pom.xml",
            archive_name + "-sources.jar",
            archive_name + "-javadoc.jar",
        ],
        visibility = visibility,
        cmd = """
    # Set source variables
    set -- $(SRCS)
    src_deploy_jar=$$1
    src_jni_archive_apk=$$2
    src_manifest_xml=$$3
    src_pom_xml=$$4
    src_sources_jar=$$5
    src_javadocs=$$6
    src_proguard_txt=$$7

    # Set output variables
    set -- $(OUTS)
    out_aar=$$1
    out_pom_xml=$$2
    out_sources_jar=$$3
    out_javadocs=$$4

    orig_dir=$$PWD
    classes_dir=$$(mktemp -d)
    echo "Creating classes.jar from $$src_deploy_jar"
    cd $$classes_dir
    unzip $$orig_dir/$$src_deploy_jar "io/envoyproxy/*" "META-INF/" > /dev/null
    zip -r classes.jar * > /dev/null
    cd $$orig_dir

    echo "Constructing aar..."
    final_dir=$$(mktemp -d)
    cp $$classes_dir/classes.jar $$final_dir
    cd $$final_dir
    unzip $$orig_dir/$$src_jni_archive_apk lib/* > /dev/null
    mv lib jni
    cp $$orig_dir/$$src_proguard_txt ./proguard.txt
    cp $$orig_dir/$$src_manifest_xml AndroidManifest.xml
    zip -r tmp.aar * > /dev/null

    echo "Outputting pom.xml, sources.jar, and javadocs.jar..."
    mv tmp.aar $$orig_dir/$$out_aar
    cd $$orig_dir
    mv $$src_pom_xml $$out_pom_xml
    mv $$src_sources_jar $$out_sources_jar
    mv $$src_javadocs $$out_javadocs
    echo "Finished!"
    """,
    )
