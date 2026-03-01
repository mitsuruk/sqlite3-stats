# Changelog

This document records the change history of sqlite3StatisticalLibrary.

This project follows [Semantic Versioning](https://semver.org/).

## [0.2.1] - 2026-03-01

### Changed

- **`ext_funcs.cpp` — `calc_ks_test()`**: Updated internal call from `statcpp::ks_test_normal()` to `statcpp::lilliefors_test()` to follow the upstream rename in statcpp v0.1.3. The SQL function name `stat_ks_test()` is unchanged.
- **`ext_funcs.cpp` — `calc_bootstrap_mean()` / `calc_bootstrap_median()` / `calc_bootstrap_stddev()` / `calc_bootstrap_generic()` / `calc_bootstrap_bca()`**: Changed the `n_bootstrap` guard from `n == 0` to `n < 2` to prevent `std::invalid_argument` thrown by statcpp v0.1.3 when `n_bootstrap` is 0 or 1. Falls back to `n = 1000` when `n < 2`.

### Documentation

- **`function_reference.md` / `function_reference-ja.md`**: Updated `stat_ks_test` description from "KS test" to "Lilliefors test".
- **`sqlite3lib_LOAD_EXTENSION.md` / `sqlite3lib_LOAD_EXTENSION-ja.md`**: Updated `stat_ks_test` description from "KS test" to "Lilliefors test".
- **`ref/parameterized_aggregates.md` / `ref/parameterized_aggregates-ja.md`**: Rewrote `stat_ks_test` section to clarify that the function performs a Lilliefors test (parameters estimated from data), not a standard KS test with known parameters. Updated SQL examples accordingly.

### Dependencies

- **statcpp**: Updated from v0.1.2 to v0.1.3 (re-downloaded from GitHub `main` branch).

---

[0.2.1]: https://github.com/mitsuruk/sqlite3-stats/releases/tag/v0.2.1
