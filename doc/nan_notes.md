# NULL, NaN and Inf: Notes

How the SQL functions treat missing values (`NULL`), `NaN` and infinities.
statcpp, the library underneath, follows the default missing-value handling of
R since v0.5.0. This extension adds the SQL conventions below on top of it, so
the result in SQL is not always the same as in R or in statcpp.

All examples were run against this version of the extension.

← [Function Reference](function_reference.md)

---

## 1. What SQLite can store

- **NaN cannot be stored.** An expression that evaluates to NaN, such as
  `9e999 - 9e999`, becomes `NULL`, both in a query and when inserted into a
  table. A column therefore never contains NaN: a missing value is `NULL`.
- **Inf can be stored.** `9e999` and `-9e999` are kept as `+Inf` and `-Inf` and
  are treated as ordinary values.

---

## 2. Aggregate functions

This applies to the basic, parameterized, two-column, group-column and complex
aggregates.

- **`NULL` rows are skipped**, as SQLite's own `avg()` does. This corresponds to
  `na.rm = TRUE` in R, not to R's default, under which `mean()` of data with
  `NA` is `NA`.
- **Two-column and group-column functions skip a row in which any of the
  columns is `NULL`** (complete cases). For example `stat_pearson_r(x, y)` uses
  only the rows where both `x` and `y` are present, and `stat_anova1(val, grp)`
  ignores rows whose value or group is `NULL`.
- **All rows `NULL`, or no rows, gives `NULL`.**
- **A result that is NaN or Inf is returned as `NULL`.** The mean of 1, 2 and
  `9e999` is `+Inf` mathematically, but `stat_mean` returns `NULL`. Inside a
  JSON result, a NaN or Inf field is written as `null`.
- **Degenerate data is an error, not `NULL`.** When statcpp rejects the input,
  the statement fails with statcpp's message:

  ```sql
  SELECT stat_t_test(v, 5)
  FROM (SELECT 5 AS v UNION ALL SELECT 5 UNION ALL SELECT 5);
  -- Error: statcpp::t_test: zero variance
  ```

---

## 3. Parameters: `NULL` gives `NULL`

Following SQL, a `NULL` parameter makes the result `NULL`. It is neither read as
0 nor replaced by the default:

```sql
SELECT stat_percentile(v, NULL) FROM t;         -- NULL
SELECT stat_t_test(v, NULL) FROM t;             -- NULL
SELECT stat_tukey_hsd(val, grp, NULL) FROM g;   -- NULL
```

- **Window functions** return `NULL` on every row when a parameter is `NULL`,
  as SQL's `lag(v, NULL)` does: `stat_rolling_mean(v, NULL)` and
  `stat_lag(v, NULL)` are `NULL` throughout.
- **To use the default, omit the parameter** where the function allows it,
  such as the trailing `alpha` of the post-hoc tests:
  `stat_tukey_hsd(val, grp)` uses `alpha` = 0.05.
- **A parameter of 0 is read as "use the default"** by the functions that have
  one, so `stat_tukey_hsd(val, grp, 0)` also uses `alpha` = 0.05, and a window
  size or lag of 0 or less becomes 1.
- **An aggregate reads its parameters from the first row whose value is not
  `NULL`**, and ignores them on later rows. Pass constants, not columns, as
  parameters.

---

## 4. Scalar functions: a `NULL` argument gives `NULL`

As with SQLite's built-in functions such as `abs(NULL)`, **a scalar function
returns `NULL` when any argument is `NULL`**:

```sql
SELECT stat_normal_cdf(NULL);              -- NULL
SELECT stat_binomial_pmf(3, 10, NULL);     -- NULL
SELECT stat_normal_pdf(0.5, NULL, 1.0);    -- NULL
SELECT stat_normal_pdf(0.5);               -- 0.352..., mu = 0 and sigma = 1
```

Optional arguments, such as `mu` and `sigma` of the normal-distribution
functions, take their defaults only when they are left out, not when they are
`NULL`.

Inf arguments are passed to statcpp as they are: `stat_t_cdf(9e999, 5)` is
`1.0`.

---

## 5. Window functions

Window functions return one value per row and need an `OVER` clause:

```sql
SELECT id, stat_rolling_mean(val, 3) OVER (
    ORDER BY id ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING)
FROM t;
```

Called without `OVER`, they run as an aggregate and return a single value,
which is not meaningful.

How `NULL` rows affect the result depends on the kind of function. The
examples use `val` = 10, 20, `NULL`, 40, 50, 30 (rows 1-6).

| Kind | Effect of a `NULL` row |
| --- | --- |
| Rolling | A window containing it is `NULL` |
| Cumulative | Every later row is `NULL` |
| Shift | Moves along like a value |
| Per row | That row is `NULL` |
| Correction | That row is `NULL` |
| Imputation | That row is filled |

- Rolling: `stat_rolling_*`, `stat_moving_avg`
- Cumulative: `stat_ema`
- Shift: `stat_lag`, `stat_diff`, `stat_seasonal_diff`
- Per row: `stat_rank`, `stat_label_encode`, `stat_bin_width`, `stat_bin_freq`,
  `stat_outliers_*`, `stat_winsorize`
- Correction: `stat_bonferroni`, `stat_bh_correction`, `stat_holm_correction`
- Imputation: `stat_fillna_*`

- **Rolling.** Row i holds the window that ends at row i, and the first n - 1
  rows are `NULL`. `stat_rolling_mean(val, 3)` gives `NULL` for rows 1-5 and
  40 for row 6.
- **Cumulative.** `stat_ema` carries its state from row to row, so after the
  first `NULL` every row is `NULL`: `stat_ema(val, 3)` gives 10, 15, then
  `NULL` for rows 3-6. If the first row is `NULL`, every row is `NULL`.
- **Shift.** `stat_lag(val, 1)` gives `NULL`, 10, 20, `NULL`, 40, 50, like SQL's
  `lag()`. `stat_diff(val, 1)` is `NULL` wherever either operand is `NULL`.
- **Per row.** The statistic is computed from the non-`NULL` rows, and the
  `NULL` rows return `NULL`: `stat_rank(val)` gives 1, 2, `NULL`, 4, 5, 3.
- **Correction.** As R's `p.adjust()` does, `NULL` p-values are not counted in
  the number of tests.
- **Imputation.** `stat_fillna_mean` and `stat_fillna_median` fill with the
  mean or median of the observed values. `stat_fillna_ffill`,
  `stat_fillna_bfill` and `stat_fillna_interp` cannot fill a `NULL` at the start
  (ffill, interp) or at the end (bfill, interp), which stays `NULL`.

### Inf in rolling windows

`stat_rolling_mean` and `stat_rolling_sum` keep a running sum, so after an Inf
every window is `NULL`, as with R's `zoo::rollmean()`. `stat_moving_avg` sums
each window separately, as R's `stats::filter()` does, so only the windows that
contain the Inf are affected:

| `v` | 1 | `9e999` | 2 | 3 |
| --- | --- | --- | --- | --- |
| `stat_rolling_mean(v, 2)` | `NULL` | Inf | Inf | `NULL` |
| `stat_moving_avg(v, 2)` | `NULL` | Inf | Inf | 2.5 |

Unlike the aggregates, the window functions return Inf as Inf; only NaN
becomes `NULL`.

---

## 6. Relation to statcpp

statcpp's own rules are in its `docs/NAN_POLICY.md`. Because this extension
removes `NULL` before calling statcpp in the aggregate, two-column and scalar
paths, statcpp's NaN handling matters only for the window functions, where
`NULL` rows are passed to statcpp as NaN.
