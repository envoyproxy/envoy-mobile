java_binary(
  name = "app",
  srcs = ["App.java"],
  main_class = 'test.App',
  data = [":libnative.so", ":libnative.jnilib"],
  jvm_flags = ["-Djava.library.path=test"],
)

genrule(
    name = "libnative.jnilib",
    outs = ["libnative.jnilib"],
    srcs = ["libnative.so"],
    cmd = """
    cp -f $< $@
    """
)

cc_binary(
  name = "libnative.so",
  srcs = ["native.cc"],
  linkshared = 1,
  deps = ["@bazel_tools//tools/jdk:jni"],
)