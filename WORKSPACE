load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

# "com_google_protobuf" declares some terribly old version of skylib,
# forcing the newest version beforehead
http_archive(
  name = "bazel_skylib",
  sha256 = "e5d90f0ec952883d56747b7604e2a15ee36e288bb556c3d0ed33e818a4d971f2",
  strip_prefix = "bazel-skylib-1.0.2",
  urls = ["https://github.com/bazelbuild/bazel-skylib/archive/1.0.2.tar.gz"],
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
    name = "vampire",
    strip_prefix = "vampire-3267e536135d0a9ac0691ee43153353cb130ca8e",
    urls = ["https://github.com/vprover/vampire/archive/3267e536135d0a9ac0691ee43153353cb130ca8e.tar.gz"],
    sha256 = "5459de1b1db951c8522b3b2e4af607c376da7e9cf41f2841b40e8271bd2abd14",
    build_file = "//:third_party/vampire.BUILD",
)

http_archive(
    name = "leancop",
    urls = ["https://storage.googleapis.com/tptp/leancop_bin.tgz"],
    sha256 = "c8c154c7f694ffd5eee7453bb50bd07f5eac0b4c564a5b9a0cb548e802ed7bbf",
    build_file = "//:third_party/leancop.BUILD",
)

http_archive(
    name = "leancop_prolog",
    urls = [
        "http://www.leancop.de/programs/leancop21.tar.gz",
        "https://storage.googleapis.com/tptp/leancop21.tar.gz",
    ],
    sha256 = "ce432c5f9368c093f08df3120218463d8bdb8412e71ec980ab9c852c13cef300",
    build_file = "//:third_party/leancop_prolog.BUILD",
)

http_file(
    name = "tptp4X",
    executable = 1,
    urls = ["https://storage.googleapis.com/tptp/tptp4X"],
    sha256 = "2418cb42c0f9013289ef7653c4ad9cd4d9b7d7ac67bbf5687dbb1ef1639e590e",
)

http_file(
    name = "mizar_problems",
    urls = ["https://storage.googleapis.com/tptp/mizar_problems.zip"],
    sha256 = "70a6e8467753395125f281ea371adc390585b667b84db79451fd0cc8780bd749",
)

http_file(
    name = "tptp_problems",
    urls = ["https://storage.googleapis.com/tptp/tptp_problems.zip"],
    sha256 = "f56cd27648898713e83e2e0dc69e295b316ba4b7acad0e41d7667610b666c5f0",
)

http_archive(
  name = "com_google_protobuf",
  strip_prefix = "protobuf-3.11.2",
  urls = ["https://github.com/google/protobuf/archive/v3.11.2.zip"],
  sha256 = "e4f8bedb19a93d0dccc359a126f51158282e0b24d92e0cad9c76a9699698268d",
)
load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

http_archive(
    name = "gtest",
    strip_prefix = "googletest-release-1.8.1",
    urls = ["https://github.com/google/googletest/archive/release-1.8.1.zip"],
    sha256 = "927827c183d01734cc5cfef85e0ff3f5a92ffe6188e0d18e909c5efebf28a0c7",
)

http_archive(
    name = "abseil",
    strip_prefix = "abseil-cpp-d659fe54b35ab9b8e35c72e50a4b8814167d5a84",
    urls = ["https://github.com/abseil/abseil-cpp/archive/d659fe54b35ab9b8e35c72e50a4b8814167d5a84.zip"],
    sha256 = "743f879030960c80a0445310567ca99d8eae87468dd0e78fd0c8bfd46f429e37",
)

http_archive(
    name = "io_bazel_rules_go",
    urls = ["https://github.com/bazelbuild/rules_go/releases/download/v0.20.0/rules_go-v0.20.0.tar.gz"],
    sha256 = "078f2a9569fa9ed846e60805fb5fb167d6f6c4ece48e6d409bf5fb2154eaf0d8",
)
load("@io_bazel_rules_go//go:deps.bzl", "go_rules_dependencies", "go_register_toolchains")
go_rules_dependencies()
go_register_toolchains()

http_archive(
    name = "bazel_gazelle",
    urls = ["https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.19.1/bazel-gazelle-v0.19.1.tar.gz"],
    sha256 = "86c6d481b3f7aedc1d60c1c211c6f76da282ae197c3b3160f54bd3a8f847896f",
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies", "go_repository")

gazelle_dependencies()

######################################################

http_archive(
    name = "io_bazel_rules_docker",
    sha256 = "df13123c44b4a4ff2c2f337b906763879d94871d16411bf82dcfeba892b58607",
    strip_prefix = "rules_docker-0.13.0",
    urls = ["https://github.com/bazelbuild/rules_docker/releases/download/v0.13.0/rules_docker-v0.13.0.tar.gz"],
)

load(
    "@io_bazel_rules_docker//repositories:repositories.bzl",
    container_repositories = "repositories",
)

container_repositories()

load(
    "@io_bazel_rules_docker//go:image.bzl",
    _go_image_repos = "repositories",
)

_go_image_repos()

load(
  "@io_bazel_rules_docker//container:container.bzl",
  "container_pull",
)

container_pull(
  name = "swipl",
  registry = "index.docker.io",
  repository = "amd64/swipl",
  digest = "sha256:b6f51e9cbccc55386bfa3381336c70bda145405cc258055fc8cc2afaddb9a35b",
)

######################################################
# haskell

http_archive(
  name = "rules_haskell",
  strip_prefix = "rules_haskell-master",
  urls = ["https://github.com/pompon0/rules_haskell/archive/master.tar.gz"],
  sha256 = "bb1444e628c09e3cd76578602b823493a531d3753eff31b696bf6ef3ad57116f",
)

load("@rules_haskell//haskell:repositories.bzl", "rules_haskell_dependencies")
rules_haskell_dependencies()
load("@rules_haskell//haskell:toolchain.bzl", "rules_haskell_toolchains")
rules_haskell_toolchains()

http_archive(
    name = "happy",
    strip_prefix = "happy-1.19.10",
    urls = ["http://hackage.haskell.org/package/happy-1.19.10/happy-1.19.10.tar.gz"],
    sha256 = "22eb606c97105b396e1c7dc27e120ca02025a87f3e44d2ea52be6a653a52caed",
    build_file = "//:third_party/happy.BUILD",
)

load("@rules_haskell//haskell:cabal.bzl", "stack_snapshot")

stack_snapshot(
    name = "stackage",
    packages = [
        "base",
        "data-default-class",
        "haskell-src-exts",
        "either",
        "tasty-hunit",
        "tasty",
        "parsec",
        "transformers",
        "unix",
        "bytestring",
        "utf8-string",
        "tar",
        "http-conduit",
        "zlib",
        "lens",
        "proto-lens-runtime",
        "proto-lens",
        "microlens",
        "lens-family",
        "text",
        "containers",
        "mtl",
        "MissingH",
        "threads",
        "concurrent-extra",
        "unbounded-delays",
        "deepseq",
        "split",
        "data-ordlist",
        "clock",
        "hashable",
        "hashtables",
        "options",
        "array",
        "vector",
        "proto-lens-protoc",
        "attoparsec",
        "prettyprinter",
        "scientific",
        "semigroups",
    ],
    snapshot = "lts-14.2",
    tools = ["@happy"],
)

http_archive(
    name = "set-monad",
    strip_prefix = "set-monad-master",
    urls = ["https://github.com/giorgidze/set-monad/archive/master.zip"],
    sha256 = "64079a9dd5d59b92ceaf7c79768ccbcb753c9bc4a9935cfb8e430519d32ca468",
    build_file = "//:third_party/set-monad.BUILD",
)

http_archive(
    name = "tptp",
    urls = ["https://hackage.haskell.org/package/tptp-0.1.1.0/tptp-0.1.1.0.tar.gz"],
    build_file = "//:third_party/tptp.BUILD",
    sha256 = "2ec3a5fd1c290f68aed82600299c17a3a698b24c189efb6a4df4ff300aa29233",
)

http_archive(
    name = "proto-lens-protoc",
    urls = ["http://hackage.haskell.org/package/proto-lens-protoc-0.5.0.0/proto-lens-protoc-0.5.0.0.tar.gz"],
    build_file = "//:third_party/proto-lens-protoc.BUILD",
    sha256 = "161dcee2aed780f62c01522c86afce61721cf89c0143f157efefb1bd1fa1d164",
    strip_prefix = "proto-lens-protoc-0.5.0.0",
)

register_toolchains(":protobuf-toolchain")

######################################################
# generated by gazelle

go_repository(
    name = "org_golang_x_sync",
    commit = "e225da77a7e68af35c70ccbf71af2b83e6acac3c",
    importpath = "golang.org/x/sync",
)

go_repository(
    name = "org_golang_google_grpc",
    importpath = "google.golang.org/grpc",
    sum = "h1:wdKvqQk7IttEw92GoRyKG2IDrUIpgpj6H6m81yfeMW0=",
    version = "v1.25.1",
)

go_repository(
    name = "org_golang_x_net",
    importpath = "golang.org/x/net",
    sum = "h1:efeOvDhwQ29Dj3SdAV/MJf8oukgn+8D8WgaCaRMchF8=",
    version = "v0.0.0-20191209160850-c0dbc17a3553",
)

go_repository(
    name = "org_golang_x_text",
    importpath = "golang.org/x/text",
    sum = "h1:tW2bmiBqwgJj/UpqtC8EpXEZVYOwU0yG4iWbprSVAcs=",
    version = "v0.3.2",
)

go_repository(
    name = "org_golang_x_oauth2",
    importpath = "golang.org/x/oauth2",
    sum = "h1:pE8b58s1HRDMi8RDc79m0HISf9D4TzseP40cEA6IGfs=",
    version = "v0.0.0-20191202225959-858c2ad4c8b6",
)

go_repository(
    name = "com_google_cloud_go",
    importpath = "cloud.google.com/go",
    sum = "h1:CH+lkubJzcPYB1Ggupcq0+k8Ni2ILdG2lYjDIgavDBQ=",
    version = "v0.49.0",
)
