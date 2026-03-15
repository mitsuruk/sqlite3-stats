# Changelog

This document records the change history of sqlite3StatisticalLibrary.

This project follows [Semantic Versioning](https://semver.org/).

## [0.2.0] - 2026-03-13

### Changed

- **`ext_funcs.cpp` — `calc_ks_test()`**: Updated internal call from `statcpp::ks_test_normal()` to `statcpp::lilliefors_test()` to follow the upstream rename in statcpp. The SQL function name `stat_ks_test()` is unchanged.
- **`ext_funcs.cpp` — Bootstrap functions**: Changed the `n_bootstrap` guard from `n == 0` to `n < 2` to prevent `std::invalid_argument` when `n_bootstrap` is 0 or 1. Falls back to `n = 1000` when `n < 2`.
- **`ext_funcs.cpp` — Weighted functions**: Migrated from deprecated 3-argument weighted API to new 4-argument overloads (with `weight_last`).

### Documentation

- **`function_reference.md` / `function_reference-ja.md`**: Updated `stat_ks_test` description from "KS test" to "Lilliefors test".
- **`sqlite3lib_LOAD_EXTENSION.md` / `sqlite3lib_LOAD_EXTENSION-ja.md`**: Updated `stat_ks_test` description from "KS test" to "Lilliefors test".
- **`ref/parameterized_aggregates.md` / `ref/parameterized_aggregates-ja.md`**: Rewrote `stat_ks_test` section to clarify that the function performs a Lilliefors test.

### Dependencies

- **statcpp**: v0.2.0
