# Changelog

This document records the change history of sqlite3StatisticalLibrary.

This project follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Dependencies

- **statcpp**: v0.3.0 -> v0.4.0. The upgrade changes the value returned by four SQL functions;
  no source change was needed in this library. All 388 Google Test cases and all 266 integration
  examples still pass.
  - **`stat_ks_test()`**: The Lilliefors p-value is now computed from the Dallal and Wilkinson
    (1986) analytic approximation instead of the previous `2 exp(-2 d_adj^2)` form. The old
    formula understated the p-value badly in the upper range: for `val` 1-10 it returned
    `0.0970` where the correct value is `1`. Samples that are consistent with normality are no
    longer reported near the 0.05 threshold. Decisions at p <= 0.10 are unaffected in direction.
  - **`stat_shapiro_wilk()`**: For a sample with W at or extremely close to 1, the upstream
    sentinel used to yield p = 0.00135 -- rejecting normality for perfectly normal data. It now
    yields p -> 1. Ordinary samples are unaffected.
  - **`stat_norm_cdf()` / `stat_normal_cdf()`**: Evaluated through `erfc` instead of
    `0.5 (1 + erf(x / sqrt(2)))`, which cancelled catastrophically in the left tail. The old form
    lost all significance below x = -5.8 and underflowed to exactly 0 below x = -8.33;
    `stat_norm_cdf(-9.0)` returned `0.0` and now returns `1.1285884059538e-19`. Values near the
    centre move by at most one or two units in the last place.
  - **Upper-tail p-values** of `stat_z_test()`, `stat_z_test_prop()`, `stat_z_test_prop2()`,
    `stat_mann_whitney()`, `stat_wilcoxon()` and the power functions are now formed with the
    survival function rather than by subtracting the CDF from 1, so they stay accurate far into
    the tail. In the ordinary range the change is at the last digit.
  - **`stat_poisson_quantile()` / `stat_nbinom_quantile()` at p = 1.0**: These have unbounded
    support, so there is no finite quantile. Upstream previously cast an infinite value to an
    unsigned integer, which is undefined behaviour; it now returns the largest representable
    value. Through the SQL wrapper this surfaces as `-1` rather than the former indeterminate
    figure (`-1000001` on this platform).

### Documentation

- **`doc/ref/parameterized_aggregates.md` / `-ja.md`**: Documented the range of validity of the
  `stat_ks_test()` p-value -- the underlying approximation is published for p <= 0.10, so a value
  above that (frequently exactly 1) indicates consistency with normality rather than an accurate
  probability.
- **`cmake/statcpp.cmake`, `README.md`, `README-ja.md`, `doc/index.md`**: Corrected the statcpp
  function count from 524 to 386, matching the count published upstream in statcpp 0.4.0. The 249
  SQL functions this extension exposes are unchanged.

### Added

- **Windows (MSVC) support**: The extension now builds and runs on Windows with MSVC (Visual Studio 2022+).
  - `ext_funcs.dll` is produced on Windows alongside `.dylib` (macOS) and `.so` (Linux).
  - `__declspec(dllexport)` added to `sqlite3_ext_funcs_init` for proper DLL export.
  - `RUNTIME_OUTPUT_DIRECTORY` added for correct `.dll` placement with the Visual Studio generator.
  - Compiler flags conditioned on `$<CXX_COMPILER_ID:MSVC>`: `/O2 /W4`, `/utf-8`, `/EHsc`, `NOMINMAX`.
  - Post-build steps (`compile_commands.json` copy, `.cache` removal) guarded with `if(NOT CMAKE_GENERATOR MATCHES "Visual Studio")`.
  - `EXT_FUNCS_PATH` changed to `$<TARGET_FILE:ext_funcs>` to resolve the correct path across generators and configurations.
  - CI matrix extended with `windows-latest` runner.
- **`tests/window_functions_test.cpp`**: Added `#include <algorithm>` (required for `std::sort` on MSVC).

### Known Limitations

- **Google Test suite (`-DSTAT_TESTS=ON`) is not supported on Windows (MSVC)** due to DLL boundary issues with dynamically linked GTest. Integration tests (`a.out.exe`, 266 tests) are fully supported.

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
