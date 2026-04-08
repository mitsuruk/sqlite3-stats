# Changelog

This document records the change history of sqlite3StatisticalLibrary.

This project follows [Semantic Versioning](https://semver.org/).

## [Unreleased]

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
