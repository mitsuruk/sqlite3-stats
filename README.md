# Statistical Functions for SQLite3

A loadable extension that adds **249 statistical functions** to SQLite3.

[![CI](https://github.com/mitsuruk/sqlite3-stats/actions/workflows/ci.yml/badge.svg)](https://github.com/mitsuruk/sqlite3-stats/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-online-brightgreen.svg)](https://mitsuruk.github.io/sqlite3-stats/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/platform-macOS%20%7C%20Linux-lightgrey.svg)](https://github.com/mitsuruk/sqlite3-stats/actions/workflows/ci.yml)
[![Tests](https://img.shields.io/badge/tests-266-brightgreen.svg)](https://github.com/mitsuruk/sqlite3-stats/actions/workflows/ci.yml)

[日本語](README-ja.md)

## Overview

sqlite3-stats (formerly sqlite3StatisticalLibrary) is a SQLite3 loadable extension that provides 249 statistical functions callable directly from SQL. Built on [statcpp](https://github.com/mitsuruk/statcpp) (a C++17 header-only statistics library with 524 functions), this extension exposes a curated subset of statistical capabilities as native SQL functions.

All 249 functions are verified by 266 integration tests.

### Key Features

- **249 SQL functions** covering descriptive statistics, hypothesis testing, distributions, resampling, and more
- **Zero dependencies at runtime** — a single shared library (`.dylib` / `.so` / `.dll`)
- **Standard SQL interface** — use with `SELECT`, `GROUP BY`, `HAVING`, subqueries, etc.
- **Window function support** — rolling statistics, moving averages, outlier detection
- **JSON output** — complex results (multi-value, frequency tables, test results) returned as JSON
- **Cross-platform** — macOS and Linux

### Function Categories

| Category | Count | Description |
| --- | --- | --- |
| Basic aggregates | 24 | Single-column: `SELECT stat_xxx(col) FROM t` |
| Parameterized aggregates | 20 | With parameters: `SELECT stat_xxx(col, param) FROM t` |
| Two-column aggregates | 27 | Two-column input: `SELECT stat_xxx(col1, col2) FROM t` |
| Window functions | 23 | Full-scan window functions returning one value per row |
| Complex aggregates | 32 | JSON-returning aggregates, two-sample tests, survival analysis |
| Scalar — tests/helpers | 40 | Distribution functions, proportion tests, multiple testing corrections |
| Scalar — distributions | 83 | Continuous/discrete distributions, effect size conversions, power analysis |

## Quick Start

### Prerequisites

- CMake 3.20+
- C++17 compiler (Apple Clang 17+, GCC 13+)
- SQLite3 (downloaded automatically by CMake)
- [statcpp](https://github.com/mitsuruk/statcpp) (downloaded automatically by CMake)

### Build

```bash
git clone https://github.com/mitsuruk/sqlite3-stats.git
cd sqlite3-stats
mkdir build && cd build
cmake ..
make
```

This produces:

- `ext_funcs.dylib` (macOS) or `ext_funcs.so` (Linux) — the loadable extension
- `a.out` — test runner (266 tests)

### Run Tests

```bash
./a.out
# [OK] All tests completed (266 tests).
```

### Load the Extension

#### From the sqlite3 CLI

```sql
-- .load command (recommended)
.load ./ext_funcs sqlite3_ext_funcs_init

-- Or via load_extension()
SELECT load_extension('./ext_funcs', 'sqlite3_ext_funcs_init');
```

#### From C/C++

```cpp
sqlite3_enable_load_extension(db, 1);
sqlite3_load_extension(db, "./ext_funcs.dylib",
                        "sqlite3_ext_funcs_init", &errmsg);
```

> **Note**: The entry point `sqlite3_ext_funcs_init` must be specified explicitly. SQLite's auto-detection strips underscores and will not find it automatically.

## Usage Examples

### Descriptive Statistics

```sql
SELECT stat_mean(score), stat_stdev(score), stat_median(score)
FROM exam_results;

SELECT class, stat_mean(score), stat_cv(score)
FROM exam_results
GROUP BY class;
```

### Hypothesis Testing

```sql
-- One-sample t-test (H0: mu = 50)
SELECT stat_t_test(score, 50) FROM exam_results;

-- Two-sample Welch's t-test (returns JSON)
SELECT stat_t_test_welch(value, group_id) FROM measurements;
```

### Window Functions

```sql
SELECT date, price,
       stat_rolling_mean(price, 7) OVER (ORDER BY date) AS ma7,
       stat_ema(price, 0.3)        OVER (ORDER BY date) AS ema
FROM stock_prices;
```

### Distribution Functions (Scalar)

```sql
-- Normal distribution quantile (z-value for 95%)
SELECT stat_normal_quantile(0.975, 0, 1);  -- 1.96

-- Power analysis: required sample size for two-sample t-test
SELECT stat_n_t2(0.5, 0.05, 0.80);  -- effect size=0.5, alpha=0.05, power=0.80
```

## Common Specifications

- **NULL handling**: NULL values are silently ignored in all aggregate/window functions.
- **Insufficient data**: Returns `NULL` when there are not enough non-NULL values (e.g., variance requires n >= 2).
- **Numeric precision**: IEEE 754 double precision (64-bit). `NaN` and `Inf` are mapped to `NULL`.
- **JSON output**: Complex aggregates return results as JSON strings (objects or arrays).

## Function Reference

Full documentation for all 249 functions:

- [Function Reference (hub)](doc/function_reference.md)
  - [Basic Aggregates (24)](doc/ref/basic_aggregates.md)
  - [Parameterized Aggregates (20)](doc/ref/parameterized_aggregates.md)
  - [Two-Column Aggregates (27)](doc/ref/two_column_aggregates.md)
  - [Window Functions (23)](doc/ref/window_functions.md)
  - [Complex Aggregates (32)](doc/ref/complex_aggregates.md)
  - [Scalar — Tests/Helpers (40)](doc/ref/scalar_tests_helpers.md)
  - [Scalar — Distributions/Transforms (83)](doc/ref/scalar_distributions.md)

## Project Structure

```text
sqlite3-stats/
├── CMakeLists.txt
├── README.md                              # This file (English)
├── README-ja.md                           # Japanese README
├── src/
│   ├── ext_funcs.cpp                      # Extension: 249 SQL functions
│   ├── main.cpp                           # Test runner (266 tests)
│   └── include/                           # Local headers
├── doc/
│   ├── function_reference.md              # Function reference hub (English)
│   ├── function_reference-ja.md           # Function reference hub (Japanese)
│   ├── sqlite3lib_LOAD_EXTENSION.md       # Extension implementation guide (English)
│   ├── sqlite3lib_LOAD_EXTENSION-ja.md    # Extension implementation guide (Japanese)
│   └── ref/                               # Per-category function details
│       ├── basic_aggregates.md            # English
│       ├── basic_aggregates-ja.md         # Japanese
│       ├── parameterized_aggregates.md
│       ├── parameterized_aggregates-ja.md
│       ├── two_column_aggregates.md
│       ├── two_column_aggregates-ja.md
│       ├── window_functions.md
│       ├── window_functions-ja.md
│       ├── complex_aggregates.md
│       ├── complex_aggregates-ja.md
│       ├── scalar_tests_helpers.md
│       ├── scalar_tests_helpers-ja.md
│       ├── scalar_distributions.md
│       └── scalar_distributions-ja.md
├── cmake/
│   ├── sqlite3.cmake                      # SQLite3 auto-download
│   └── statcpp.cmake                      # statcpp auto-download
└── download/                              # Auto-downloaded dependencies
    ├── sqlite3/
    └── statcpp/
```

## Related Projects

sqlite3-stats is part of the **statcpp family**. Choose the one that fits your use case:

| Use case | Repository | Description |
| --- | --- | --- |
| C++ library | [statcpp](https://github.com/mitsuruk/statcpp) | C++17 header-only statistics library (524 functions) |
| UNIX CLI | [statcppCLI](https://github.com/mitsuruk/statcppCLI) | Command-line interface for UNIX pipelines |
| SQL (SQLite3) | **sqlite3-stats** (this repo) | SQLite3 loadable extension (249 functions) |

**Required statcpp version**: v0.1.0 or later

## Tested Environments

- macOS + Apple Clang 17.0.0
- macOS + GCC 15 (Homebrew)

## License

This project is licensed under the MIT License.

## Acknowledgments

This project uses the following tools and AI assistants:

- **[statcpp](https://github.com/mitsuruk/statcpp)** — C++17 header-only statistics library (524 functions)
- **Claude Code for VS Code (Opus 4.5/4.6)** — Code generation, refactoring, documentation
- **OpenAI ChatGPT 5.2** — Documentation review
- **LM Studio google/gemma-2-27b** — Documentation review

---

**Disclaimer**: This library is not intended to replace commercial statistical software in terms of numerical stability or edge-case handling. When using results in research or production, we recommend cross-validating with established tools such as R or Python/SciPy.
