[日本語](sqlite3lib_LOAD_EXTENSION-ja.md)

# SQLite3 Loadable Extension Guide

An implementation guide for SQLite3 runtime extensions (Loadable Extensions).
This project implements the extension library in C++ (`ext_funcs.cpp`).

---

## Table of Contents

1. [What is a Loadable Extension](#1-what-is-a-loadable-extension)
2. [Project Structure](#2-project-structure)
3. [Build and Run](#3-build-and-run)
4. [How Loadable Extensions Work](#4-how-loadable-extensions-work)
5. [Extension Function Implementation Patterns](#5-extension-function-implementation-patterns)
6. [SQLite C API Reference](#6-sqlite-c-api-reference)
7. [C++ Implementation Techniques](#7-c-implementation-techniques)
8. [Dynamic Library Considerations](#8-dynamic-library-considerations)

---

## 1. What is a Loadable Extension

### Overview

An SQLite3 Loadable Extension is a mechanism for **dynamically adding custom functionality at runtime** to SQLite.
By loading a dynamic library (`.dylib` / `.so` / `.dll`) written in C++ via
`sqlite3_load_extension()`, you can extend SQLite's functionality without recompiling the SQLite core.

### Capabilities

| Feature | Description | Example |
|---|---|---|
| **Scalar functions** | Functions that return a single value for given input values | `square(x)`, `upper(s)` |
| **Aggregate functions** | Functions that aggregate multiple rows and return a single value | `my_sum(x)`, `sha256_agg(s)` |
| **Window functions** | Aggregate functions used with OVER clauses (SQLite 3.25+) | Custom moving average |
| **Collations** | Define string comparison rules | Japanese sort order |
| **Virtual Tables** | Expose external data sources as tables | CSV, JSON, REST API |
| **VFS (Virtual File System)** | Customize the file I/O layer | Encryption, memory FS |
| **Overriding existing functions** | Replace built-in functions with custom implementations | Multilingual `upper()` |
| **Batch registration of multiple functions** | Register multiple functions from a single extension | This project's implementation |

### Limitations

| Constraint | Description |
|---|---|
| **Cannot modify SQL syntax** | New SQL keywords or syntax cannot be added |
| **Cannot modify storage engine** | SQLite's internal B-Tree structure and page format cannot be changed |
| **Cannot auto-create triggers or views** | There is no mechanism for implicitly executing DDL (explicit `sqlite3_exec` is possible) |
| **No multi-threading guarantees** | Depends on SQLite's threading mode. Extensions should not create their own threads |
| **No persistent auto-loading** | Extension information is not stored in the DB file. Loading is required for each connection |
| **No sandboxed execution** | Extensions run with the same privileges as the host process. Arbitrary system calls can be made |
| **Cannot write directly in other languages** | The C/C++ function pointer interface is required. Other languages must go through FFI |

### Security Notes

- Extensions execute in the **same address space** as the host process, so bugs or malicious code can affect the entire process
- Extension loading is **disabled by default** unless `sqlite3_enable_load_extension(db, 1)` is called
- The `load_extension()` SQL function has even stricter restrictions and requires separate enablement via `sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL)`
- Untrusted extensions must never be loaded

---

## 2. Project Structure

```text
sqlite3StatisticalLibrary/
├── CMakeLists.txt          # Build configuration
├── README.md
├── cmake/
│   ├── sqlite3.cmake       # SQLite3 download configuration
│   └── statcpp.cmake       # statcpp download configuration
├── doc/                    # Documentation
├── download/               # Downloaded dependency libraries
│   ├── sqlite3/            # SQLite3 source
│   └── statcpp/            # statcpp header-only library
└── src/
    ├── main.cpp             # C++ program that loads and tests the extension
    ├── ext_funcs.cpp        # Extension library (C++) — statistical function implementations
    └── include/             # (currently empty)
```

### External Libraries

#### statcpp

- **Overview**: A C++ header-only statistical computation library
- **Location**: `download/statcpp/statcpp-install/include/statcpp/`
- **Usage**: Automatically downloaded and placed by CMake's `statcpp.cmake`

In this project, `ext_funcs.cpp` includes it with `#include <statcpp/statcpp.hpp>`,
delegating the statistical computation logic to statcpp.

### Provided SQL Functions

#### Basic Aggregate Functions (24 functions)

| Function | Arguments | Return | Description |
|---|---|---|---|
| `stat_mean(col)` | numeric x1 | numeric | Arithmetic mean |
| `stat_median(col)` | numeric x1 | numeric | Median |
| `stat_mode(col)` | numeric x1 | numeric | Mode (smallest value if multiple exist) |
| `stat_geometric_mean(col)` | numeric x1 | numeric | Geometric mean |
| `stat_harmonic_mean(col)` | numeric x1 | numeric | Harmonic mean |
| `stat_range(col)` | numeric x1 | numeric | Range (max - min) |
| `stat_var(col)` | numeric x1 | numeric | Variance (population variance, ddof=0) |
| `stat_population_variance(col)` | numeric x1 | numeric | Population variance |
| `stat_sample_variance(col)` | numeric x1 | numeric | Sample variance (unbiased variance) |
| `stat_stdev(col)` | numeric x1 | numeric | Standard deviation (population, ddof=0) |
| `stat_population_stddev(col)` | numeric x1 | numeric | Population standard deviation |
| `stat_sample_stddev(col)` | numeric x1 | numeric | Sample standard deviation |
| `stat_cv(col)` | numeric x1 | numeric | Coefficient of variation |
| `stat_iqr(col)` | numeric x1 | numeric | Interquartile range |
| `stat_mad_mean(col)` | numeric x1 | numeric | Mean absolute deviation |
| `stat_geometric_stddev(col)` | numeric x1 | numeric | Geometric standard deviation |
| `stat_population_skewness(col)` | numeric x1 | numeric | Population skewness |
| `stat_skewness(col)` | numeric x1 | numeric | Sample skewness |
| `stat_population_kurtosis(col)` | numeric x1 | numeric | Population kurtosis (excess kurtosis) |
| `stat_kurtosis(col)` | numeric x1 | numeric | Sample kurtosis (excess kurtosis) |
| `stat_se(col)` | numeric x1 | numeric | Standard error |
| `stat_mad(col)` | numeric x1 | numeric | Median absolute deviation (MAD) |
| `stat_mad_scaled(col)` | numeric x1 | numeric | Scaled MAD |
| `stat_hodges_lehmann(col)` | numeric x1 | numeric | Hodges-Lehmann estimator |

#### Aggregate Functions with Parameters (20 functions)

| Function | Arguments | Return | Description |
|---|---|---|---|
| `stat_trimmed_mean(col, proportion)` | numeric, numeric | numeric | Trimmed mean |
| `stat_quartile(col)` | numeric x1 | JSON | Quartiles (Q1, Q2, Q3) |
| `stat_percentile(col, p)` | numeric, numeric | numeric | Percentile |
| `stat_z_test(col, mu0, sigma)` | numeric, numeric, numeric | JSON | One-sample z-test |
| `stat_t_test(col, mu0)` | numeric, numeric | JSON | One-sample t-test |
| `stat_chisq_gof_uniform(col)` | numeric x1 | JSON | Chi-squared goodness-of-fit test (uniform) |
| `stat_shapiro_wilk(col)` | numeric x1 | JSON | Shapiro-Wilk test |
| `stat_ks_test(col)` | numeric x1 | JSON | KS test (normality) |
| `stat_wilcoxon(col, mu0)` | numeric, numeric | JSON | Wilcoxon signed-rank test |
| `stat_ci_mean(col, confidence)` | numeric, numeric | JSON | Confidence interval for the mean (t-distribution) |
| `stat_ci_mean_z(col, sigma, confidence)` | numeric, numeric, numeric | JSON | Confidence interval for the mean (z-distribution) |
| `stat_ci_var(col, confidence)` | numeric, numeric | JSON | Confidence interval for variance |
| `stat_moe_mean(col, confidence)` | numeric, numeric | numeric | Margin of error for the mean |
| `stat_cohens_d(col, mu0)` | numeric, numeric | numeric | Cohen's d (one-sample) |
| `stat_hedges_g(col, mu0)` | numeric, numeric | numeric | Hedges' g (one-sample) |
| `stat_acf_lag(col, lag)` | numeric, numeric | numeric | Autocorrelation coefficient |
| `stat_biweight_midvar(col, c)` | numeric, numeric | numeric | Biweight Midvariance |
| `stat_bootstrap_mean(col, n)` | numeric, numeric | JSON | Bootstrap for the mean |
| `stat_bootstrap_median(col, n)` | numeric, numeric | JSON | Bootstrap for the median |
| `stat_bootstrap_stddev(col, n)` | numeric, numeric | JSON | Bootstrap for the standard deviation |

#### Two-Column Aggregate Functions (27 functions)

| Function | Arguments | Return | Description |
|---|---|---|---|
| `stat_population_covariance(x, y)` | numeric, numeric | numeric | Population covariance |
| `stat_covariance(x, y)` | numeric, numeric | numeric | Sample covariance |
| `stat_pearson_r(x, y)` | numeric, numeric | numeric | Pearson correlation coefficient |
| `stat_spearman_r(x, y)` | numeric, numeric | numeric | Spearman rank correlation |
| `stat_kendall_tau(x, y)` | numeric, numeric | numeric | Kendall rank correlation |
| `stat_weighted_covariance(val, wt)` | numeric, numeric | numeric | Weighted covariance |
| `stat_weighted_mean(val, wt)` | numeric, numeric | numeric | Weighted mean |
| `stat_weighted_harmonic_mean(val, wt)` | numeric, numeric | numeric | Weighted harmonic mean |
| `stat_weighted_variance(val, wt)` | numeric, numeric | numeric | Weighted variance |
| `stat_weighted_stddev(val, wt)` | numeric, numeric | numeric | Weighted standard deviation |
| `stat_weighted_median(val, wt)` | numeric, numeric | numeric | Weighted median |
| `stat_weighted_percentile(val, wt, p)` | numeric, numeric, numeric | numeric | Weighted percentile |
| `stat_simple_regression(x, y)` | numeric, numeric | JSON | Simple linear regression |
| `stat_r_squared(actual, pred)` | numeric, numeric | numeric | Coefficient of determination R-squared |
| `stat_adjusted_r_squared(actual, pred)` | numeric, numeric | numeric | Adjusted R-squared |
| `stat_t_test_paired(x, y)` | numeric, numeric | JSON | Paired t-test |
| `stat_chisq_gof(obs, exp)` | numeric, numeric | JSON | Chi-squared goodness-of-fit test |
| `stat_mae(actual, pred)` | numeric, numeric | numeric | Mean absolute error |
| `stat_mse(actual, pred)` | numeric, numeric | numeric | Mean squared error |
| `stat_rmse(actual, pred)` | numeric, numeric | numeric | RMSE |
| `stat_mape(actual, pred)` | numeric, numeric | numeric | Mean absolute percentage error |
| `stat_euclidean_dist(a, b)` | numeric, numeric | numeric | Euclidean distance |
| `stat_manhattan_dist(a, b)` | numeric, numeric | numeric | Manhattan distance |
| `stat_cosine_sim(a, b)` | numeric, numeric | numeric | Cosine similarity |
| `stat_cosine_dist(a, b)` | numeric, numeric | numeric | Cosine distance |
| `stat_minkowski_dist(a, b, p)` | numeric, numeric, numeric | numeric | Minkowski distance |
| `stat_chebyshev_dist(a, b)` | numeric, numeric | numeric | Chebyshev distance |

#### Window Functions (23 functions)

Window functions are implemented using a full-scan approach. Each row returns a single value. Can also be used without an `OVER()` clause.

| Function | Arguments | Return | Description |
|---|---|---|---|
| `stat_rolling_mean(col, window)` | numeric, integer | numeric/row | Rolling mean |
| `stat_rolling_std(col, window)` | numeric, integer | numeric/row | Rolling standard deviation |
| `stat_rolling_min(col, window)` | numeric, integer | numeric/row | Rolling minimum |
| `stat_rolling_max(col, window)` | numeric, integer | numeric/row | Rolling maximum |
| `stat_rolling_sum(col, window)` | numeric, integer | numeric/row | Rolling sum |
| `stat_moving_avg(col, window)` | numeric, integer | numeric/row | Simple moving average |
| `stat_ema(col, span)` | numeric, integer | numeric/row | Exponential moving average (alpha=2/(span+1)) |
| `stat_rank(col)` | numeric x1 | numeric/row | Rank transformation |
| `stat_fillna_mean(col)` | numeric x1 | numeric/row | Mean imputation |
| `stat_fillna_median(col)` | numeric x1 | numeric/row | Median imputation |
| `stat_fillna_ffill(col)` | numeric x1 | numeric/row | Forward fill |
| `stat_fillna_bfill(col)` | numeric x1 | numeric/row | Backward fill |
| `stat_fillna_interp(col)` | numeric x1 | numeric/row | Linear interpolation |
| `stat_label_encode(col)` | numeric x1 | numeric/row | Label encoding |
| `stat_bin_width(col, n_bins)` | numeric, integer | numeric/row | Equal-width binning |
| `stat_bin_freq(col, n_bins)` | numeric, integer | numeric/row | Equal-frequency binning |
| `stat_lag(col, k)` | numeric, integer | numeric/row | Lag (value from k rows before) |
| `stat_diff(col, order)` | numeric, integer | numeric/row | Differencing |
| `stat_seasonal_diff(col, period)` | numeric, integer | numeric/row | Seasonal differencing |
| `stat_outliers_iqr(col)` | numeric x1 | numeric/row | Outlier detection (IQR method, 1.0/0.0) |
| `stat_outliers_zscore(col)` | numeric x1 | numeric/row | Outlier detection (Z-score method) |
| `stat_outliers_mzscore(col)` | numeric x1 | numeric/row | Outlier detection (modified Z-score method) |
| `stat_winsorize(col, pct)` | numeric, integer | numeric/row | Winsorization |

#### Composite Aggregate Functions (32 functions)

Aggregate functions that return JSON (multiple values), or composite aggregate functions such as two-sample tests and survival analysis.

| Function | Arguments | Return | Description |
|---|---|---|---|
| `stat_modes(col)` | numeric x1 | JSON | Modes (all) |
| `stat_five_number_summary(col)` | numeric x1 | JSON | Five-number summary (min, Q1, median, Q3, max) |
| `stat_frequency_table(col)` | numeric x1 | JSON | Frequency table |
| `stat_frequency_count(col)` | numeric x1 | JSON | Frequency count for each value |
| `stat_relative_frequency(col)` | numeric x1 | JSON | Relative frequency |
| `stat_cumulative_frequency(col)` | numeric x1 | JSON | Cumulative frequency |
| `stat_cumulative_relative_frequency(col)` | numeric x1 | JSON | Cumulative relative frequency |
| `stat_t_test2(grp1, grp2)` | numeric, numeric | JSON | Two-sample t-test (pooled variance) |
| `stat_t_test_welch(grp1, grp2)` | numeric, numeric | JSON | Welch t-test |
| `stat_chisq_independence(col1, col2)` | numeric, numeric | JSON | Chi-squared independence test |
| `stat_f_test(grp1, grp2)` | numeric, numeric | JSON | F-test (variance comparison) |
| `stat_mann_whitney(grp1, grp2)` | numeric, numeric | JSON | Mann-Whitney U test |
| `stat_anova1(val, grp)` | numeric, numeric | JSON | One-way ANOVA |
| `stat_contingency_table(col1, col2)` | numeric, numeric | JSON | Contingency table |
| `stat_cohens_d2(grp1, grp2)` | numeric, numeric | numeric | Cohen's d (two-sample) |
| `stat_hedges_g2(grp1, grp2)` | numeric, numeric | numeric | Hedges' g (two-sample) |
| `stat_glass_delta(grp1, grp2)` | numeric, numeric | numeric | Glass's Delta |
| `stat_ci_mean_diff(grp1, grp2)` | numeric, numeric | JSON | Confidence interval for two-sample mean difference |
| `stat_ci_mean_diff_welch(grp1, grp2)` | numeric, numeric | JSON | Confidence interval for two-sample mean difference (Welch) |
| `stat_kaplan_meier(time, event)` | numeric, numeric | JSON | Kaplan-Meier survival curve |
| `stat_nelson_aalen(time, event)` | numeric, numeric | JSON | Nelson-Aalen cumulative hazard estimate |
| `stat_logrank(time, event, grp)` | numeric, numeric, numeric | JSON | Log-rank test |
| `stat_bootstrap(col, n)` | numeric, numeric | JSON | General bootstrap estimation |
| `stat_bootstrap_bca(col, n)` | numeric, numeric | JSON | BCa bootstrap |
| `stat_bootstrap_sample(col)` | numeric x1 | JSON | Bootstrap sample generation |
| `stat_permutation_test2(grp1, grp2)` | numeric, numeric | JSON | Two-sample permutation test |
| `stat_permutation_paired(x, y)` | numeric, numeric | JSON | Paired permutation test |
| `stat_permutation_corr(x, y)` | numeric, numeric | JSON | Correlation permutation test |
| `stat_acf(col, max_lag)` | numeric, numeric | JSON | Autocorrelation function |
| `stat_pacf(col, max_lag)` | numeric, numeric | JSON | Partial autocorrelation function |
| `stat_sample_replace(col, n)` | numeric, numeric | JSON | Sampling with replacement |
| `stat_sample(col, n)` | numeric, numeric | JSON | Sampling without replacement |

#### Scalar Functions -- Test Helpers (40 functions)

Scalar functions that compute results from parameters alone. Essential for calculating p-values and deriving confidence intervals in statistical tests.

| Function | Arguments | Return | Description |
|---|---|---|---|
| `stat_normal_pdf(x [,mu, sigma])` | 1-3 | numeric | Normal distribution PDF |
| `stat_normal_cdf(x [,mu, sigma])` | 1-3 | numeric | Normal distribution CDF |
| `stat_normal_quantile(p [,mu, sigma])` | 1-3 | numeric | Normal distribution quantile |
| `stat_normal_rand([mu, sigma])` | 0-2 | numeric | Normal distribution random variate |
| `stat_chisq_pdf(x, df)` | 2 | numeric | Chi-squared distribution PDF |
| `stat_chisq_cdf(x, df)` | 2 | numeric | Chi-squared distribution CDF |
| `stat_chisq_quantile(p, df)` | 2 | numeric | Chi-squared distribution quantile |
| `stat_chisq_rand(df)` | 1 | numeric | Chi-squared distribution random variate |
| `stat_t_pdf(x, df)` | 2 | numeric | t-distribution PDF |
| `stat_t_cdf(x, df)` | 2 | numeric | t-distribution CDF |
| `stat_t_quantile(p, df)` | 2 | numeric | t-distribution quantile |
| `stat_t_rand(df)` | 1 | numeric | t-distribution random variate |
| `stat_f_pdf(x, df1, df2)` | 3 | numeric | F-distribution PDF |
| `stat_f_cdf(x, df1, df2)` | 3 | numeric | F-distribution CDF |
| `stat_f_quantile(p, df1, df2)` | 3 | numeric | F-distribution quantile |
| `stat_f_rand(df1, df2)` | 2 | numeric | F-distribution random variate |
| `stat_betainc(a, b, x)` | 3 | numeric | Regularized incomplete beta function |
| `stat_betaincinv(a, b, p)` | 3 | numeric | Inverse regularized incomplete beta function |
| `stat_norm_cdf(x)` | 1 | numeric | Standard normal distribution CDF |
| `stat_norm_quantile(p)` | 1 | numeric | Standard normal distribution inverse CDF |
| `stat_gammainc_lower(a, x)` | 2 | numeric | Lower incomplete gamma function |
| `stat_gammainc_upper(a, x)` | 2 | numeric | Upper incomplete gamma function |
| `stat_gammainc_lower_inv(a, p)` | 2 | numeric | Inverse lower incomplete gamma function |
| `stat_z_test_prop(x, n, p0)` | 3 | JSON | One-sample proportion z-test |
| `stat_z_test_prop2(x1, n1, x2, n2)` | 4 | JSON | Two-sample proportion z-test |
| `stat_bonferroni(p, m)` | 2 | numeric | Bonferroni correction |
| `stat_bh_correction(p, rank, total)` | 3 | numeric | Benjamini-Hochberg correction |
| `stat_holm_correction(p, rank, total)` | 3 | numeric | Holm correction |
| `stat_fisher_exact(a, b, c, d)` | 4 | JSON | Fisher's exact test |
| `stat_odds_ratio(a, b, c, d)` | 4 | numeric | Odds ratio |
| `stat_relative_risk(a, b, c, d)` | 4 | numeric | Relative risk |
| `stat_risk_difference(a, b, c, d)` | 4 | numeric | Risk difference |
| `stat_nnt(a, b, c, d)` | 4 | numeric | Number needed to treat (NNT) |
| `stat_ci_prop(x, n [,confidence])` | 2-3 | JSON | Confidence interval for proportion (Wald method) |
| `stat_ci_prop_wilson(x, n [,confidence])` | 2-3 | JSON | Confidence interval for proportion (Wilson method) |
| `stat_ci_prop_diff(x1, n1, x2, n2 [,conf])` | 4-5 | JSON | Confidence interval for two-sample proportion difference |
| `stat_aic(log_likelihood, k)` | 2 | numeric | AIC |
| `stat_aicc(log_likelihood, n, k)` | 3 | numeric | Corrected AIC (AICc) |
| `stat_bic(log_likelihood, n, k)` | 3 | numeric | BIC |
| `stat_boxcox(x, lambda)` | 2 | numeric | Box-Cox transformation |

#### Scalar Functions -- Distribution & Transformation (83 functions)

Calculator-style scalar functions that compute results from parameters alone. Distribution functions, effect size conversions, power analysis, and more.

| Function | Arguments | Return | Description |
|---|---|---|---|
| `stat_uniform_pdf(x [,a, b])` | 1-3 | numeric | Uniform distribution PDF |
| `stat_uniform_cdf(x [,a, b])` | 1-3 | numeric | Uniform distribution CDF |
| `stat_uniform_quantile(p [,a, b])` | 1-3 | numeric | Uniform distribution quantile |
| `stat_uniform_rand([a, b])` | 0-2 | numeric | Uniform distribution random variate |
| `stat_exponential_pdf(x [,lambda])` | 1-2 | numeric | Exponential distribution PDF |
| `stat_exponential_cdf(x [,lambda])` | 1-2 | numeric | Exponential distribution CDF |
| `stat_exponential_quantile(p [,lambda])` | 1-2 | numeric | Exponential distribution quantile |
| `stat_exponential_rand([lambda])` | 0-1 | numeric | Exponential distribution random variate |
| `stat_gamma_pdf(x, shape, scale)` | 3 | numeric | Gamma distribution PDF |
| `stat_gamma_cdf(x, shape, scale)` | 3 | numeric | Gamma distribution CDF |
| `stat_gamma_quantile(p, shape, scale)` | 3 | numeric | Gamma distribution quantile |
| `stat_gamma_rand(shape, scale)` | 2 | numeric | Gamma distribution random variate |
| `stat_beta_pdf(x, alpha, beta)` | 3 | numeric | Beta distribution PDF |
| `stat_beta_cdf(x, alpha, beta)` | 3 | numeric | Beta distribution CDF |
| `stat_beta_quantile(p, alpha, beta)` | 3 | numeric | Beta distribution quantile |
| `stat_beta_rand(alpha, beta)` | 2 | numeric | Beta distribution random variate |
| `stat_lognormal_pdf(x [,mu, sigma])` | 1-3 | numeric | Log-normal distribution PDF |
| `stat_lognormal_cdf(x [,mu, sigma])` | 1-3 | numeric | Log-normal distribution CDF |
| `stat_lognormal_quantile(p [,mu, sigma])` | 1-3 | numeric | Log-normal distribution quantile |
| `stat_lognormal_rand([mu, sigma])` | 0-2 | numeric | Log-normal distribution random variate |
| `stat_weibull_pdf(x, shape, scale)` | 3 | numeric | Weibull distribution PDF |
| `stat_weibull_cdf(x, shape, scale)` | 3 | numeric | Weibull distribution CDF |
| `stat_weibull_quantile(p, shape, scale)` | 3 | numeric | Weibull distribution quantile |
| `stat_weibull_rand(shape, scale)` | 2 | numeric | Weibull distribution random variate |
| `stat_binomial_pmf(k, n, p)` | 3 | numeric | Binomial distribution PMF |
| `stat_binomial_cdf(k, n, p)` | 3 | numeric | Binomial distribution CDF |
| `stat_binomial_quantile(q, n, p)` | 3 | numeric | Binomial distribution quantile |
| `stat_binomial_rand(n, p)` | 2 | numeric | Binomial distribution random variate |
| `stat_poisson_pmf(k, lambda)` | 2 | numeric | Poisson distribution PMF |
| `stat_poisson_cdf(k, lambda)` | 2 | numeric | Poisson distribution CDF |
| `stat_poisson_quantile(q, lambda)` | 2 | numeric | Poisson distribution quantile |
| `stat_poisson_rand(lambda)` | 1 | numeric | Poisson distribution random variate |
| `stat_geometric_pmf(k, p)` | 2 | numeric | Geometric distribution PMF |
| `stat_geometric_cdf(k, p)` | 2 | numeric | Geometric distribution CDF |
| `stat_geometric_quantile(q, p)` | 2 | numeric | Geometric distribution quantile |
| `stat_geometric_rand(p)` | 1 | numeric | Geometric distribution random variate |
| `stat_nbinom_pmf(k, r, p)` | 3 | numeric | Negative binomial distribution PMF |
| `stat_nbinom_cdf(k, r, p)` | 3 | numeric | Negative binomial distribution CDF |
| `stat_nbinom_quantile(q, r, p)` | 3 | numeric | Negative binomial distribution quantile |
| `stat_nbinom_rand(r, p)` | 2 | numeric | Negative binomial distribution random variate |
| `stat_hypergeom_pmf(k, N, K, n)` | 4 | numeric | Hypergeometric distribution PMF |
| `stat_hypergeom_cdf(k, N, K, n)` | 4 | numeric | Hypergeometric distribution CDF |
| `stat_hypergeom_quantile(q, N, K, n)` | 4 | numeric | Hypergeometric distribution quantile |
| `stat_hypergeom_rand(N, K, n)` | 3 | numeric | Hypergeometric distribution random variate |
| `stat_bernoulli_pmf(k, p)` | 2 | numeric | Bernoulli distribution PMF |
| `stat_bernoulli_cdf(k, p)` | 2 | numeric | Bernoulli distribution CDF |
| `stat_bernoulli_quantile(q, p)` | 2 | numeric | Bernoulli distribution quantile |
| `stat_bernoulli_rand(p)` | 1 | numeric | Bernoulli distribution random variate |
| `stat_duniform_pmf(k, a, b)` | 3 | numeric | Discrete uniform distribution PMF |
| `stat_duniform_cdf(k, a, b)` | 3 | numeric | Discrete uniform distribution CDF |
| `stat_duniform_quantile(q, a, b)` | 3 | numeric | Discrete uniform distribution quantile |
| `stat_duniform_rand(a, b)` | 2 | numeric | Discrete uniform distribution random variate |
| `stat_binomial_coef(n, k)` | 2 | integer | Binomial coefficient |
| `stat_log_binomial_coef(n, k)` | 2 | numeric | Log binomial coefficient |
| `stat_log_factorial(n)` | 1 | numeric | Log factorial |
| `stat_lgamma(x)` | 1 | numeric | Log-gamma function |
| `stat_tgamma(x)` | 1 | numeric | Gamma function |
| `stat_beta_func(a, b)` | 2 | numeric | Beta function |
| `stat_lbeta(a, b)` | 2 | numeric | Log-beta function |
| `stat_erf(x)` | 1 | numeric | Error function |
| `stat_erfc(x)` | 1 | numeric | Complementary error function |
| `stat_logarithmic_mean(a, b)` | 2 | numeric | Logarithmic mean |
| `stat_hedges_j(n)` | 1 | numeric | Hedges correction factor |
| `stat_t_to_r(t, df)` | 2 | numeric | t-value to correlation coefficient conversion |
| `stat_d_to_r(d)` | 1 | numeric | Cohen's d to correlation coefficient conversion |
| `stat_r_to_d(r)` | 1 | numeric | Correlation coefficient to Cohen's d conversion |
| `stat_eta_squared_ef(ss_effect, ss_total)` | 2 | numeric | Eta-squared (effect size) |
| `stat_partial_eta_sq(F, df1, df2)` | 3 | numeric | Partial eta-squared |
| `stat_omega_squared_ef(ss_effect, ss_total, ms_error, df_effect)` | 4 | numeric | Omega-squared (effect size) |
| `stat_cohens_h(p1, p2)` | 2 | numeric | Cohen's h (proportion difference) |
| `stat_interpret_d(d)` | 1 | text | Interpretation of Cohen's d |
| `stat_interpret_r(r)` | 1 | text | Interpretation of correlation coefficient |
| `stat_interpret_eta2(eta2)` | 1 | text | Interpretation of eta-squared |
| `stat_power_t1(d, n, alpha)` | 3 | numeric | Power of one-sample t-test |
| `stat_n_t1(d, power, alpha)` | 3 | numeric | Required sample size for one-sample t-test |
| `stat_power_t2(d, n1, n2, alpha)` | 4 | numeric | Power of two-sample t-test |
| `stat_n_t2(d, power, alpha)` | 3 | numeric | Required sample size for two-sample t-test |
| `stat_power_prop(p1, p2, n, alpha)` | 4 | numeric | Power of proportion test |
| `stat_n_prop(p1, p2, power, alpha)` | 4 | numeric | Required sample size for proportion test |
| `stat_moe_prop(x, n [,confidence])` | 2-3 | numeric | Margin of error for proportion |
| `stat_moe_prop_worst(n [,confidence])` | 1-2 | numeric | Worst-case margin of error |
| `stat_n_moe_prop(moe [,confidence [,p]])` | 1-3 | numeric | Sample size for proportion estimation |
| `stat_n_moe_mean(moe, sigma [,confidence])` | 2-3 | numeric | Sample size for mean estimation |

All aggregate functions ignore `NULL` input rows (following SQLite3 aggregate function conventions).
Window functions track `NULL` inputs and either return `NULL` or impute values in the result (function-dependent).
Scalar functions return `NULL` if any argument is `NULL`.
`NULL` is returned when all rows are `NULL` or the result set is empty.
`NULL` is returned when the computed result is `NaN` or `Inf`.
Random variate functions (`*_rand`) are registered as non-deterministic (without the `SQLITE_DETERMINISTIC` flag).

---

## 3. Build and Run

```bash
# Build
cmake -B build
cmake --build build

# Run (C++ test program)
./build/a.out
```

### Using from sqlite3 CLI

The built extension library can also be used from the `sqlite3` command-line tool.

#### Prerequisites

The `sqlite3` CLI itself must be built with `SQLITE_ENABLE_LOAD_EXTENSION` enabled.
The macOS standard `sqlite3` has this enabled.

#### Method 1: `.load` dot-command (recommended)

```bash
cd build
sqlite3 test.db
```

```sql
sqlite> .load ./ext_funcs sqlite3_ext_funcs_init
sqlite> SELECT stat_mean(val), stat_median(val) FROM data;
5.5|5.5
sqlite> SELECT stat_sample_stddev(val) FROM data;
3.02765035409749
sqlite> SELECT category, stat_mean(score) FROM exam GROUP BY category;
```

The first argument to `.load` is the library path, and the second argument is the entry point name.
The file extension (`.dylib`) can be omitted (SQLite auto-completes it).

#### Method 2: `load_extension()` SQL function

```sql
SELECT load_extension('./ext_funcs', 'sqlite3_ext_funcs_init');
SELECT square(7);
```

This method has stricter restrictions than `.load` and requires separate enablement via
`sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL)`.
Using `.load` is generally more reliable.

#### Why Entry Point Name Must Be Specified

SQLite auto-infers the entry point name from the filename, but
`ext_funcs` becomes `sqlite3_extfuncs_init` with `_` stripped.
Since the actual entry point name is `sqlite3_ext_funcs_init`,
it must be explicitly specified as the second argument.

---

## 4. How Loadable Extensions Work

### Overall Flow

```text
1. main.cpp: sqlite3_enable_load_extension(db, 1)
   └─ Enable extension loading

2. main.cpp: sqlite3_load_extension(db, "ext_funcs.dylib", "sqlite3_ext_funcs_init", ...)
   └─ dyld loads the dynamic library
   └─ Calls the entry point function

3. ext_funcs: sqlite3_ext_funcs_init(db, ...)
   └─ Initialize API routines with SQLITE_EXTENSION_INIT2(pApi)
   └─ Register custom functions with sqlite3_create_function()

4. main.cpp: SELECT square(5), upper('hello'), sha256_agg(content), ...
   └─ SQLite calls the registered callback functions
```

### Entry Point Naming Convention

SQLite infers the entry point name from the library filename:

```text
ext_funcs.dylib → sqlite3_ext_funcs_init
```

If the filename contains `-` or `.`, auto-inference will fail, so
the entry point must be explicitly specified as the third argument of `sqlite3_load_extension()`.

### Required Macros

```cpp
#include <sqlite3ext.h>

SQLITE_EXTENSION_INIT1              // Once at global scope
// ...
extern "C"
int sqlite3_ext_funcs_init(...) {
    SQLITE_EXTENSION_INIT2(pApi);   // Once at the beginning of the entry point
    // ...
}
```

- `SQLITE_EXTENSION_INIT1`: Declares the global pointer for the SQLite API
- `SQLITE_EXTENSION_INIT2(pApi)`: Sets the API pointer passed from the host application
- `extern "C"`: Specifies C linkage for the entry point when compiling with C++

Without these, API calls such as `sqlite3_create_function` cannot be made.

### Compile Flags

`SQLITE_ENABLE_LOAD_EXTENSION` must be enabled on the host side (SQLite core):

```cmake
target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_LOAD_EXTENSION)
```

---

## 5. Extension Function Implementation Patterns

### 5.1. Function Registration (sqlite3_create_function)

```cpp
int sqlite3_create_function(
    sqlite3 *db,              // Database connection
    const char *zFunctionName,// SQL function name
    int nArg,                 // Number of arguments (-1 for variadic)
    int eTextRep,             // Text encoding + flags
    void *pApp,               // User data passed to callbacks (optional)
    void (*xFunc)(...),       // Scalar function callback
    void (*xStep)(...),       // Per-row processing for aggregate functions (nullptr for scalar)
    void (*xFinal)(...)       // Final processing for aggregate functions (nullptr for scalar)
);
```

#### Flags for eTextRep

| Flag | Meaning |
|---|---|
| `SQLITE_UTF8` | UTF-8 text (typically this one) |
| `SQLITE_UTF16` | UTF-16 text |
| `SQLITE_DETERMINISTIC` | Always returns the same result for the same input (optimization hint) |
| `SQLITE_INNOCUOUS` | No side effects, safe (SQLite 3.31+) |
| `SQLITE_DIRECTONLY` | Can only be called from SQL (SQLite 3.30+) |

### 5.2. Callback Function Signature

```cpp
void callback(sqlite3_context *ctx, int argc, sqlite3_value **argv);
```

- `ctx`: Context used for returning results and reporting errors
- `argc`: Number of arguments
- `argv`: Array of arguments

### 5.3. Retrieving Arguments (sqlite3_value_*)

| Function | Return Type | Description |
|---|---|---|
| `sqlite3_value_type(v)` | `int` | Returns the type of the value (see type constants below) |
| `sqlite3_value_int(v)` | `int` | Retrieve as 32-bit integer |
| `sqlite3_value_int64(v)` | `sqlite3_int64` | Retrieve as 64-bit integer |
| `sqlite3_value_double(v)` | `double` | Retrieve as floating point |
| `sqlite3_value_text(v)` | `const unsigned char *` | Retrieve as UTF-8 string |
| `sqlite3_value_text16(v)` | `const void *` | Retrieve as UTF-16 string |
| `sqlite3_value_blob(v)` | `const void *` | Retrieve as BLOB data |
| `sqlite3_value_bytes(v)` | `int` | Byte count of text/blob |

#### Type Constants (return values of sqlite3_value_type)

| Constant | Value | Description |
|---|---|---|
| `SQLITE_INTEGER` | 1 | Integer |
| `SQLITE_FLOAT` | 2 | Floating point |
| `SQLITE_TEXT` | 3 | Text |
| `SQLITE_BLOB` | 4 | Binary |
| `SQLITE_NULL` | 5 | NULL |

### 5.4. Returning Results (sqlite3_result_*)

#### Returning Scalar Values

| Function | Return Type | C++ Type |
|---|---|---|
| `sqlite3_result_int(ctx, int)` | 32-bit integer | `int` |
| `sqlite3_result_int64(ctx, sqlite3_int64)` | 64-bit integer | `sqlite3_int64` |
| `sqlite3_result_double(ctx, double)` | Floating point | `double` |
| `sqlite3_result_text(ctx, str, len, destructor)` | UTF-8 string | `const char *` |
| `sqlite3_result_text16(ctx, str, len, destructor)` | UTF-16 string | `const void *` |
| `sqlite3_result_blob(ctx, data, len, destructor)` | Binary data | `const void *` |
| `sqlite3_result_null(ctx)` | NULL | -- |

#### Special Returns

| Function | Usage |
|---|---|
| `sqlite3_result_zeroblob(ctx, n)` | Return an n-byte zero-filled BLOB |
| `sqlite3_result_value(ctx, sqlite3_value*)` | Return the input value as-is (preserving its type) |
| `sqlite3_result_pointer(ctx, ptr, type, destructor)` | Pass a pointer (SQLite 3.20+) |
| `sqlite3_result_subtype(ctx, unsigned int)` | Attach subtype information such as JSON |

#### Returning Errors

| Function | Usage |
|---|---|
| `sqlite3_result_error(ctx, msg, len)` | Return an error message |
| `sqlite3_result_error_nomem(ctx)` | Return an out-of-memory error |
| `sqlite3_result_error_toobig(ctx)` | Return a data-too-large error |
| `sqlite3_result_error_code(ctx, int)` | Return an arbitrary error code |

#### destructor Argument

Memory management specification passed as the last argument to `_text` and `_blob` functions:

| Value | Meaning |
|---|---|
| `SQLITE_STATIC` | Data is managed by the caller. SQLite does not copy it |
| `SQLITE_TRANSIENT` | SQLite creates an internal copy. Safe even for local variables |
| Function pointer (e.g., `sqlite3_free`) | SQLite calls the destructor after use |

### 5.5. Memory Management

Within extensions, it is recommended to use SQLite-provided functions instead of standard `malloc`/`free`:

| Function | Description |
|---|---|
| `sqlite3_malloc(n)` | Allocate n bytes of memory |
| `sqlite3_realloc(ptr, n)` | Reallocate memory |
| `sqlite3_free(ptr)` | Free memory |
| `sqlite3_mprintf(fmt, ...)` | printf-style string creation with memory allocation |

By passing `sqlite3_free` as the destructor to `sqlite3_result_text`,
SQLite manages the deallocation of the buffer:

```cpp
auto *result = static_cast<char *>(sqlite3_malloc(len + 1));
// ... write data to result ...
sqlite3_result_text(ctx, result, len, sqlite3_free);
// → SQLite will call sqlite3_free(result) when done with result
```

### 5.6. Implementation Template

```cpp
static void my_func(sqlite3_context *ctx, int /*argc*/, sqlite3_value **argv) {
    // 1. NULL check
    if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
        sqlite3_result_null(ctx);
        return;
    }

    // 2. Retrieve arguments
    const char *input = reinterpret_cast<const char *>(sqlite3_value_text(argv[0]));
    int len = static_cast<int>(std::strlen(input));

    // 3. Processing
    auto *result = static_cast<char *>(sqlite3_malloc(len + 1));
    if (!result) {
        sqlite3_result_error_nomem(ctx);
        return;
    }
    // ... build result ...

    // 4. Return result
    sqlite3_result_text(ctx, result, len, sqlite3_free);
}
```

---

## 6. SQLite C API Reference

### Extension Loading API

| Function | Description |
|---|---|
| `sqlite3_enable_load_extension(db, onoff)` | Enable/disable extension loading |
| `sqlite3_load_extension(db, file, proc, errmsg)` | Load an extension library |
| `sqlite3_auto_extension(xEntryPoint)` | Register an extension to auto-load on new DB connections |
| `sqlite3_cancel_auto_extension(xEntryPoint)` | Unregister an auto-load extension |
| `sqlite3_reset_auto_extension()` | Unregister all auto-load extensions |

### Using pApp (User Data)

An arbitrary pointer can be passed as the 5th argument `pApp` of `sqlite3_create_function`.
Retrieve it within the callback using `sqlite3_user_data(ctx)`:

```cpp
// At registration
int multiplier = 10;
sqlite3_create_function(db, "scale", 1, SQLITE_UTF8, &multiplier,
                         scale_func, nullptr, nullptr);

// Inside the callback
static void scale_func(sqlite3_context *ctx, int /*argc*/, sqlite3_value **argv) {
    auto *mul = static_cast<int *>(sqlite3_user_data(ctx));
    double x = sqlite3_value_double(argv[0]);
    sqlite3_result_double(ctx, x * (*mul));
}
```

### Implementing Aggregate Functions

To implement aggregate functions (like SUM, AVG), use `xStep` and `xFinal`.
In this project, a pattern is adopted where the state object is allocated on the heap and only a pointer is stored in `sqlite3_aggregate_context`:

```cpp
struct SingleColumnState {
    std::vector<double> values;
};

// xStep: Called for each row (NULL rows are skipped)
static void xStep(sqlite3_context *ctx, int /*argc*/, sqlite3_value **argv) {
    if (sqlite3_value_type(argv[0]) == SQLITE_NULL) return;
    auto **pp = static_cast<SingleColumnState**>(
        sqlite3_aggregate_context(ctx, sizeof(SingleColumnState*)));
    if (!pp) return;
    if (!*pp) *pp = new (std::nothrow) SingleColumnState();
    if (*pp) (*pp)->values.push_back(sqlite3_value_double(argv[0]));
}

// xFinal: Called once at the end
static void xFinal(sqlite3_context *ctx) {
    auto **pp = static_cast<SingleColumnState**>(
        sqlite3_aggregate_context(ctx, 0));
    if (!pp || !*pp || (*pp)->values.empty()) {
        sqlite3_result_null(ctx);
    } else {
        double result = statcpp::mean((*pp)->values.begin(), (*pp)->values.end());
        if (std::isnan(result) || std::isinf(result)) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_double(ctx, result);
        }
    }
    if (pp && *pp) { delete *pp; *pp = nullptr; }
}

// Registration (xFunc = nullptr, specify xStep and xFinal)
sqlite3_create_function_v2(db, "stat_mean", 1,
    SQLITE_UTF8 | SQLITE_DETERMINISTIC,
    nullptr, nullptr, xStep, xFinal, nullptr);
```

---

## 7. C++ Implementation Techniques

### Three Callback Coding Styles

In this project's `ext_funcs.cpp`, callbacks passed to `sqlite3_create_function` are written in three styles:

#### Method 1: Regular static function

```cpp
// Function definition
static void reverse_func(sqlite3_context *ctx, int, sqlite3_value **argv) {
    // ...
}

// Registration
sqlite3_create_function(db, "reverse", 1, ..., reverse_func, nullptr, nullptr);
```

#### Method 2: Lambda assigned to auto variable

```cpp
// Store lambda in a named variable
static auto repeat_func = [](sqlite3_context *ctx, int, sqlite3_value **argv) {
    // ...
};

// Registration — convert to function pointer with + and pass
sqlite3_create_function(db, "repeat", 2, ..., +repeat_func, nullptr, nullptr);
```

#### Method 3: Inline lambda

```cpp
// Write directly at registration
sqlite3_create_function(db, "is_palindrome", 1, ...,
    +[](sqlite3_context *ctx, int, sqlite3_value **argv) {
        // ...
    },
    nullptr, nullptr);
```

All three styles result in identical function pointers after compilation, so there is no runtime difference.

### Concise Registration with Template Helpers

This project uses six template patterns, designed so that only the statistical computation logic is swapped out.

```cpp
// --- Template A: Basic aggregate functions ---
static double calc_mean(const std::vector<double>& v) {
    return statcpp::mean(v.begin(), v.end());
}
rc = SingleColumnAggregate<calc_mean>::register_func(db, "stat_mean");

// --- Template B: Aggregate functions with parameters ---
rc = SingleColumnParamAggregate<1, calc_percentile>::register_func(db, "stat_percentile");
rc = SingleColumnParamAggregateText<1, calc_t_test>::register_func(db, "stat_t_test");

// --- Template C: Two-column aggregate functions ---
rc = TwoColumnAggregate<calc_pearson_r>::register_func(db, "stat_pearson_r");
rc = TwoColumnAggregateText<calc_simple_regression>::register_func(db, "stat_simple_regression");
rc = TwoColumnParamAggregate<1, calc_weighted_percentile>::register_func(db, "stat_weighted_percentile");

// --- Template D: Window functions ---
rc = FullScanWindowFunction<wf_rolling_mean>::register_func_2(db, "stat_rolling_mean");  // 2 arguments
rc = FullScanWindowFunction<wf_rank>::register_func_1(db, "stat_rank");                   // 1 argument

// --- Template E: Composite aggregate functions ---
rc = SingleColumnAggregateText<calc_modes>::register_func(db, "stat_modes");
rc = TwoColumnAggregateText<calc_t_test2>::register_func(db, "stat_t_test2");

// --- Template F: Scalar functions ---
rc = register_scalar(db, "stat_normal_pdf", -1, sf_normal_pdf);     // Deterministic
rc = register_scalar_nd(db, "stat_normal_rand", -1, sf_normal_rand); // Non-deterministic (random)
```

### How Lambda Expressions Work

```cpp
+[](sqlite3_context *ctx, int, sqlite3_value **argv) { ... }
```

- A captureless lambda (`[]`) is converted to a regular function pointer **at compile time**
- The leading `+` is an idiom for explicitly converting the lambda to a function pointer
- The generated code is **identical** to `static void func(...)`
- `std::function` is not a function pointer and therefore cannot be passed directly to `sqlite3_create_function`

### How Templates Work

```cpp
template <double (*Fn)(double)>
int register_double_func(sqlite3 *db, const char *name) { ... }
```

- Templates are **expanded at compile time**
- The `Fn(x)` call is **inlined** and does not result in an indirect call

### Overloading Considerations

When overloads of the same function name exist, the template argument becomes ambiguous:

```cpp
static double cube(double x) { return x * x * x; }
static long   cube(long x)   { return x * x * x; }

// Explicitly cast to resolve overload
register_double_func<static_cast<double(*)(double)>(cube)>(db, "cube");
```

Since SQLite values are internally dynamically typed, unifying on `double` is the simplest approach.

---

## 8. Dynamic Library Considerations

### Entry Point Symbol

`extern "C"` suppresses name mangling, exporting the symbol name that SQLite expects.

```bash
nm -gU ext_funcs.dylib | grep sqlite3_ext_funcs_init
# → _sqlite3_ext_funcs_init
```

Lambdas and templates have internal linkage (equivalent to `static`) and do not appear in the `.dylib` public symbol table.

### C++ Runtime Dependency

```bash
otool -L ext_funcs.dylib
```

Use of `std::string`, `std::reverse`, etc. adds a dynamic link dependency on `libc++`.

### libc++ Loading Cost in This Project

In this project, since `main.cpp` is compiled and linked as C++,
`dyld` has already loaded `libc++.1.dylib` at process startup.

```text
1. main starts
   └─ dyld loads libc++.1.dylib (required because main is C++)

2. sqlite3_load_extension() loads ext_funcs.dylib
   └─ ext_funcs.dylib also depends on libc++
   └─ But libc++ is already in memory
   └─ → Only the reference count is incremented; no reload occurs
```

macOS's `dyld` does not load the same library twice.
It merely shares the already-mapped address, so the cost is essentially zero.

### How to Eliminate libc++ Dependency

1. Avoid `std::string` and use `sqlite3_malloc` + hand-written loops instead
2. Use only header-only functions from `<algorithm>` (they are inlined)
3. Disable exceptions and RTTI at build time:

```cmake
target_compile_options(ext_funcs PRIVATE -fno-exceptions -fno-rtti)
```

### Runtime Overhead

| Element | Overhead | Description |
|---|---|---|
| Lambda to function pointer | None | Compile-time conversion |
| Template expansion | None | Compile-time expansion, inlined |
| `extern "C"` | None | Linkage specification only |
| `std::string` + `SQLITE_TRANSIENT` | Minimal | One extra copy (e.g., reverse function) |
