[日本語](function_reference-ja.md)

# sqlite3StatisticalLibrary — Function Reference

## Overview

This library is an extension module that makes statistical functions directly available from SQL using SQLite3's `LOAD_EXTENSION` feature.

This reference covers a total of **249 functions**.

| Category | Count | Description |
|---|---|---|
| [Basic Aggregate Functions](ref/basic_aggregates.md) | 24 | Single-column aggregates: `SELECT stat_xxx(col) FROM table` |
| [Parameterized Aggregate Functions](ref/parameterized_aggregates.md) | 20 | Aggregates with parameters: `SELECT stat_xxx(col, param) FROM table` |
| [Two-Column Aggregate Functions](ref/two_column_aggregates.md) | 27 | Two-column input aggregates: `SELECT stat_xxx(col1, col2) FROM table` |
| [Window Functions](ref/window_functions.md) | 23 | Full-scan window functions that return a value per row |
| [Complex Aggregate Functions](ref/complex_aggregates.md) | 32 | Aggregate functions returning JSON results, two-sample tests, survival analysis, etc. |
| [Scalar Functions — Test Helpers](ref/scalar_tests_helpers.md) | 40 | DB-independent: distribution functions, special functions, proportion tests, multiple testing corrections, etc. |
| [Scalar Functions — Distributions & Transformations](ref/scalar_distributions.md) | 83 | DB-independent: additional distribution functions, effect size conversions, power analysis, etc. |

- **Basic Aggregate Functions through Two-Column Aggregate Functions** (71 functions) are all **aggregate functions** and can be used in any context where SQLite3 standard aggregate functions are available, including `GROUP BY`, `HAVING`, and subqueries.
- **Window Functions** (23 functions) are implemented as full-scan type and return one value per row.
- **Complex Aggregate Functions** (32 functions) are **aggregate functions** (including those returning JSON results).
- **Scalar Functions** (123 functions) compute results solely from their parameters.

---

## Loading the Extension

### Loading from sqlite3 CLI

```sql
-- .load command (recommended)
.load ./ext_funcs sqlite3_ext_funcs_init

-- load_extension() function
SELECT load_extension('./ext_funcs', 'sqlite3_ext_funcs_init');
```

### Loading from C/C++ Programs

```cpp
sqlite3_enable_load_extension(db, 1);
sqlite3_load_extension(db, "./ext_funcs.dylib",
                        "sqlite3_ext_funcs_init", &errmsg);
```

> **Note**: The entry point name `sqlite3_ext_funcs_init` must be explicitly specified. SQLite's auto-detection removes `_` characters.

---

## Common Specifications

### NULL Handling

- Rows with `NULL` input values are **ignored** (following SQLite3 aggregate function conventions)
- Returns `NULL` when all rows are `NULL` or the result set is empty
- Returns `NULL` when the computation results in `NaN` or `Inf`

```sql
-- Data with NULL values
CREATE TABLE sample (val REAL);
INSERT INTO sample VALUES (1), (NULL), (3), (NULL), (5);

-- NULLs are ignored; computed over 3 values: 1, 3, 5
SELECT stat_mean(val) FROM sample;
-- → 3.0

-- All rows are NULL
SELECT stat_mean(NULL);
-- → NULL

-- Empty result set
SELECT stat_mean(val) FROM sample WHERE val > 100;
-- → NULL
```

### Minimum Data Count Requirements

Some functions require a minimum number of data points. When insufficient data is provided, `NULL` is returned.

| Function | Minimum Data Count |
|---|---|
| `stat_sample_variance`, `stat_sample_stddev`, `stat_se`, `stat_cv` | 2 |
| `stat_population_skewness`, `stat_skewness` | 3 |
| `stat_population_kurtosis`, `stat_kurtosis` | 4 |
| Others | 1 |

### Using with GROUP BY

All aggregate functions can be used with `GROUP BY`.

```sql
SELECT category,
       stat_mean(score)   AS mean_score,
       stat_median(score) AS median_score,
       stat_sample_stddev(score) AS stddev_score
FROM exam_results
GROUP BY category;
```

---

## Function List (Quick Reference)

### Basic Aggregate Functions (24 functions)

| Function | Description | Return | Min n | Details |
|---|---|---|---|---|
| `stat_mean(col)` | Arithmetic mean | REAL | 1 | [Details](ref/basic_aggregates.md#stat_mean) |
| `stat_median(col)` | Median | REAL | 1 | [Details](ref/basic_aggregates.md#stat_median) |
| `stat_mode(col)` | Mode (smallest value) | REAL | 1 | [Details](ref/basic_aggregates.md#stat_mode) |
| `stat_geometric_mean(col)` | Geometric mean | REAL | 1 | [Details](ref/basic_aggregates.md#stat_geometric_mean) |
| `stat_harmonic_mean(col)` | Harmonic mean | REAL | 1 | [Details](ref/basic_aggregates.md#stat_harmonic_mean) |
| `stat_range(col)` | Range | REAL | 1 | [Details](ref/basic_aggregates.md#stat_range) |
| `stat_var(col)` | Variance (population) | REAL | 1 | [Details](ref/basic_aggregates.md#stat_var) |
| `stat_population_variance(col)` | Population variance | REAL | 1 | [Details](ref/basic_aggregates.md#stat_population_variance) |
| `stat_sample_variance(col)` | Sample variance (unbiased) | REAL | 2 | [Details](ref/basic_aggregates.md#stat_sample_variance) |
| `stat_stdev(col)` | Standard deviation (population) | REAL | 1 | [Details](ref/basic_aggregates.md#stat_stdev) |
| `stat_population_stddev(col)` | Population standard deviation | REAL | 1 | [Details](ref/basic_aggregates.md#stat_population_stddev) |
| `stat_sample_stddev(col)` | Sample standard deviation | REAL | 2 | [Details](ref/basic_aggregates.md#stat_sample_stddev) |
| `stat_cv(col)` | Coefficient of variation | REAL | 2 | [Details](ref/basic_aggregates.md#stat_cv) |
| `stat_iqr(col)` | Interquartile range | REAL | 1 | [Details](ref/basic_aggregates.md#stat_iqr) |
| `stat_mad_mean(col)` | Mean absolute deviation | REAL | 1 | [Details](ref/basic_aggregates.md#stat_mad_mean) |
| `stat_geometric_stddev(col)` | Geometric standard deviation | REAL | 1 | [Details](ref/basic_aggregates.md#stat_geometric_stddev) |
| `stat_population_skewness(col)` | Population skewness | REAL | 3 | [Details](ref/basic_aggregates.md#stat_population_skewness) |
| `stat_skewness(col)` | Sample skewness | REAL | 3 | [Details](ref/basic_aggregates.md#stat_skewness) |
| `stat_population_kurtosis(col)` | Population kurtosis (excess) | REAL | 4 | [Details](ref/basic_aggregates.md#stat_population_kurtosis) |
| `stat_kurtosis(col)` | Sample kurtosis (excess) | REAL | 4 | [Details](ref/basic_aggregates.md#stat_kurtosis) |
| `stat_se(col)` | Standard error | REAL | 2 | [Details](ref/basic_aggregates.md#stat_se) |
| `stat_mad(col)` | Median absolute deviation (MAD) | REAL | 1 | [Details](ref/basic_aggregates.md#stat_mad) |
| `stat_mad_scaled(col)` | Scaled MAD | REAL | 1 | [Details](ref/basic_aggregates.md#stat_mad_scaled) |
| `stat_hodges_lehmann(col)` | Hodges-Lehmann estimator | REAL | 1 | [Details](ref/basic_aggregates.md#stat_hodges_lehmann) |

### Parameterized Aggregate Functions (20 functions)

| Function | Description | Return | Min n | Details |
|---|---|---|---|---|
| `stat_trimmed_mean(col, proportion)` | Trimmed mean | REAL | 1 | [Details](ref/parameterized_aggregates.md#stat_trimmed_mean) |
| `stat_quartile(col)` | Quartiles (Q1, Q2, Q3) | JSON | 1 | [Details](ref/parameterized_aggregates.md#stat_quartile) |
| `stat_percentile(col, p)` | Percentile | REAL | 1 | [Details](ref/parameterized_aggregates.md#stat_percentile) |
| `stat_z_test(col, mu0, sigma)` | One-sample z-test | JSON | 1 | [Details](ref/parameterized_aggregates.md#stat_z_test) |
| `stat_t_test(col, mu0)` | One-sample t-test | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_t_test) |
| `stat_chisq_gof_uniform(col)` | Chi-square goodness-of-fit test | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_chisq_gof_uniform) |
| `stat_shapiro_wilk(col)` | Shapiro-Wilk test | JSON | 3 | [Details](ref/parameterized_aggregates.md#stat_shapiro_wilk) |
| `stat_ks_test(col)` | Lilliefors test (normality) | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_ks_test) |
| `stat_wilcoxon(col, mu0)` | Wilcoxon signed-rank test | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_wilcoxon) |
| `stat_ci_mean(col, confidence)` | Confidence interval for the mean (t) | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_ci_mean) |
| `stat_ci_mean_z(col, sigma, confidence)` | Confidence interval for the mean (z) | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_ci_mean_z) |
| `stat_ci_var(col, confidence)` | Confidence interval for variance | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_ci_var) |
| `stat_moe_mean(col, confidence)` | Margin of error for the mean | REAL | 2 | [Details](ref/parameterized_aggregates.md#stat_moe_mean) |
| `stat_cohens_d(col, mu0)` | Cohen's d (one-sample) | REAL | 2 | [Details](ref/parameterized_aggregates.md#stat_cohens_d) |
| `stat_hedges_g(col, mu0)` | Hedges' g (one-sample) | REAL | 2 | [Details](ref/parameterized_aggregates.md#stat_hedges_g) |
| `stat_acf_lag(col, lag)` | Autocorrelation coefficient | REAL | lag+1 | [Details](ref/parameterized_aggregates.md#stat_acf_lag) |
| `stat_biweight_midvar(col, c)` | Biweight Midvariance | REAL | 1 | [Details](ref/parameterized_aggregates.md#stat_biweight_midvar) |
| `stat_bootstrap_mean(col, n)` | Bootstrap of the mean | JSON | 1 | [Details](ref/parameterized_aggregates.md#stat_bootstrap_mean) |
| `stat_bootstrap_median(col, n)` | Bootstrap of the median | JSON | 1 | [Details](ref/parameterized_aggregates.md#stat_bootstrap_median) |
| `stat_bootstrap_stddev(col, n)` | Bootstrap of standard deviation | JSON | 2 | [Details](ref/parameterized_aggregates.md#stat_bootstrap_stddev) |

### Two-Column Aggregate Functions (27 functions)

| Function | Description | Return | Min n | Details |
|---|---|---|---|---|
| `stat_population_covariance(x, y)` | Population covariance | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_population_covariance) |
| `stat_covariance(x, y)` | Sample covariance | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_covariance) |
| `stat_pearson_r(x, y)` | Pearson correlation coefficient | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_pearson_r) |
| `stat_spearman_r(x, y)` | Spearman rank correlation | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_spearman_r) |
| `stat_kendall_tau(x, y)` | Kendall rank correlation | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_kendall_tau) |
| `stat_weighted_covariance(val, wt)` | Weighted covariance | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_weighted_covariance) |
| `stat_weighted_mean(val, wt)` | Weighted mean | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_weighted_mean) |
| `stat_weighted_harmonic_mean(val, wt)` | Weighted harmonic mean | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_weighted_harmonic_mean) |
| `stat_weighted_variance(val, wt)` | Weighted variance | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_weighted_variance) |
| `stat_weighted_stddev(val, wt)` | Weighted standard deviation | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_weighted_stddev) |
| `stat_weighted_median(val, wt)` | Weighted median | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_weighted_median) |
| `stat_weighted_percentile(val, wt, p)` | Weighted percentile | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_weighted_percentile) |
| `stat_simple_regression(x, y)` | Simple linear regression | JSON | 3 | [Details](ref/two_column_aggregates.md#stat_simple_regression) |
| `stat_r_squared(actual, pred)` | Coefficient of determination R² | REAL | 2 | [Details](ref/two_column_aggregates.md#stat_r_squared) |
| `stat_adjusted_r_squared(actual, pred)` | Adjusted R² | REAL | 3 | [Details](ref/two_column_aggregates.md#stat_adjusted_r_squared) |
| `stat_t_test_paired(x, y)` | Paired t-test | JSON | 2 | [Details](ref/two_column_aggregates.md#stat_t_test_paired) |
| `stat_chisq_gof(obs, exp)` | Chi-square goodness-of-fit test | JSON | 2 | [Details](ref/two_column_aggregates.md#stat_chisq_gof) |
| `stat_mae(actual, pred)` | Mean absolute error | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_mae) |
| `stat_mse(actual, pred)` | Mean squared error | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_mse) |
| `stat_rmse(actual, pred)` | RMSE | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_rmse) |
| `stat_mape(actual, pred)` | Mean absolute percentage error | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_mape) |
| `stat_euclidean_dist(a, b)` | Euclidean distance | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_euclidean_dist) |
| `stat_manhattan_dist(a, b)` | Manhattan distance | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_manhattan_dist) |
| `stat_cosine_sim(a, b)` | Cosine similarity | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_cosine_sim) |
| `stat_cosine_dist(a, b)` | Cosine distance | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_cosine_dist) |
| `stat_minkowski_dist(a, b, p)` | Minkowski distance | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_minkowski_dist) |
| `stat_chebyshev_dist(a, b)` | Chebyshev distance | REAL | 1 | [Details](ref/two_column_aggregates.md#stat_chebyshev_dist) |

### Window Functions (23 functions)

| Function | Description | Return | Details |
|---|---|---|---|
| `stat_rolling_mean(col, window)` | Rolling mean | REAL/row | [Details](ref/window_functions.md#stat_rolling_mean) |
| `stat_rolling_std(col, window)` | Rolling standard deviation | REAL/row | [Details](ref/window_functions.md#stat_rolling_std) |
| `stat_rolling_min(col, window)` | Rolling minimum | REAL/row | [Details](ref/window_functions.md#stat_rolling_min) |
| `stat_rolling_max(col, window)` | Rolling maximum | REAL/row | [Details](ref/window_functions.md#stat_rolling_max) |
| `stat_rolling_sum(col, window)` | Rolling sum | REAL/row | [Details](ref/window_functions.md#stat_rolling_sum) |
| `stat_moving_avg(col, window)` | Simple moving average | REAL/row | [Details](ref/window_functions.md#stat_moving_avg) |
| `stat_ema(col, span)` | Exponential moving average | REAL/row | [Details](ref/window_functions.md#stat_ema) |
| `stat_rank(col)` | Rank transformation | REAL/row | [Details](ref/window_functions.md#stat_rank) |
| `stat_fillna_mean(col)` | Mean imputation | REAL/row | [Details](ref/window_functions.md#stat_fillna_mean) |
| `stat_fillna_median(col)` | Median imputation | REAL/row | [Details](ref/window_functions.md#stat_fillna_median) |
| `stat_fillna_ffill(col)` | Forward fill | REAL/row | [Details](ref/window_functions.md#stat_fillna_ffill) |
| `stat_fillna_bfill(col)` | Backward fill | REAL/row | [Details](ref/window_functions.md#stat_fillna_bfill) |
| `stat_fillna_interp(col)` | Linear interpolation | REAL/row | [Details](ref/window_functions.md#stat_fillna_interp) |
| `stat_label_encode(col)` | Label encoding | REAL/row | [Details](ref/window_functions.md#stat_label_encode) |
| `stat_bin_width(col, n_bins)` | Equal-width binning | REAL/row | [Details](ref/window_functions.md#stat_bin_width) |
| `stat_bin_freq(col, n_bins)` | Equal-frequency binning | REAL/row | [Details](ref/window_functions.md#stat_bin_freq) |
| `stat_lag(col, k)` | Lag | REAL/row | [Details](ref/window_functions.md#stat_lag) |
| `stat_diff(col, order)` | Differencing | REAL/row | [Details](ref/window_functions.md#stat_diff) |
| `stat_seasonal_diff(col, period)` | Seasonal differencing | REAL/row | [Details](ref/window_functions.md#stat_seasonal_diff) |
| `stat_outliers_iqr(col)` | Outlier detection (IQR) | REAL/row | [Details](ref/window_functions.md#stat_outliers_iqr) |
| `stat_outliers_zscore(col)` | Outlier detection (Z-score) | REAL/row | [Details](ref/window_functions.md#stat_outliers_zscore) |
| `stat_outliers_mzscore(col)` | Outlier detection (modified Z) | REAL/row | [Details](ref/window_functions.md#stat_outliers_mzscore) |
| `stat_winsorize(col, pct)` | Winsorization | REAL/row | [Details](ref/window_functions.md#stat_winsorize) |

### Complex Aggregate Functions (32 functions)

| Function | Description | Return | Details |
|---|---|---|---|
| `stat_modes(col)` | Modes (all) | JSON | [Details](ref/complex_aggregates.md#stat_modes) |
| `stat_five_number_summary(col)` | Five-number summary | JSON | [Details](ref/complex_aggregates.md#stat_five_number_summary) |
| `stat_frequency_table(col)` | Frequency table | JSON | [Details](ref/complex_aggregates.md#stat_frequency_table) |
| `stat_frequency_count(col)` | Frequency count for each value | JSON | [Details](ref/complex_aggregates.md#stat_frequency_count) |
| `stat_relative_frequency(col)` | Relative frequency | JSON | [Details](ref/complex_aggregates.md#stat_relative_frequency) |
| `stat_cumulative_frequency(col)` | Cumulative frequency | JSON | [Details](ref/complex_aggregates.md#stat_cumulative_frequency) |
| `stat_cumulative_relative_frequency(col)` | Cumulative relative frequency | JSON | [Details](ref/complex_aggregates.md#stat_cumulative_relative_frequency) |
| `stat_t_test2(grp1, grp2)` | Two-sample t-test | JSON | [Details](ref/complex_aggregates.md#stat_t_test2) |
| `stat_t_test_welch(grp1, grp2)` | Welch t-test | JSON | [Details](ref/complex_aggregates.md#stat_t_test_welch) |
| `stat_chisq_independence(col1, col2)` | Chi-square test of independence | JSON | [Details](ref/complex_aggregates.md#stat_chisq_independence) |
| `stat_f_test(grp1, grp2)` | F-test | JSON | [Details](ref/complex_aggregates.md#stat_f_test) |
| `stat_mann_whitney(grp1, grp2)` | Mann-Whitney U test | JSON | [Details](ref/complex_aggregates.md#stat_mann_whitney) |
| `stat_anova1(val, grp)` | One-way ANOVA | JSON | [Details](ref/complex_aggregates.md#stat_anova1) |
| `stat_contingency_table(col1, col2)` | Contingency table | JSON | [Details](ref/complex_aggregates.md#stat_contingency_table) |
| `stat_cohens_d2(grp1, grp2)` | Cohen's d (two-sample) | REAL | [Details](ref/complex_aggregates.md#stat_cohens_d2) |
| `stat_hedges_g2(grp1, grp2)` | Hedges' g (two-sample) | REAL | [Details](ref/complex_aggregates.md#stat_hedges_g2) |
| `stat_glass_delta(ctrl, trt)` | Glass's Delta | REAL | [Details](ref/complex_aggregates.md#stat_glass_delta) |
| `stat_ci_mean_diff(grp1, grp2)` | CI for two-sample mean difference | JSON | [Details](ref/complex_aggregates.md#stat_ci_mean_diff) |
| `stat_ci_mean_diff_welch(grp1, grp2)` | Welch CI for mean difference | JSON | [Details](ref/complex_aggregates.md#stat_ci_mean_diff_welch) |
| `stat_kaplan_meier(time, event)` | Kaplan-Meier survival curve | JSON | [Details](ref/complex_aggregates.md#stat_kaplan_meier) |
| `stat_nelson_aalen(time, event)` | Nelson-Aalen cumulative hazard | JSON | [Details](ref/complex_aggregates.md#stat_nelson_aalen) |
| `stat_logrank(time, event, grp)` | Log-rank test | JSON | [Details](ref/complex_aggregates.md#stat_logrank) |
| `stat_bootstrap(col, n)` | General bootstrap | JSON | [Details](ref/complex_aggregates.md#stat_bootstrap) |
| `stat_bootstrap_bca(col, n)` | BCa bootstrap | JSON | [Details](ref/complex_aggregates.md#stat_bootstrap_bca) |
| `stat_bootstrap_sample(col)` | Bootstrap sample | JSON | [Details](ref/complex_aggregates.md#stat_bootstrap_sample) |
| `stat_permutation_test2(grp1, grp2)` | Two-sample permutation test | JSON | [Details](ref/complex_aggregates.md#stat_permutation_test2) |
| `stat_permutation_paired(x, y)` | Paired permutation test | JSON | [Details](ref/complex_aggregates.md#stat_permutation_paired) |
| `stat_permutation_corr(x, y)` | Correlation permutation test | JSON | [Details](ref/complex_aggregates.md#stat_permutation_corr) |
| `stat_acf(col, max_lag)` | Autocorrelation function | JSON | [Details](ref/complex_aggregates.md#stat_acf) |
| `stat_pacf(col, max_lag)` | Partial autocorrelation function | JSON | [Details](ref/complex_aggregates.md#stat_pacf) |
| `stat_sample_replace(col, n)` | Sampling with replacement | JSON | [Details](ref/complex_aggregates.md#stat_sample_replace) |
| `stat_sample(col, n)` | Sampling without replacement | JSON | [Details](ref/complex_aggregates.md#stat_sample) |

### Scalar Functions — Test Helpers (40 functions)

| Function | Description | Return | Details |
|---|---|---|---|
| `stat_normal_pdf(x [,mu, sigma])` | Normal distribution PDF | REAL | [Details](ref/scalar_tests_helpers.md#normal-distribution) |
| `stat_normal_cdf(x [,mu, sigma])` | Normal distribution CDF | REAL | [Details](ref/scalar_tests_helpers.md#normal-distribution) |
| `stat_normal_quantile(p [,mu, sigma])` | Normal distribution quantile | REAL | [Details](ref/scalar_tests_helpers.md#normal-distribution) |
| `stat_normal_rand([mu, sigma])` | Normal distribution random variate | REAL | [Details](ref/scalar_tests_helpers.md#normal-distribution) |
| `stat_chisq_pdf(x, df)` | Chi-square distribution PDF | REAL | [Details](ref/scalar_tests_helpers.md#chi-square-distribution) |
| `stat_chisq_cdf(x, df)` | Chi-square distribution CDF | REAL | [Details](ref/scalar_tests_helpers.md#chi-square-distribution) |
| `stat_chisq_quantile(p, df)` | Chi-square distribution quantile | REAL | [Details](ref/scalar_tests_helpers.md#chi-square-distribution) |
| `stat_chisq_rand(df)` | Chi-square distribution random variate | REAL | [Details](ref/scalar_tests_helpers.md#chi-square-distribution) |
| `stat_t_pdf(x, df)` | t-distribution PDF | REAL | [Details](ref/scalar_tests_helpers.md#t-distribution) |
| `stat_t_cdf(x, df)` | t-distribution CDF | REAL | [Details](ref/scalar_tests_helpers.md#t-distribution) |
| `stat_t_quantile(p, df)` | t-distribution quantile | REAL | [Details](ref/scalar_tests_helpers.md#t-distribution) |
| `stat_t_rand(df)` | t-distribution random variate | REAL | [Details](ref/scalar_tests_helpers.md#t-distribution) |
| `stat_f_pdf(x, df1, df2)` | F-distribution PDF | REAL | [Details](ref/scalar_tests_helpers.md#f-distribution) |
| `stat_f_cdf(x, df1, df2)` | F-distribution CDF | REAL | [Details](ref/scalar_tests_helpers.md#f-distribution) |
| `stat_f_quantile(p, df1, df2)` | F-distribution quantile | REAL | [Details](ref/scalar_tests_helpers.md#f-distribution) |
| `stat_f_rand(df1, df2)` | F-distribution random variate | REAL | [Details](ref/scalar_tests_helpers.md#f-distribution) |
| `stat_betainc(a, b, x)` | Regularized incomplete beta function | REAL | [Details](ref/scalar_tests_helpers.md#special-functions) |
| `stat_betaincinv(a, b, p)` | Inverse incomplete beta function | REAL | [Details](ref/scalar_tests_helpers.md#special-functions) |
| `stat_norm_cdf(x)` | Standard normal CDF | REAL | [Details](ref/scalar_tests_helpers.md#special-functions) |
| `stat_norm_quantile(p)` | Standard normal inverse CDF | REAL | [Details](ref/scalar_tests_helpers.md#special-functions) |
| `stat_gammainc_lower(a, x)` | Lower incomplete gamma function | REAL | [Details](ref/scalar_tests_helpers.md#special-functions) |
| `stat_gammainc_upper(a, x)` | Upper incomplete gamma function | REAL | [Details](ref/scalar_tests_helpers.md#special-functions) |
| `stat_gammainc_lower_inv(a, p)` | Inverse incomplete gamma function | REAL | [Details](ref/scalar_tests_helpers.md#special-functions) |
| `stat_z_test_prop(x, n, p0)` | One-sample proportion z-test | JSON | [Details](ref/scalar_tests_helpers.md#stat_z_test_prop) |
| `stat_z_test_prop2(x1, n1, x2, n2)` | Two-sample proportion z-test | JSON | [Details](ref/scalar_tests_helpers.md#stat_z_test_prop2) |
| `stat_bonferroni(p, m)` | Bonferroni correction | REAL | [Details](ref/scalar_tests_helpers.md#multiple-testing-corrections) |
| `stat_bh_correction(p, rank, total)` | BH correction | REAL | [Details](ref/scalar_tests_helpers.md#multiple-testing-corrections) |
| `stat_holm_correction(p, rank, total)` | Holm correction | REAL | [Details](ref/scalar_tests_helpers.md#multiple-testing-corrections) |
| `stat_fisher_exact(a, b, c, d)` | Fisher's exact test | JSON | [Details](ref/scalar_tests_helpers.md#stat_fisher_exact) |
| `stat_odds_ratio(a, b, c, d)` | Odds ratio | REAL | [Details](ref/scalar_tests_helpers.md#categorical-analysis-scalar) |
| `stat_relative_risk(a, b, c, d)` | Relative risk | REAL | [Details](ref/scalar_tests_helpers.md#categorical-analysis-scalar) |
| `stat_risk_difference(a, b, c, d)` | Risk difference | REAL | [Details](ref/scalar_tests_helpers.md#categorical-analysis-scalar) |
| `stat_nnt(a, b, c, d)` | Number needed to treat | REAL | [Details](ref/scalar_tests_helpers.md#categorical-analysis-scalar) |
| `stat_ci_prop(x, n [,conf])` | CI for proportion (Wald) | JSON | [Details](ref/scalar_tests_helpers.md#proportion-confidence-intervals) |
| `stat_ci_prop_wilson(x, n [,conf])` | CI for proportion (Wilson) | JSON | [Details](ref/scalar_tests_helpers.md#proportion-confidence-intervals) |
| `stat_ci_prop_diff(x1, n1, x2, n2 [,conf])` | CI for proportion difference | JSON | [Details](ref/scalar_tests_helpers.md#proportion-confidence-intervals) |
| `stat_aic(ll, k)` | AIC | REAL | [Details](ref/scalar_tests_helpers.md#model-selection) |
| `stat_aicc(ll, n, k)` | AICc | REAL | [Details](ref/scalar_tests_helpers.md#model-selection) |
| `stat_bic(ll, n, k)` | BIC | REAL | [Details](ref/scalar_tests_helpers.md#model-selection) |
| `stat_boxcox(x, lambda)` | Box-Cox transformation | REAL | [Details](ref/scalar_tests_helpers.md#stat_boxcox) |

### Scalar Functions — Distributions & Transformations (83 functions)

| Function | Description | Return | Details |
|---|---|---|---|
| `stat_uniform_pdf/cdf/quantile/rand` | Uniform distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#uniform-distribution) |
| `stat_exponential_pdf/cdf/quantile/rand` | Exponential distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#exponential-distribution) |
| `stat_gamma_pdf/cdf/quantile/rand` | Gamma distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#gamma-distribution) |
| `stat_beta_pdf/cdf/quantile/rand` | Beta distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#beta-distribution) |
| `stat_lognormal_pdf/cdf/quantile/rand` | Log-normal distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#log-normal-distribution) |
| `stat_weibull_pdf/cdf/quantile/rand` | Weibull distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#weibull-distribution) |
| `stat_binomial_pmf/cdf/quantile/rand` | Binomial distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#binomial-distribution) |
| `stat_poisson_pmf/cdf/quantile/rand` | Poisson distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#poisson-distribution) |
| `stat_geometric_pmf/cdf/quantile/rand` | Geometric distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#geometric-distribution) |
| `stat_nbinom_pmf/cdf/quantile/rand` | Negative binomial distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#negative-binomial-distribution) |
| `stat_hypergeom_pmf/cdf/quantile/rand` | Hypergeometric distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#hypergeometric-distribution) |
| `stat_bernoulli_pmf/cdf/quantile/rand` | Bernoulli distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#bernoulli-distribution) |
| `stat_duniform_pmf/cdf/quantile/rand` | Discrete uniform distribution (4 functions) | REAL | [Details](ref/scalar_distributions.md#discrete-uniform-distribution) |
| `stat_binomial_coef(n, k)` | Binomial coefficient | INTEGER | [Details](ref/scalar_distributions.md#combinatorics) |
| `stat_log_binomial_coef(n, k)` | Log binomial coefficient | REAL | [Details](ref/scalar_distributions.md#combinatorics) |
| `stat_log_factorial(n)` | Log factorial | REAL | [Details](ref/scalar_distributions.md#combinatorics) |
| `stat_lgamma(x)` | Log-gamma function | REAL | [Details](ref/scalar_distributions.md#special-functions) |
| `stat_tgamma(x)` | Gamma function | REAL | [Details](ref/scalar_distributions.md#special-functions) |
| `stat_beta_func(a, b)` | Beta function | REAL | [Details](ref/scalar_distributions.md#special-functions) |
| `stat_lbeta(a, b)` | Log-beta function | REAL | [Details](ref/scalar_distributions.md#special-functions) |
| `stat_erf(x)` | Error function | REAL | [Details](ref/scalar_distributions.md#special-functions) |
| `stat_erfc(x)` | Complementary error function | REAL | [Details](ref/scalar_distributions.md#special-functions) |
| `stat_logarithmic_mean(a, b)` | Logarithmic mean | REAL | [Details](ref/scalar_distributions.md#special-functions) |
| `stat_hedges_j(n)` | Hedges correction factor | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_t_to_r(t, df)` | t to r conversion | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_d_to_r(d)` | d to r conversion | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_r_to_d(r)` | r to d conversion | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_eta_squared_ef(ss_eff, ss_total)` | Eta-squared | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_partial_eta_sq(F, df1, df2)` | Partial eta-squared | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_omega_squared_ef(ss_eff, ss_tot, ms_err, df_eff)` | Omega-squared | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_cohens_h(p1, p2)` | Cohen's h | REAL | [Details](ref/scalar_distributions.md#effect-size-conversions) |
| `stat_interpret_d(d)` | Cohen's d interpretation | TEXT | [Details](ref/scalar_distributions.md#effect-size-interpretation) |
| `stat_interpret_r(r)` | Correlation coefficient interpretation | TEXT | [Details](ref/scalar_distributions.md#effect-size-interpretation) |
| `stat_interpret_eta2(eta2)` | Eta-squared interpretation | TEXT | [Details](ref/scalar_distributions.md#effect-size-interpretation) |
| `stat_power_t1(d, n, alpha)` | One-sample power | REAL | [Details](ref/scalar_distributions.md#power-analysis) |
| `stat_n_t1(d, power, alpha)` | One-sample required n | REAL | [Details](ref/scalar_distributions.md#power-analysis) |
| `stat_power_t2(d, n1, n2, alpha)` | Two-sample power | REAL | [Details](ref/scalar_distributions.md#power-analysis) |
| `stat_n_t2(d, power, alpha)` | Two-sample required n | REAL | [Details](ref/scalar_distributions.md#power-analysis) |
| `stat_power_prop(p1, p2, n, alpha)` | Proportion power | REAL | [Details](ref/scalar_distributions.md#power-analysis) |
| `stat_n_prop(p1, p2, power, alpha)` | Proportion required n | REAL | [Details](ref/scalar_distributions.md#power-analysis) |
| `stat_moe_prop(x, n [,conf])` | Margin of error for proportion | REAL | [Details](ref/scalar_distributions.md#sample-size-margin-of-error) |
| `stat_moe_prop_worst(n [,conf])` | Worst-case margin of error | REAL | [Details](ref/scalar_distributions.md#sample-size-margin-of-error) |
| `stat_n_moe_prop(moe [,conf [,p]])` | Required n for proportion estimation | REAL | [Details](ref/scalar_distributions.md#sample-size-margin-of-error) |
| `stat_n_moe_mean(moe, sigma [,conf])` | Required n for mean estimation | REAL | [Details](ref/scalar_distributions.md#sample-size-margin-of-error) |

---

## Practical Usage Examples

### Batch Descriptive Statistics

```sql
SELECT
    COUNT(val)                       AS n,
    stat_mean(val)                   AS mean,
    stat_median(val)                 AS median,
    stat_mode(val)                   AS mode,
    stat_sample_stddev(val)          AS stddev,
    stat_sample_variance(val)        AS variance,
    MIN(val)                         AS min,
    MAX(val)                         AS max,
    stat_range(val)                  AS range,
    stat_iqr(val)                    AS iqr,
    stat_se(val)                     AS se,
    stat_skewness(val)               AS skewness,
    stat_kurtosis(val)               AS kurtosis
FROM measurements;
```

### Comparing Robust and Classical Statistics

```sql
-- Assess the impact of outliers
SELECT
    'classical' AS type,
    stat_mean(val)            AS location,
    stat_sample_stddev(val)   AS spread
FROM data
UNION ALL
SELECT
    'robust' AS type,
    stat_hodges_lehmann(val)  AS location,
    stat_mad_scaled(val)      AS spread
FROM data;
```

### Detailed Analysis by Group

```sql
SELECT
    category,
    COUNT(score) AS n,
    stat_mean(score)              AS mean,
    stat_median(score)            AS median,
    stat_sample_stddev(score)     AS stddev,
    stat_cv(score)                AS cv,
    stat_iqr(score)               AS iqr,
    stat_skewness(score)          AS skewness,
    stat_kurtosis(score)          AS kurtosis,
    stat_mad_scaled(score)        AS robust_spread,
    stat_hodges_lehmann(score)    AS robust_location
FROM exam_results
GROUP BY category
ORDER BY mean DESC;
```

### Quick Normality Assessment

```sql
-- Check normality of multiple columns at once
SELECT
    'height' AS variable,
    stat_skewness(height) AS skewness,
    stat_kurtosis(height) AS kurtosis,
    CASE
        WHEN ABS(stat_skewness(height)) < 2 AND ABS(stat_kurtosis(height)) < 7
        THEN 'approximately normal'
        ELSE 'non-normal'
    END AS normality
FROM people
UNION ALL
SELECT
    'weight',
    stat_skewness(weight),
    stat_kurtosis(weight),
    CASE
        WHEN ABS(stat_skewness(weight)) < 2 AND ABS(stat_kurtosis(weight)) < 7
        THEN 'approximately normal'
        ELSE 'non-normal'
    END
FROM people;
```

### Regression Analysis and Prediction Accuracy

```sql
-- Run regression analysis and evaluate accuracy at once
SELECT
    stat_simple_regression(x, y) AS regression,
    stat_pearson_r(x, y) AS correlation,
    stat_r_squared(x, y) AS r_squared,
    stat_mae(x, y) AS mae,
    stat_rmse(x, y) AS rmse
FROM experiment_data;
```

### Time Series Analysis Pipeline

```sql
-- Comprehensive time series analysis of stock price data
SELECT date, close_price,
       stat_moving_avg(close_price, 20) AS sma_20,
       stat_ema(close_price, 12) AS ema_12,
       stat_rolling_std(close_price, 20) AS volatility,
       stat_diff(close_price, 1) AS daily_change,
       stat_lag(close_price, 1) AS prev_close
FROM stock_prices;
```

### Missing Value Handling and Outlier Removal

```sql
-- Impute missing values -> detect outliers -> analyze clean data
SELECT id, raw_value,
       stat_fillna_interp(raw_value) AS filled,
       stat_outliers_iqr(raw_value) AS is_outlier,
       stat_winsorize(raw_value, 5) AS winsorized
FROM sensor_data;
```
