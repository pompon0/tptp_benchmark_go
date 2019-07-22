load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

"""
local_repository(
  name = "tptp_parser",
  path = "/home/pompon/github/tptp_parser"
)
"""
http_archive(
  name = "tptp_parser",
  strip_prefix = "tptp_parser-master",
  urls = ["https://github.com/pompon0/tptp_parser/archive/master.zip"],
)

http_archive(
  name = "tptp_parser_bin",
  urls = ["https://storage.googleapis.com/tptp/tools.tgz"],
  build_file = "//:third_party/tptp_parser_bin.BUILD",
)

http_archive(
  name = "eprover",
  urls = [
    "http://wwwlehre.dhbw-stuttgart.de/~sschulz/WORK/E_DOWNLOAD/V_2.3/E.tgz",
    "https://storage.googleapis.com/tptp/eprover_2_3.tgz",
  ],
  sha256 = "5366d2de77e6856250e26a967642389e81a6f823caedccaf5022a09242aceb96",
  build_file = "//:third_party/eprover.BUILD",
)

http_archive(
    name = "leancop",
    urls = ["https://storage.googleapis.com/tptp/leancop.tgz"],
    build_file = "//:third_party/leancop.BUILD",
)

http_archive(
    name = "tptp_sample",
    urls = ["https://storage.googleapis.com/tptp/tptp_sample.tgz"],
    build_file = "//:third_party/tptp_sample.BUILD",
)

http_archive(
  name = "com_google_protobuf",
  strip_prefix = "protobuf-3.9.0",
  urls = ["https://github.com/google/protobuf/archive/v3.9.0.zip"],
)
load("@com_google_protobuf//:protobuf_deps.bzl","protobuf_deps")
protobuf_deps()

http_archive(
  name = "gtest",
  strip_prefix = "googletest-release-1.8.1",
  urls = ["https://github.com/google/googletest/archive/release-1.8.1.zip"],
  sha256 = "927827c183d01734cc5cfef85e0ff3f5a92ffe6188e0d18e909c5efebf28a0c7",
)

http_archive(
  name = "abseil",
  strip_prefix = "abseil-cpp-master",
  urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
)

http_archive(
    name = "io_bazel_rules_go",
    urls = ["https://github.com/bazelbuild/rules_go/releases/download/0.19.1/rules_go-0.19.1.tar.gz"],
    sha256 = "8df59f11fb697743cbb3f26cfb8750395f30471e9eabde0d174c3aebc7a1cd39",
)
load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")
go_rules_dependencies()
go_register_toolchains()

http_archive(
    name = "bazel_gazelle",
    urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/0.17.0/bazel-gazelle-0.17.0.tar.gz"],
    sha256 = "3c681998538231a2d24d0c07ed5a7658cb72bfb5fd4bf9911157c0e9ac6a2687",
)
load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", "go_repository")
gazelle_dependencies()

######################################################
# generated by gazelle

go_repository(
    name = "org_golang_x_sync",
    commit = "e225da77a7e68af35c70ccbf71af2b83e6acac3c",
    importpath = "golang.org/x/sync",
)
