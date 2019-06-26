## General file style

- General style guidelines -- like newlines at the end of file -- are linted
for using [pre-commit](https://pre-commit.com/)
- The rules enfornced are available in the
repo's [.pre-commit-config.yaml file](.pre-commit-config.yaml).
- You can install pre-commit locally with these [instructions](https://pre-commit.com/#install).
- The linter may be run locally using `pre-commit run --all-files`.
Additionally it can be installed as a commit hook with `pre-commit install`.

## C++ coding style

- C++ code uses the [Envoy style guide](https://github.com/envoyproxy/envoy/blob/master/STYLE.md)
- Code is auto-formatted using `clang-format` with [these rules](./.clang-format)

## Java coding style

- Java code is auto-formatted using `clang-format` with [these rules](./.clang-format)

## Objective-C coding style

- Objective-C code is auto-formatted using `clang-format` with [these rules](./.clang-format)

## Kotlin coding style

- Kotlin code style is validated using
  [detekt](https://github.com/arturbosch/detekt)
- The rules enforced are available in the repo's [.kotlinlint.yml file](./.kotlinlint.yml).
  We build upon the default config provided by
  [`detekt`](https://github.com/arturbosch/detekt/blob/master/detekt-cli/src/main/resources/default-detekt-config.yml)

## Swift coding style

- Swift code style is validated using [SwiftLint](https://github.com/realm/swiftlint)
- The rules enforced are available in the repo's [.swiftlint.yml file](./.swiftlint.yml)
- The linter may be run locally using `swiftlint` or auto-corrected with `swiftlint autocorrect`
