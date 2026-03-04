# Statistical Functions for SQLite3

A loadable extension that adds **249 statistical functions** to SQLite3.

## Overview

sqlite3StatisticalLibrary is a SQLite3 loadable extension that provides 249 statistical functions callable directly from SQL. Built on [statcpp](https://github.com/mitsuruk/statcpp) (a C++17 header-only statistics library with 524 functions), this extension exposes a curated subset of statistical capabilities as native SQL functions.

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

### Build

```bash
git clone https://github.com/mitsuruk/sqlite3StatisticalLibrary.git
cd sqlite3StatisticalLibrary
mkdir build && cd build
cmake ..
make
```

### Load the Extension

```sql
.load ./ext_funcs sqlite3_ext_funcs_init
```

### Usage Examples

```sql
-- Descriptive statistics
SELECT stat_mean(score), stat_stdev(score), stat_median(score)
FROM exam_results;

-- One-sample t-test
SELECT stat_t_test(score, 50) FROM exam_results;

-- Window function: rolling mean
SELECT date, price,
       stat_rolling_mean(price, 7) OVER (ORDER BY date) AS ma7
FROM stock_prices;

-- Distribution function
SELECT stat_normal_quantile(0.975, 0, 1);  -- 1.96
```

## Documentation

- [Function Reference](function_reference.md) — All 249 functions overview
- [Load Extension Guide](sqlite3lib_LOAD_EXTENSION.md) — How to load the extension

## License

MIT License
