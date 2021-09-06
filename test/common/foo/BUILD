cc_library(
    name = "a",
    srcs = ["a.cc"],
)

cc_test(
    name = "a_test",
    srcs = ["a_test.cc"],
    copts = ["-Iexternal/gtest"],
    deps = ["@gtest//:gtest_main", ":a"],
)
