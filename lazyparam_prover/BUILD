load("@io_bazel_rules_go//proto:def.bzl", "go_proto_library")

package(
    default_visibility = ["//visibility:public"],
)

proto_library(
    name = "prover_proto",
    srcs = ["prover.proto"],
)

cc_proto_library(
    name = "prover_cc_proto",
    deps = [":prover_proto"],
)

go_proto_library(
    name = "prover_go_proto",
    importpath = "github.com/pompon0/tptp_benchmark_go/lazyparam_prover/prover_go_proto",
    proto = ":prover_proto",
)

cc_library(
    name = "derived",
    hdrs = [
        "constraint.h",
        "derived.h",
    ],
    deps = [
        "//utils:utils",
        "//lazyparam_prover/memory:memory",
        "//lazyparam_prover/syntax:syntax",
    ],
)

cc_test(
    name = "derived_test",
    srcs = [
        "derived_test.cc",
        "constraint_test.cc",
    ],
    deps = [
        ":derived",
        "@gtest//:gtest_main",
        "//utils:utils",
    ],
)

cc_library(
    name = "node",
    hdrs = ["node.h"],
    deps = [
        "//utils:utils",
        "//tptp_parser/proto:tptp_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "parse",
    hdrs = [
        "parse.h",
    ],
    deps = [
        ":node",
        "//lazyparam_prover/syntax:syntax",
        "//tptp_parser/proto:tptp_cc_proto",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_library(
    name = "parse2",
    hdrs = [
        "parse2.h",
    ],
    deps = [
        ":node",
        "//lazyparam_prover/syntax:syntax",
        "//tptp_parser/proto:tptp_cc_proto",
        # vampire agressively takes over signal handling of the whole binary
        # in particular it severly reduces the global memory limits.
        # DO NOT LINK into actual prover, until you refector it.
        "@vampire//:lib",
        "@com_google_protobuf//:protobuf",
    ],
)

cc_binary(
    name = "tool",
    srcs = ["tool.cc"],
    deps = [
        "//tptp_parser/proto:tptp_cc_proto",
        ":parse2",
        "//utils:utils",
        "@abseil//absl/flags:parse",
        "@abseil//absl/flags:flag",
    ],
    visibility = ["//visibility:public"],
    features = ["fully_static_link"],
)

cc_library(
    name = "mgu",
    hdrs = [
        "mgu.h",
        "kbo.h",
        "lpo.h",
        "fun_ord.h",
        "constraint.h",
        "constrained_valuation.h",
        "rewrite_test_utils.h",
    ],
    deps = [
        ":derived",
        "//tptp_parser/proto:solutions_cc_proto",
        "//lazyparam_prover/syntax:syntax",
        "//lazyparam_prover/memory:memory",
    ],
)

cc_library(
    name = "prover",
    hdrs = [
        "search_state.h",
        "eq_axioms.h",
        "ground.h",
        "lpmod.h",
        "index.h",
        "prover_output.h",
    ],
    deps = [
        "//tptp_parser/proto:tptp_cc_proto",
        "//tptp_parser/proto:solutions_cc_proto",
        ":parse",
        "//lazyparam_prover/syntax:syntax",
        ":mgu",
        "//utils:utils",
        "@com_google_protobuf//:protobuf",
    ],
)

"""cc_library(
    name = "controller",
    hdrs = ["controller.h"],
    deps = [
      ":parse",
      ":prover",
      "//tool/bin:wrapper",
      "//utils:utils",
      "//lazyparam_prover/connection_tableau:prover",
    ],
)

cc_test(
    name = "controller_test",
    srcs = ["controller_test.cc"],
    deps = [
        ":controller",
        "//utils:utils",
        "@gtest//:gtest_main",
    ],
)"""

cc_test(
    name = "mgu_test",
    srcs = ["mgu_test.cc"],
    deps = [
        ":mgu",
        "//utils:utils",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "kbo_test",
    srcs = ["kbo_test.cc"],
    deps = [
        ":mgu",
        "//utils:utils",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "lpo_test",
    srcs = ["lpo_test.cc"],
    deps = [
        ":mgu",
        "//utils:utils",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "eq_axioms_test",
    srcs = ["eq_axioms_test.cc"],
    deps = [
        ":prover",
        "//utils:utils",
        "@gtest//:gtest_main",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":prover",
        ":prover_cc_proto",
        "//lazyparam_prover/lazy_paramodulation:prover",
        "//lazyparam_prover/connection_tableau:prover",
        "//lazyparam_prover/engine:engine",
        "//utils:utils",
        "@abseil//absl/flags:flag",
        "@abseil//absl/flags:parse",
    ],
    visibility = ["//visibility:public"],
    features = ["fully_static_link"],
    # linkopts = ["-static","--sysroot=/tmp/","-v"],
)
