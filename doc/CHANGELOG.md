# Changelog

This document records the change history of sqlite3StatisticalLibrary.

This project follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added

- **Nine group-column functions**, taking a value column and a group column in the same form as
  `stat_anova1`. Groups are numbered 0, 1, 2, … in ascending order of the group column's values.
  The SQL function count goes from 249 to 258.

  | Function | Description |
  |---|---|
  | `stat_kruskal_wallis(val, grp)` | Kruskal-Wallis test |
  | `stat_levene(val, grp)` | Levene test for homogeneity of variance |
  | `stat_bartlett(val, grp)` | Bartlett test for homogeneity of variance |
  | `stat_cohens_f(val, grp)` | Cohen's f, the one-way ANOVA effect size |
  | `stat_tukey_hsd(val, grp [,alpha])` | Tukey HSD post-hoc, all pairs |
  | `stat_bonferroni_posthoc(val, grp [,alpha])` | Bonferroni post-hoc, all pairs |
  | `stat_scheffe_posthoc(val, grp [,alpha])` | Scheffe post-hoc, all pairs |
  | `stat_dunnett_posthoc(val, grp [,ctrl, alpha])` | Dunnett post-hoc, against a control |
  | `stat_stratified_sample(val, grp [,ratio])` | Stratified random sample |

  `stat_levene` and `stat_bartlett` close a practical gap: the equal-variance assumption behind
  `stat_anova1` and `stat_t_test2` could not be checked from SQL at all. Levene uses the
  median-based Brown-Forsythe form, matching the default of R's `car::leveneTest()`; Bartlett
  matches R's `bartlett.test()`. `stat_kruskal_wallis` matches R's `kruskal.test()` and
  `stat_tukey_hsd` matches R's `TukeyHSD()`, though statcpp reports a comparison as
  group1 - group2 with group1 the lower index, so the mean difference and interval bounds are
  negated relative to R's "2-1" convention. `stat_dunnett_posthoc` uses a Bonferroni
  approximation rather than the exact multivariate t distribution.

  The post-hoc functions compute the one-way ANOVA internally, so they are called on the raw
  value and group columns. Optional parameters may be omitted from the right: `alpha` defaults
  to 0.05, Dunnett's control group index to 0, and the sampling ratio to 0.5.

  ```sql
  -- Check the assumption, then run the test, then locate the differences
  SELECT stat_levene(score, class_id)   AS levene,
         stat_anova1(score, class_id)   AS anova,
         stat_tukey_hsd(score, class_id) AS posthoc
  FROM exam_results;
  ```

  Implemented with the existing two-column aggregate templates plus one new
  `TwoColumnParamAggregateText` (two columns and parameters returning JSON), which mirrors the
  existing `TwoColumnParamAggregate`. The group-splitting code was extracted from `calc_anova1`
  into a shared `split_by_group()` helper.

### Fixed

- **`stat_bh_correction` and `stat_holm_correction` returned incorrect adjusted p-values**: both
  reimplemented the correction formula locally instead of delegating to statcpp, and omitted the
  monotonicity step that both procedures require. BH takes a cumulative minimum over p-values in
  descending order and Holm a cumulative maximum in ascending order; the per-row formulas
  `min(p * total / rank, 1)` and `min(p * (total - rank + 1), 1)` cannot express either, because
  an adjusted value depends on the other p-values in the set. For `p = (0.040, 0.041, 0.042)`:

  ```text
  p        BH (was)   BH (now)   Holm (was)   Holm (now)
  0.040    0.120      0.042      0.120        0.120
  0.041    0.0615     0.042      0.082        0.120
  0.042    0.042      0.042      0.042        0.120
  ```

  R's `p.adjust()` gives 0.042 for all three under BH and 0.12 under Holm, matching the new
  column. The Holm error was anti-conservative: the largest p-value was adjusted to 0.042 rather
  than 0.12, turning a non-significant result into a false positive at α = 0.05.

  Both are now full-scan window functions taking a single argument, delegating to
  `statcpp::benjamini_hochberg_correction()` and `statcpp::holm_correction()`. The three-argument
  scalar forms have been **removed**; a caller no longer supplies `rank` and `total`:

  ```sql
  -- before (removed)
  SELECT stat_bh_correction(p, ROW_NUMBER() OVER (ORDER BY p), COUNT(*) OVER ()) FROM t;

  -- now
  SELECT stat_bh_correction(p) OVER (
      ORDER BY id ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING
  ) FROM t;
  ```

  NULL rows are excluded from the correction and stay NULL in the output. `stat_bonferroni(p, m)`
  was already correct and is unchanged; a one-argument window form `stat_bonferroni(p)` was added
  for consistency. The total SQL function count is unchanged at 249.

- **An exception from any callback terminated the host process**: `ext_funcs.cpp` contained no
  exception handling at all. statcpp reports an argument outside a function's domain by throwing
  `std::invalid_argument`, and SQLite invokes its callbacks across a C ABI, so the exception
  unwound C frames and reached `std::terminate`. A plain SQL expression was enough to kill any
  process that had loaded the extension:

  ```text
  sqlite> SELECT stat_poisson_quantile(1.5, 2.5);
  libc++abi: terminating due to uncaught exception of type std::invalid_argument
  ```

  Every callback body now runs through `invoke_guarded()`, which converts an exception into an
  ordinary SQL error via `sqlite3_result_error()`. The statement fails, the connection stays
  usable, and the process survives. The guard is applied at the registration boundary rather than
  at each call site: `register_scalar()` and `register_scalar_nd()` take the implementation as a
  non-type template parameter and always install a guarded stub, so an unguarded scalar function
  cannot be registered. The eight aggregate and window templates and `stat_logrank` wrap their
  `xStep`, `xValue` and `xFinal` bodies directly.
- **Aggregate state leaked when a computation threw**: each `xFinal` released its heap-allocated
  state by calling `cleanupState()` on the normal path, which an exception skipped. Release is now
  handled by the RAII guard `AggregateStateGuard`, so the state is destroyed on every path.
- **`stat_poisson_quantile()`, `stat_geometric_quantile()` and `stat_nbinom_quantile()` returned
  `-1` at q = 1.0**: these distributions have unbounded support, so no finite value satisfies
  q = 1.0. statcpp signals this with the largest representable unsigned value, which the wrapper
  cast straight to a signed integer. They now return `NULL`. The binomial, hypergeometric and
  discrete uniform quantiles have bounded support and are unchanged.

  Results for every valid argument are unchanged: all 266 integration examples produce identical
  output before and after, apart from the functions that draw random numbers.

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
