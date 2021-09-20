cc_library(
    name = "b",
    srcs = ["b.cc"],
)

cc_test(
    name = "b_test",
    srcs = ["b_test.cc"],
    copts = ["-Iexternal/gtest"],
    deps = ["@gtest//:gtest_main", ":b"],
)
