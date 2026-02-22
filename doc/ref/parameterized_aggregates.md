# Parameterized Aggregates (20 Functions)

Aggregate functions with parameters. Used in the form `SELECT stat_xxx(column, param) FROM table`. Available in all contexts where SQLite3 standard aggregate functions can be used, including `GROUP BY`, `HAVING`, subqueries, etc.

← [Function Reference](../function_reference.md)

---

## Basic Statistics (Parameterized)

### stat_trimmed_mean

Computes the **trimmed mean** (truncated mean). Removes a fixed proportion of data from both ends before computing the mean. Robust against outliers.

$$\bar{x}_{trim} = \frac{1}{n - 2k}\sum_{i=k+1}^{n-k} x_{(i)}$$

where $k = \lfloor n \times proportion \rfloor$.

**Syntax**: `stat_trimmed_mean(column, proportion)`

| Parameter | Description | Range |
|---|---|---|
| `proportion` | Proportion to remove from each end | 0.0 or more, less than 0.5 |

```sql
-- 5% trimmed mean (removes 5% from each end)
SELECT stat_trimmed_mean(salary, 0.05) AS trimmed_mean FROM employees;

-- 10% trimmed mean (most common)
SELECT stat_trimmed_mean(score, 0.1) AS trimmed_10pct FROM exam;

-- Comparison of arithmetic mean, median, and trimmed mean
SELECT stat_mean(val)                AS mean,
       stat_trimmed_mean(val, 0.1)   AS trimmed_mean_10,
       stat_trimmed_mean(val, 0.25)  AS trimmed_mean_25,
       stat_median(val)              AS median
FROM data;
-- proportion=0 -> arithmetic mean, proportion->0.5 -> approaches the median

-- Comparison when outliers are present
CREATE TABLE outlier_test (val REAL);
INSERT INTO outlier_test VALUES (1),(2),(3),(4),(5),(6),(7),(8),(9),(1000);

SELECT stat_mean(val)              AS mean,          -- 104.5 (pulled by outlier)
       stat_trimmed_mean(val, 0.1) AS trimmed_mean,  -- 5.0 (outlier removed)
       stat_median(val)            AS median          -- 5.5
FROM outlier_test;

-- Robust mean per group
SELECT department,
       stat_mean(salary)              AS mean_salary,
       stat_trimmed_mean(salary, 0.1) AS trimmed_salary
FROM employees
GROUP BY department;
```

---

## Order Statistics (Parameterized)

### stat_quartile

Computes the **quartiles** (Q1, Q2, Q3) at once and returns them as JSON.

**Syntax**: `stat_quartile(column)`

**Return value**: JSON string `{"q1": ..., "q2": ..., "q3": ...}`

| Key | Description |
|---|---|
| `q1` | First quartile (25th percentile) |
| `q2` | Second quartile (median, 50th percentile) |
| `q3` | Third quartile (75th percentile) |

```sql
-- Basic usage
SELECT stat_quartile(score) AS quartiles FROM exam;
-- -> {"q1":45.5,"q2":62.0,"q3":78.25}

-- Extract individual values using SQLite3's json_extract()
SELECT json_extract(stat_quartile(score), '$.q1') AS Q1,
       json_extract(stat_quartile(score), '$.q2') AS Q2,
       json_extract(stat_quartile(score), '$.q3') AS Q3
FROM exam;

-- Outlier detection using IQR (combined with stat_iqr)
SELECT val,
       CASE
           WHEN val < json_extract(q, '$.q1') - 1.5 * stat_iqr(val)
             OR val > json_extract(q, '$.q3') + 1.5 * stat_iqr(val)
           THEN 'outlier'
           ELSE 'normal'
       END AS status
FROM data,
     (SELECT stat_quartile(val) AS q, stat_iqr(val) AS iqr FROM data);

-- Quartiles per group
SELECT category, stat_quartile(price) AS quartiles
FROM products
GROUP BY category;

-- Box plot summary statistics
SELECT MIN(val) AS whisker_low,
       json_extract(stat_quartile(val), '$.q1') AS q1,
       json_extract(stat_quartile(val), '$.q2') AS median,
       json_extract(stat_quartile(val), '$.q3') AS q3,
       MAX(val) AS whisker_high
FROM data;
```

---

### stat_percentile

Computes the **percentile** (quantile).

**Syntax**: `stat_percentile(column, p)`

| Parameter | Description | Range |
|---|---|---|
| `p` | Percentile position | 0.0 to 1.0 |

```sql
-- Median (50th percentile)
SELECT stat_percentile(score, 0.5) AS median FROM exam;

-- Key percentiles
SELECT stat_percentile(response_time, 0.50) AS p50,
       stat_percentile(response_time, 0.90) AS p90,
       stat_percentile(response_time, 0.95) AS p95,
       stat_percentile(response_time, 0.99) AS p99
FROM api_logs;

-- SLA monitoring: check whether the 95th percentile is within the threshold
SELECT endpoint,
       stat_percentile(latency_ms, 0.95) AS p95_latency
FROM requests
GROUP BY endpoint
HAVING stat_percentile(latency_ms, 0.95) > 500;

-- Deciles
SELECT stat_percentile(income, 0.1) AS D1,
       stat_percentile(income, 0.2) AS D2,
       stat_percentile(income, 0.5) AS D5,
       stat_percentile(income, 0.9) AS D9
FROM households;
```

---

## Parametric Tests

### Common: Test Result JSON Format

Parametric test and nonparametric test functions return their results as a JSON string.

```json
{"statistic": 5.744, "p_value": 0.000278, "df": 9}
```

| Key | Description |
|---|---|
| `statistic` | Test statistic |
| `p_value` | p-value (two-sided test) |
| `df` | Degrees of freedom (`null` if not applicable) |

Individual values can be extracted using SQLite3's `json_extract()`.

```sql
-- Example: extract only the p-value
SELECT json_extract(stat_t_test(val, 0), '$.p_value') AS p_value FROM data;
```

---

### stat_z_test

Performs a **one-sample z-test**. Tests the population mean when the population standard deviation is known.

$$z = \frac{\bar{x} - \mu_0}{\sigma / \sqrt{n}}$$

**Syntax**: `stat_z_test(column, mu0, sigma)`

| Parameter | Description |
|---|---|
| `mu0` | Population mean under the null hypothesis |
| `sigma` | Known population standard deviation |

**Return value**: JSON `{"statistic": z-value, "p_value": ..., "df": null}`

```sql
-- H0: mu = 100 (population standard deviation sigma = 15 is known)
SELECT stat_z_test(iq_score, 100, 15) AS result FROM students;
-- -> {"statistic":2.1,"p_value":0.0357,"df":null}

-- Extract only the p-value and determine significance
SELECT
    json_extract(stat_z_test(weight, 500, 10), '$.p_value') AS p_value,
    CASE
        WHEN json_extract(stat_z_test(weight, 500, 10), '$.p_value') < 0.05
        THEN 'reject H0'
        ELSE 'fail to reject H0'
    END AS decision
FROM products;

-- Quality test per production line (target: 100g, known sigma: 2g)
SELECT line_id,
       stat_z_test(weight, 100.0, 2.0) AS z_test_result
FROM production
GROUP BY line_id;
```

---

### stat_t_test

Performs a **one-sample t-test**. Tests the population mean when the population standard deviation is unknown.

$$t = \frac{\bar{x} - \mu_0}{s / \sqrt{n}}, \quad df = n - 1$$

**Syntax**: `stat_t_test(column, mu0)`

| Parameter | Description |
|---|---|
| `mu0` | Population mean under the null hypothesis |

**Return value**: JSON `{"statistic": t-value, "p_value": ..., "df": ...}`

> **Minimum data count**: 2

```sql
-- H0: mu = 0 (test whether there is no difference)
SELECT stat_t_test(improvement, 0) AS result FROM treatment;
-- -> {"statistic":3.46,"p_value":0.0074,"df":9}

-- Test whether average body temperature is 36.5 degrees Celsius
SELECT stat_t_test(body_temp, 36.5) AS result FROM patients;

-- Obtain p-value and confidence interval simultaneously
SELECT json_extract(stat_t_test(val, 0), '$.p_value') AS p_value,
       stat_ci_mean(val, 0.95) AS confidence_interval
FROM data;

-- Test per group
SELECT department,
       stat_mean(salary) AS mean_salary,
       stat_t_test(salary, 50000) AS test_vs_50k
FROM employees
GROUP BY department;

-- Test of differences in paired data (compute the differences first)
-- diff = after - before, computed in advance
SELECT stat_t_test(diff, 0) AS paired_test
FROM (SELECT after_val - before_val AS diff FROM paired_data);
```

---

### stat_chisq_gof_uniform

Performs a **chi-square goodness-of-fit test** (for uniform distribution). Tests whether observed frequencies deviate from a uniform distribution.

$$\chi^2 = \sum_{i=1}^{k}\frac{(O_i - E)^2}{E}, \quad df = k - 1$$

**Syntax**: `stat_chisq_gof_uniform(column)`

**Return value**: JSON `{"statistic": chi-squared value, "p_value": ..., "df": ...}`

> **Minimum data count**: 2

```sql
-- Test whether die rolls are uniformly distributed
-- Frequency of each face: 1->18, 2->15, 3->20, 4->22, 5->12, 6->13
CREATE TABLE dice (count REAL);
INSERT INTO dice VALUES (18),(15),(20),(22),(12),(13);

SELECT stat_chisq_gof_uniform(count) AS result FROM dice;
-- A large p_value is consistent with uniformity

-- Test whether access counts by day of week are uniform
SELECT stat_chisq_gof_uniform(access_count) AS uniformity_test
FROM (SELECT day_of_week, COUNT(*) AS access_count
      FROM web_logs
      GROUP BY day_of_week);
```

---

## Nonparametric Tests

### stat_shapiro_wilk

Performs the **Shapiro-Wilk test**. Tests whether data follow a normal distribution. One of the most powerful tests for normality.

**Syntax**: `stat_shapiro_wilk(column)`

**Return value**: JSON `{"statistic": W-value, "p_value": ..., "df": n}`

> **Minimum data count**: 3

```sql
-- Normality test
SELECT stat_shapiro_wilk(score) AS result FROM exam;
-- p_value > 0.05 -> consistent with normal distribution

-- Use to select an analysis method
SELECT
    stat_shapiro_wilk(val) AS normality_test,
    CASE
        WHEN json_extract(stat_shapiro_wilk(val), '$.p_value') > 0.05
        THEN 'parametric OK (e.g., t-test)'
        ELSE 'use nonparametric (e.g., Wilcoxon)'
    END AS recommendation
FROM data;

-- Normality test per group
SELECT category,
       stat_shapiro_wilk(measurement) AS normality
FROM samples
GROUP BY category;

-- Distribution diagnosis combined with skewness and kurtosis
SELECT stat_shapiro_wilk(val) AS shapiro_wilk,
       stat_skewness(val)     AS skewness,
       stat_kurtosis(val)     AS kurtosis
FROM data;
```

---

### stat_ks_test

Performs the **Kolmogorov-Smirnov test** (normality test). Tests whether data follow a normal distribution.

**Syntax**: `stat_ks_test(column)`

**Return value**: JSON `{"statistic": D-value, "p_value": ..., "df": n}`

> **Minimum data count**: 2

```sql
-- KS normality test
SELECT stat_ks_test(response_time) AS ks_result FROM api_logs;

-- Comparison of Shapiro-Wilk and KS
SELECT stat_shapiro_wilk(val) AS shapiro_wilk,
       stat_ks_test(val)      AS ks_test
FROM data;
-- In general, Shapiro-Wilk has higher statistical power

-- Normality test for large datasets
-- The KS test is convenient even for large samples
SELECT stat_ks_test(measurement) AS normality
FROM sensor_data
WHERE sensor_id = 'A001';
```

---

### stat_wilcoxon

Performs the **Wilcoxon signed-rank test**. A nonparametric test of location. An alternative to the t-test that does not assume normality.

**Syntax**: `stat_wilcoxon(column, mu0)`

| Parameter | Description |
|---|---|
| `mu0` | Median under the null hypothesis |

**Return value**: JSON `{"statistic": W-value, "p_value": ..., "df": n}`

> **Minimum data count**: 2

```sql
-- H0: median = 0 (test whether the treatment has no effect)
SELECT stat_wilcoxon(improvement, 0) AS result FROM treatment;

-- Alternative to t-test when normality is doubtful
-- First check normality
SELECT stat_shapiro_wilk(val) AS normality FROM data;
-- If p < 0.05, use Wilcoxon instead of t-test
SELECT stat_wilcoxon(val, 100) AS nonparam_test FROM data;

-- Compare t-test and Wilcoxon results
SELECT stat_t_test(val, 0)    AS t_test,
       stat_wilcoxon(val, 0)  AS wilcoxon
FROM data;

-- Nonparametric test per group
SELECT region,
       stat_wilcoxon(delivery_days, 3) AS test_vs_3days
FROM orders
GROUP BY region;
```

---

## Estimation

### Common: Confidence Interval JSON Format

Functions that return confidence intervals return their results as a JSON string.

```json
{"lower": 3.334, "upper": 7.666, "point_estimate": 5.5, "confidence_level": 0.95}
```

| Key | Description |
|---|---|
| `lower` | Lower bound of the confidence interval |
| `upper` | Upper bound of the confidence interval |
| `point_estimate` | Point estimate |
| `confidence_level` | Confidence level |

---

### stat_ci_mean

Computes a **confidence interval for the mean** (t-distribution based). Used when the population standard deviation is unknown.

$$\bar{x} \pm t_{\alpha/2, n-1} \cdot \frac{s}{\sqrt{n}}$$

**Syntax**: `stat_ci_mean(column, confidence)`

| Parameter | Description | Typical values |
|---|---|---|
| `confidence` | Confidence level | 0.90, 0.95, 0.99 |

**Return value**: JSON `{"lower": ..., "upper": ..., "point_estimate": ..., "confidence_level": ...}`

> **Minimum data count**: 2

```sql
-- 95% confidence interval
SELECT stat_ci_mean(score, 0.95) AS ci FROM exam;
-- -> {"lower":62.3,"upper":78.7,"point_estimate":70.5,"confidence_level":0.95}

-- Compare different confidence levels
SELECT stat_ci_mean(val, 0.90) AS ci_90,
       stat_ci_mean(val, 0.95) AS ci_95,
       stat_ci_mean(val, 0.99) AS ci_99
FROM data;

-- Extract lower and upper bounds for display
SELECT json_extract(stat_ci_mean(salary, 0.95), '$.lower') AS ci_lower,
       json_extract(stat_ci_mean(salary, 0.95), '$.point_estimate') AS mean,
       json_extract(stat_ci_mean(salary, 0.95), '$.upper') AS ci_upper
FROM employees;

-- Confidence interval per group
SELECT department,
       stat_ci_mean(salary, 0.95) AS salary_ci
FROM employees
GROUP BY department;

-- Display t-test result alongside confidence interval
SELECT stat_t_test(val, 0) AS t_test,
       stat_ci_mean(val, 0.95) AS ci_95
FROM data;
-- CI does not contain 0 <=> t-test p < 0.05
```

---

### stat_ci_mean_z

Computes a **confidence interval for the mean** (z-distribution based). Used when the population standard deviation is known.

$$\bar{x} \pm z_{\alpha/2} \cdot \frac{\sigma}{\sqrt{n}}$$

**Syntax**: `stat_ci_mean_z(column, sigma, confidence)`

| Parameter | Description |
|---|---|
| `sigma` | Known population standard deviation |
| `confidence` | Confidence level |

**Return value**: JSON `{"lower": ..., "upper": ..., "point_estimate": ..., "confidence_level": ...}`

> **Minimum data count**: 2

```sql
-- 95% confidence interval when population standard deviation sigma=15 is known
SELECT stat_ci_mean_z(iq_score, 15, 0.95) AS ci FROM students;

-- Comparison of t-distribution based and z-distribution based
SELECT stat_ci_mean(val, 0.95)       AS ci_t,
       stat_ci_mean_z(val, 3.0, 0.95) AS ci_z
FROM data;
-- When n is large, both are nearly identical

-- Quality control: when population standard deviation is known from control charts
SELECT batch_id,
       stat_ci_mean_z(weight, 0.5, 0.99) AS weight_ci
FROM production
GROUP BY batch_id;
```

---

### stat_ci_var

Computes a **confidence interval for variance** (chi-square distribution based).

$$\left[\frac{(n-1)s^2}{\chi^2_{\alpha/2}},\ \frac{(n-1)s^2}{\chi^2_{1-\alpha/2}}\right]$$

**Syntax**: `stat_ci_var(column, confidence)`

| Parameter | Description | Typical values |
|---|---|---|
| `confidence` | Confidence level | 0.90, 0.95, 0.99 |

**Return value**: JSON `{"lower": ..., "upper": ..., "point_estimate": sample variance, "confidence_level": ...}`

> **Minimum data count**: 2

```sql
-- 95% confidence interval for variance
SELECT stat_ci_var(measurement, 0.95) AS var_ci FROM data;

-- Quality control: confidence interval for variability
SELECT production_line,
       stat_sample_variance(weight) AS s2,
       stat_ci_var(weight, 0.95) AS variance_ci
FROM products
GROUP BY production_line;

-- Convert to confidence interval for standard deviation
-- CI(sigma) = [sqrt(lower), sqrt(upper)]
SELECT SQRT(json_extract(stat_ci_var(val, 0.95), '$.lower')) AS sd_lower,
       stat_sample_stddev(val) AS sd_estimate,
       SQRT(json_extract(stat_ci_var(val, 0.95), '$.upper')) AS sd_upper
FROM data;
```

---

### stat_moe_mean

Computes the **margin of error for the mean**. The half-width of the confidence interval.

$$MOE = t_{\alpha/2, n-1} \cdot \frac{s}{\sqrt{n}}$$

**Syntax**: `stat_moe_mean(column, confidence)`

| Parameter | Description | Typical values |
|---|---|---|
| `confidence` | Confidence level | 0.90, 0.95, 0.99 |

> **Minimum data count**: 2

```sql
-- Margin of error at the 95% confidence level
SELECT stat_moe_mean(score, 0.95) AS moe FROM exam;

-- Display as mean +/- MOE
SELECT stat_mean(val) AS mean,
       stat_moe_mean(val, 0.95) AS moe,
       stat_mean(val) - stat_moe_mean(val, 0.95) AS lower,
       stat_mean(val) + stat_moe_mean(val, 0.95) AS upper
FROM data;

-- Evaluate sample size adequacy
-- Check whether MOE is within the target precision
SELECT COUNT(val) AS n,
       stat_moe_mean(val, 0.95) AS moe
FROM data;
-- If MOE is too large, increase the sample size

-- Compare estimation precision across groups
SELECT region,
       COUNT(score) AS n,
       stat_mean(score) AS mean,
       stat_moe_mean(score, 0.95) AS moe
FROM survey
GROUP BY region;
```

---

## Effect Size

### stat_cohens_d

Computes **Cohen's d** (one-sample). An effect size that standardizes the difference between the mean and a hypothesized value by the sample standard deviation.

$$d = \frac{\bar{x} - \mu_0}{s}$$

**Syntax**: `stat_cohens_d(column, mu0)`

| Parameter | Description |
|---|---|
| `mu0` | Reference value (population mean under the null hypothesis) |

> **Minimum data count**: 2

```sql
-- Magnitude of treatment effect
SELECT stat_cohens_d(improvement, 0) AS effect_size FROM treatment;
-- |d| < 0.2: small effect, 0.2-0.8: medium effect, > 0.8: large effect

-- Report t-test and effect size together
SELECT stat_t_test(val, 100) AS significance,
       stat_cohens_d(val, 100) AS effect_size
FROM data;
-- Reporting effect size alongside p-value is modern statistical practice

-- Effect size per group
SELECT department,
       stat_mean(score) AS mean_score,
       stat_cohens_d(score, 70) AS effect_vs_benchmark
FROM employees
GROUP BY department;

-- Effect size interpretation guide
SELECT stat_cohens_d(val, 0) AS d,
       CASE
           WHEN ABS(stat_cohens_d(val, 0)) < 0.2 THEN 'negligible'
           WHEN ABS(stat_cohens_d(val, 0)) < 0.5 THEN 'small'
           WHEN ABS(stat_cohens_d(val, 0)) < 0.8 THEN 'medium'
           ELSE 'large'
       END AS interpretation
FROM data;
```

---

### stat_hedges_g

Computes **Hedges' g** (one-sample). An effect size that applies bias correction to Cohen's d. More accurate than Cohen's d for small samples.

$$g = d \times \left(1 - \frac{3}{4(n-1) - 1}\right)$$

**Syntax**: `stat_hedges_g(column, mu0)`

| Parameter | Description |
|---|---|
| `mu0` | Reference value |

> **Minimum data count**: 2

```sql
-- Bias-corrected effect size
SELECT stat_hedges_g(improvement, 0) AS effect_size FROM treatment;

-- Comparison of Cohen's d and Hedges' g
SELECT stat_cohens_d(val, 0)  AS cohens_d,
       stat_hedges_g(val, 0)  AS hedges_g
FROM data;
-- The larger n is, the more they agree
-- For small n, Hedges' g is more accurate

-- Effect size calculation for meta-analysis
SELECT study_group,
       COUNT(outcome) AS n,
       stat_hedges_g(outcome, 0) AS g,
       stat_se(outcome) AS se
FROM clinical_trial
GROUP BY study_group;
```

---

## Time Series

### stat_acf_lag

Computes the **autocorrelation coefficient** (Autocorrelation Function). The autocorrelation at lag k in time series data.

$$r_k = \frac{\sum_{t=1}^{n-k}(x_t - \bar{x})(x_{t+k} - \bar{x})}{\sum_{t=1}^{n}(x_t - \bar{x})^2}$$

**Syntax**: `stat_acf_lag(column, lag)`

| Parameter | Description | Range |
|---|---|---|
| `lag` | Lag order | 0 or more (always 1.0 when 0) |

> **Note**: Returns `NULL` if `lag` is greater than or equal to the number of data points.

```sql
-- Autocorrelation at lag 1
SELECT stat_acf_lag(temperature, 1) AS acf_1 FROM daily_weather;

-- Correlogram (autocorrelation at multiple lags)
SELECT stat_acf_lag(val, 0) AS lag0,  -- always 1.0
       stat_acf_lag(val, 1) AS lag1,
       stat_acf_lag(val, 2) AS lag2,
       stat_acf_lag(val, 3) AS lag3,
       stat_acf_lag(val, 5) AS lag5,
       stat_acf_lag(val, 10) AS lag10
FROM time_series;

-- Detecting seasonality (for daily data, high autocorrelation at lag=7 suggests weekly pattern)
SELECT stat_acf_lag(daily_sales, 7) AS weekly_autocorr
FROM sales_data;

-- Checking serial independence
-- |ACF| within 2/sqrt(n) is considered non-significant
SELECT stat_acf_lag(residual, 1) AS acf_1,
       2.0 / SQRT(COUNT(*)) AS significance_bound
FROM model_residuals;
```

---

## Robust Statistics

### stat_biweight_midvar

Computes the **biweight midvariance**. A highly robust estimator of variance against outliers. Based on Tukey's biweight function.

**Syntax**: `stat_biweight_midvar(column, c)`

| Parameter | Description | Typical values |
|---|---|---|
| `c` | Tuning constant | 9.0 (standard default) |

> A larger `c` is more tolerant of outliers. `c = 9.0` provides a robust estimate with 95% efficiency under the Gaussian distribution.

```sql
-- Standard biweight midvariance (c=9.0)
SELECT stat_biweight_midvar(val, 9.0) AS bwmv FROM data;

-- Comparison with sample variance
SELECT stat_sample_variance(val)   AS classical_var,
       stat_biweight_midvar(val, 9.0) AS robust_var,
       stat_mad_scaled(val) * stat_mad_scaled(val) AS mad_squared
FROM data;

-- Effect of the tuning constant c
SELECT stat_biweight_midvar(val, 6.0)  AS strict,   -- strict against outliers
       stat_biweight_midvar(val, 9.0)  AS standard,  -- standard
       stat_biweight_midvar(val, 12.0) AS lenient    -- tolerant of outliers
FROM data;

-- Quality control using robust variance estimation
SELECT batch_id,
       stat_median(weight)              AS center,
       SQRT(stat_biweight_midvar(weight, 9.0)) AS robust_sd
FROM production
GROUP BY batch_id;
```

---

## Resampling

### Common: Bootstrap Result JSON Format

Bootstrap functions return their results as a JSON string.

```json
{"estimate": 5.5, "standard_error": 0.89, "ci_lower": 3.8, "ci_upper": 7.3, "bias": 0.07}
```

| Key | Description |
|---|---|
| `estimate` | Statistic computed from the original data |
| `standard_error` | Bootstrap standard error |
| `ci_lower` | Lower bound of the bootstrap confidence interval (95%) |
| `ci_upper` | Upper bound of the bootstrap confidence interval (95%) |
| `bias` | Bias (mean of replicates minus estimate) |

> **Note**: Because bootstrap uses random numbers, results vary slightly between runs. Specify a sufficiently large `n_bootstrap` when reproducibility is needed.

---

### stat_bootstrap_mean

Performs **bootstrap estimation of the mean**. Estimates the standard error and confidence interval of the mean via resampling.

**Syntax**: `stat_bootstrap_mean(column, n_bootstrap)`

| Parameter | Description | Typical values |
|---|---|---|
| `n_bootstrap` | Number of bootstrap iterations | 1000 to 10000 |

```sql
-- 1000 bootstrap iterations
SELECT stat_bootstrap_mean(score, 1000) AS result FROM exam;

-- Compare bootstrap confidence interval with t-distribution confidence interval
SELECT stat_ci_mean(val, 0.95) AS parametric_ci,
       stat_bootstrap_mean(val, 5000) AS bootstrap_ci
FROM data;
-- When normality is doubtful, bootstrap is more reliable

-- Extract only the standard error
SELECT json_extract(stat_bootstrap_mean(val, 2000), '$.standard_error') AS boot_se,
       stat_se(val) AS analytic_se
FROM data;

-- Bootstrap estimation per group
SELECT category,
       stat_bootstrap_mean(price, 1000) AS price_bootstrap
FROM products
GROUP BY category;
```

---

### stat_bootstrap_median

Performs **bootstrap estimation of the median**. The confidence interval for the median is difficult to derive analytically, so bootstrap is particularly useful.

**Syntax**: `stat_bootstrap_median(column, n_bootstrap)`

| Parameter | Description | Typical values |
|---|---|---|
| `n_bootstrap` | Number of bootstrap iterations | 1000 to 10000 |

```sql
-- Bootstrap confidence interval for the median
SELECT stat_bootstrap_median(income, 2000) AS result FROM households;

-- Evaluate estimation precision of the median
SELECT stat_median(val) AS median,
       json_extract(stat_bootstrap_median(val, 5000), '$.standard_error') AS se_median,
       json_extract(stat_bootstrap_median(val, 5000), '$.ci_lower') AS ci_lower,
       json_extract(stat_bootstrap_median(val, 5000), '$.ci_upper') AS ci_upper
FROM data;

-- With outliers: bootstrap of median is more stable than mean
SELECT stat_bootstrap_mean(val, 1000)   AS boot_mean,
       stat_bootstrap_median(val, 1000) AS boot_median
FROM data_with_outliers;
```

---

### stat_bootstrap_stddev

Performs **bootstrap estimation of the standard deviation**.

**Syntax**: `stat_bootstrap_stddev(column, n_bootstrap)`

| Parameter | Description | Typical values |
|---|---|---|
| `n_bootstrap` | Number of bootstrap iterations | 1000 to 10000 |

> **Minimum data count**: 2

```sql
-- Bootstrap estimation of standard deviation
SELECT stat_bootstrap_stddev(measurement, 2000) AS result FROM data;

-- Compare with parametric confidence interval for variance
SELECT stat_ci_var(val, 0.95) AS parametric_var_ci,
       stat_bootstrap_stddev(val, 5000) AS bootstrap_sd
FROM data;

-- Compare variability across groups (with bootstrap confidence intervals)
SELECT category,
       stat_sample_stddev(price) AS sd,
       stat_bootstrap_stddev(price, 1000) AS bootstrap
FROM products
GROUP BY category;
```

---
