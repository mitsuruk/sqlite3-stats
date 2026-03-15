# Basic Aggregates (24 Functions)

Single-column aggregate functions. Used in the form `SELECT stat_xxx(column) FROM table`. Available in all contexts where SQLite3 standard aggregate functions can be used, including `GROUP BY`, `HAVING`, subqueries, etc.

← [Function Reference](../function_reference.md)

---

## Basic Statistics

### stat_mean

Computes the **arithmetic mean**.

$$\bar{x} = \frac{1}{n}\sum_{i=1}^{n} x_i$$

**Syntax**: `stat_mean(column)`

```sql
-- Basic usage
SELECT stat_mean(score) FROM students;

-- Average salary by department
SELECT department, stat_mean(salary) AS avg_salary
FROM employees
GROUP BY department;

-- Calculate difference from mean using subquery
SELECT name, score,
       score - (SELECT stat_mean(score) FROM students) AS diff_from_mean
FROM students;

-- Extract groups with mean above threshold using HAVING
SELECT class, stat_mean(score) AS avg_score
FROM students
GROUP BY class
HAVING stat_mean(score) >= 70;

-- Comparison with SQLite3 built-in AVG()
SELECT stat_mean(val) AS stat_mean_result,
       AVG(val)        AS sqlite_avg_result
FROM data;
-- → Both return the same result
```

---

### stat_median

Computes the **median**. The middle value when data is sorted. For an even number of elements, returns the average of the two middle values.

**Syntax**: `stat_median(column)`

```sql
-- Basic usage
SELECT stat_median(price) FROM products;

-- Compare mean and median to check distribution skewness
SELECT stat_mean(income)   AS mean_income,
       stat_median(income) AS median_income
FROM households;
-- mean > median → right-skewed distribution (influence of high earners)
-- mean ≈ median → approximately symmetric distribution

-- Median by category
SELECT region,
       stat_median(sales) AS median_sales
FROM store_data
GROUP BY region
ORDER BY median_sales DESC;

-- Median as a representative value robust to outliers
-- Example: [1, 2, 3, 4, 1000] → mean=202, median=3
CREATE TABLE outlier_data (val REAL);
INSERT INTO outlier_data VALUES (1),(2),(3),(4),(1000);

SELECT stat_mean(val)   AS mean,    -- 202.0
       stat_median(val) AS median   -- 3.0
FROM outlier_data;
```

---

### stat_mode

Returns the **mode** (the most frequently occurring value). If there are multiple modes, returns the smallest value.

**Syntax**: `stat_mode(column)`

```sql
-- Basic usage
SELECT stat_mode(shoe_size) FROM customers;

-- Compare mode with mean and median
SELECT stat_mode(rating)   AS mode_rating,
       stat_mean(rating)   AS mean_rating,
       stat_median(rating) AS median_rating
FROM reviews;

-- Mode by group
SELECT category, stat_mode(color) AS most_common_color
FROM products
GROUP BY category;

-- If all values are unique, returns the smallest value
CREATE TABLE unique_data (val REAL);
INSERT INTO unique_data VALUES (5),(3),(1),(4),(2);

SELECT stat_mode(val) FROM unique_data;
-- → 1.0 (all values appear once, so returns the smallest)
```

---

### stat_geometric_mean

Computes the **geometric mean**. Suitable for averaging growth rates and ratios.

$$\bar{x}_g = \left(\prod_{i=1}^{n} x_i\right)^{1/n}$$

**Syntax**: `stat_geometric_mean(column)`

> **Constraint**: All values must be positive.

```sql
-- Average annual growth rate of an investment
-- 3-year growth rates: 1.10 (10%), 1.05 (5%), 1.20 (20%)
CREATE TABLE growth (rate REAL);
INSERT INTO growth VALUES (1.10),(1.05),(1.20);

SELECT stat_geometric_mean(rate) AS avg_growth_rate;
-- → 1.11535... (average +11.5%)

-- Difference from arithmetic mean
SELECT stat_mean(rate)           AS arithmetic_mean,  -- 1.1167
       stat_geometric_mean(rate) AS geometric_mean     -- 1.1154
FROM growth;
-- Geometric mean is always ≤ arithmetic mean (AM-GM inequality)

-- Average multiplication factor of bacterial growth
SELECT stat_geometric_mean(multiplication_factor) AS avg_factor
FROM bacteria_growth;
```

---

### stat_harmonic_mean

Computes the **harmonic mean**. Suitable for averaging speeds and ratios.

$$\bar{x}_h = \frac{n}{\sum_{i=1}^{n} \frac{1}{x_i}}$$

**Syntax**: `stat_harmonic_mean(column)`

> **Constraint**: All values must be non-zero.

```sql
-- Average speed for a round trip
-- Outbound: 60 km/h, return: 40 km/h
CREATE TABLE speed (kmph REAL);
INSERT INTO speed VALUES (60),(40);

SELECT stat_harmonic_mean(kmph) AS avg_speed;
-- → 48.0 km/h (less than arithmetic mean of 50; this is the correct average speed)

-- Comparison of three types of means
SELECT stat_mean(val)           AS arithmetic,
       stat_geometric_mean(val) AS geometric,
       stat_harmonic_mean(val)  AS harmonic
FROM positive_data;
-- Always: harmonic ≤ geometric ≤ arithmetic

-- Average price-to-earnings ratio (P/E ratio)
SELECT stat_harmonic_mean(pe_ratio) AS avg_pe
FROM stocks
WHERE pe_ratio > 0;
```

---

## Dispersion / Variability

### stat_range

Computes the **range** (maximum - minimum).

**Syntax**: `stat_range(column)`

```sql
-- Basic usage
SELECT stat_range(temperature) AS temp_range FROM weather;

-- Compare range across groups
SELECT sensor_id,
       MIN(reading)        AS min_val,
       MAX(reading)        AS max_val,
       stat_range(reading) AS range_val
FROM sensor_data
GROUP BY sensor_id;

-- Quality control: detect batches where range exceeds threshold
SELECT batch_id, stat_range(measurement) AS r
FROM quality_data
GROUP BY batch_id
HAVING stat_range(measurement) > 5.0;
```

---

### stat_var

Computes the **variance** (population variance, ddof=0).

$$\sigma^2 = \frac{1}{n}\sum_{i=1}^{n}(x_i - \bar{x})^2$$

**Syntax**: `stat_var(column)`

> **Note**: `stat_var` returns population variance (ddof=0). Use `stat_sample_variance` if you need sample variance (unbiased).

```sql
-- Population variance
SELECT stat_var(score) AS variance FROM exam;

-- Comparison of population and sample variance
SELECT stat_var(score)             AS pop_var,   -- divides by N
       stat_sample_variance(score) AS sample_var  -- divides by N-1
FROM exam;
```

---

### stat_population_variance

Computes the **population variance**. Equivalent to `stat_var`.

$$\sigma^2 = \frac{1}{N}\sum_{i=1}^{N}(x_i - \mu)^2$$

**Syntax**: `stat_population_variance(column)`

```sql
-- Use when working with complete population data (census)
SELECT stat_population_variance(height) FROM census_data;

-- Quality metric for all products on a production line
SELECT production_line,
       stat_population_variance(weight) AS var_weight
FROM all_products
GROUP BY production_line;
```

---

### stat_sample_variance

Computes the **sample variance** (unbiased variance).

$$s^2 = \frac{1}{n-1}\sum_{i=1}^{n}(x_i - \bar{x})^2$$

**Syntax**: `stat_sample_variance(column)`

> **Minimum data count**: 2

```sql
-- Use when estimating population variance from a sample
SELECT stat_sample_variance(weight) FROM sample_products;

-- Compare variance between two groups
SELECT group_name,
       stat_sample_variance(score) AS var_score
FROM experiment
GROUP BY group_name;

-- Identify the category with the largest variance
SELECT category, stat_sample_variance(price) AS var_price
FROM products
GROUP BY category
ORDER BY var_price DESC
LIMIT 1;
```

---

### stat_stdev

Computes the **standard deviation** (population standard deviation, ddof=0).

$$\sigma = \sqrt{\frac{1}{n}\sum_{i=1}^{n}(x_i - \bar{x})^2}$$

**Syntax**: `stat_stdev(column)`

> **Note**: `stat_stdev` returns population standard deviation (ddof=0). Use `stat_sample_stddev` if you need sample standard deviation.

```sql
-- Population standard deviation
SELECT stat_stdev(score) AS pop_stddev FROM exam;

-- Check the proportion within mean ± 1 sigma
SELECT
    CAST(COUNT(CASE
        WHEN score BETWEEN
            (SELECT stat_mean(score) FROM exam) - (SELECT stat_stdev(score) FROM exam)
        AND (SELECT stat_mean(score) FROM exam) + (SELECT stat_stdev(score) FROM exam)
        THEN 1 END) AS REAL) / COUNT(*) * 100 AS pct_within_1sigma
FROM exam;
-- Approximately 68% for a normal distribution
```

---

### stat_population_stddev

Computes the **population standard deviation**. Equivalent to `stat_stdev`.

**Syntax**: `stat_population_stddev(column)`

```sql
SELECT stat_population_stddev(measurement) FROM all_measurements;
```

---

### stat_sample_stddev

Computes the **sample standard deviation**.

$$s = \sqrt{\frac{1}{n-1}\sum_{i=1}^{n}(x_i - \bar{x})^2}$$

**Syntax**: `stat_sample_stddev(column)`

> **Minimum data count**: 2

```sql
-- Sample standard deviation
SELECT stat_sample_stddev(weight) FROM sample_data;

-- Get mean and standard deviation at once
SELECT stat_mean(score)          AS mean,
       stat_sample_stddev(score) AS stddev
FROM test_scores;

-- Compute descriptive statistics by group in one query
SELECT department,
       COUNT(salary)                  AS n,
       stat_mean(salary)              AS mean,
       stat_sample_stddev(salary)     AS stddev,
       stat_median(salary)            AS median,
       MIN(salary)                    AS min,
       MAX(salary)                    AS max
FROM employees
GROUP BY department;
```

---

### stat_cv

Computes the **coefficient of variation**. The sample standard deviation divided by the absolute value of the mean, enabling comparison of variability across data with different units.

$$CV = \frac{s}{|\bar{x}|}$$

**Syntax**: `stat_cv(column)`

> **Minimum data count**: 2

```sql
-- Coefficient of variation
SELECT stat_cv(height) AS cv_height,
       stat_cv(weight) AS cv_weight
FROM people;
-- Enables comparison of which has relatively more variability: height or weight

-- Higher CV indicates greater variability
SELECT product_type,
       stat_mean(price) AS mean_price,
       stat_cv(price)   AS cv_price
FROM products
GROUP BY product_type
ORDER BY cv_price DESC;

-- Used as a stability indicator
-- CV < 0.1: very stable, CV > 0.3: high variability
SELECT sensor_id, stat_cv(reading) AS stability
FROM readings
GROUP BY sensor_id
HAVING stat_cv(reading) > 0.3;
```

---

### stat_iqr

Computes the **interquartile range** (IQR). Q3 - Q1.

**Syntax**: `stat_iqr(column)`

```sql
-- Interquartile range
SELECT stat_iqr(score) AS iqr FROM exam;

-- IQR as a criterion for outlier detection
-- Common rule: below Q1 - 1.5*IQR or above Q3 + 1.5*IQR
-- * For Q1 and Q3 computation, see stat_quartile

-- Summarize distribution with median and IQR (descriptive statistics robust to outliers)
SELECT stat_median(price) AS median_price,
       stat_iqr(price)    AS iqr_price
FROM listings;

-- Compare IQR across groups
SELECT category,
       stat_iqr(duration) AS iqr_duration
FROM events
GROUP BY category;
```

---

### stat_mad_mean

Computes the **mean absolute deviation** (Mean Absolute Deviation from mean).

$$MAD_{mean} = \frac{1}{n}\sum_{i=1}^{n}|x_i - \bar{x}|$$

**Syntax**: `stat_mad_mean(column)`

```sql
-- Mean absolute deviation
SELECT stat_mad_mean(score) AS mad_from_mean FROM exam;

-- Comparison with standard deviation
-- Mean absolute deviation is less sensitive to outliers
SELECT stat_mad_mean(val) AS mad_mean,
       stat_stdev(val)    AS stdev
FROM data;

-- Compare variability across groups
SELECT region,
       stat_mad_mean(delivery_time) AS avg_deviation
FROM orders
GROUP BY region;
```

---

### stat_geometric_stddev

Computes the **geometric standard deviation**. A measure of variability for data following a log-normal distribution.

**Syntax**: `stat_geometric_stddev(column)`

> **Constraint**: All values must be positive.

```sql
-- Geometric standard deviation
SELECT stat_geometric_mean(concentration) AS geo_mean,
       stat_geometric_stddev(concentration) AS geo_stddev
FROM environmental_samples;

-- Useful for log-normally distributed data (particle sizes, concentrations, income, etc.)
-- Geometric mean × geometric stddev^±1 roughly corresponds to the 68% range
SELECT stat_geometric_mean(particle_size) AS gm,
       stat_geometric_stddev(particle_size) AS gsd
FROM aerosol_data;
```

---

## Distribution Shape

### stat_population_skewness

Computes the **population skewness**. A measure of distribution asymmetry.

$$g_1 = \frac{\frac{1}{N}\sum_{i=1}^{N}(x_i - \mu)^3}{\sigma^3}$$

**Syntax**: `stat_population_skewness(column)`

> **Minimum data count**: 3

```sql
-- Interpreting skewness
SELECT stat_population_skewness(income) AS skewness FROM survey;
-- skewness > 0: right-tailed (positive skew) — common in income distributions
-- skewness = 0: symmetric (close to normal distribution)
-- skewness < 0: left-tailed (negative skew)

-- Review distribution shape at a glance
SELECT stat_mean(val)                 AS mean,
       stat_median(val)               AS median,
       stat_population_skewness(val)  AS skewness,
       stat_population_kurtosis(val)  AS kurtosis
FROM measurements;
```

---

### stat_skewness

Computes the **sample skewness** (with Fisher's correction).

$$G_1 = \frac{n}{(n-1)(n-2)} \cdot \frac{\sum_{i=1}^{n}(x_i - \bar{x})^3}{s^3}$$

**Syntax**: `stat_skewness(column)`

> **Minimum data count**: 3

```sql
-- Sample skewness (estimate population skewness from a sample)
SELECT stat_skewness(response_time) AS skewness FROM api_logs;

-- Comparison of population and sample skewness
SELECT stat_population_skewness(val) AS pop_skew,
       stat_skewness(val)            AS sample_skew
FROM data;
-- The two converge as n increases

-- Quick normality check
-- If |skewness| < 2 and |kurtosis| < 7, roughly considered normal
SELECT stat_skewness(val)  AS skew,
       stat_kurtosis(val)  AS kurt
FROM data;
```

---

### stat_population_kurtosis

Computes the **population kurtosis** (excess kurtosis). A measure of peakedness relative to the normal distribution (baseline = 0).

$$g_2 = \frac{\frac{1}{N}\sum_{i=1}^{N}(x_i - \mu)^4}{\sigma^4} - 3$$

**Syntax**: `stat_population_kurtosis(column)`

> **Minimum data count**: 4

```sql
-- Interpreting kurtosis
SELECT stat_population_kurtosis(val) AS kurtosis FROM data;
-- kurtosis > 0: peaked distribution (heavy tails) — leptokurtic
-- kurtosis = 0: same as normal distribution — mesokurtic
-- kurtosis < 0: flat distribution (light tails) — platykurtic

-- Risk assessment of financial data
-- High kurtosis → extreme values (crashes/spikes) are more likely
SELECT asset_name,
       stat_population_kurtosis(daily_return) AS kurtosis
FROM returns
GROUP BY asset_name
ORDER BY kurtosis DESC;
```

---

### stat_kurtosis

Computes the **sample kurtosis** (excess kurtosis, with Fisher's correction).

**Syntax**: `stat_kurtosis(column)`

> **Minimum data count**: 4

```sql
-- Sample kurtosis
SELECT stat_kurtosis(val) AS kurtosis FROM sample_data;

-- Complete descriptive statistics of distribution shape
SELECT stat_mean(val)     AS mean,
       stat_median(val)   AS median,
       stat_stdev(val)    AS stddev,
       stat_skewness(val) AS skewness,
       stat_kurtosis(val) AS kurtosis
FROM sample_data;
```

---

## Estimation

### stat_se

Computes the **standard error** (Standard Error of the Mean). A measure of the precision of the mean estimate.

$$SE = \frac{s}{\sqrt{n}}$$

**Syntax**: `stat_se(column)`

> **Minimum data count**: 2

```sql
-- Standard error
SELECT stat_se(score) AS se FROM exam;

-- Approximate 95% confidence interval for the mean (95% CI ≈ mean ± 1.96 * SE)
SELECT stat_mean(val) AS mean,
       stat_se(val)   AS se,
       stat_mean(val) - 1.96 * stat_se(val) AS ci_lower_95,
       stat_mean(val) + 1.96 * stat_se(val) AS ci_upper_95
FROM large_sample;
-- Note: This is an approximation valid for sufficiently large n. For exact CI, see stat_ci_mean.

-- Relationship between sample size and standard error
SELECT COUNT(val) AS n,
       stat_sample_stddev(val) AS stddev,
       stat_se(val)            AS se
FROM data;
-- When n quadruples, SE halves

-- Compare estimation precision across groups
SELECT group_name,
       COUNT(score) AS n,
       stat_mean(score) AS mean,
       stat_se(score)   AS se
FROM experiment
GROUP BY group_name;
```

---

## Robust Statistics

### stat_mad

Computes the **median absolute deviation** (MAD). A measure of variability that is highly robust to outliers.

$$MAD = \text{median}(|x_i - \tilde{x}|)$$

where $\tilde{x}$ is the median of the data.

**Syntax**: `stat_mad(column)`

```sql
-- MAD (variability measure robust to outliers)
SELECT stat_mad(val) AS mad FROM data;

-- Comparison with standard deviation (when outliers are present)
CREATE TABLE with_outlier (val REAL);
INSERT INTO with_outlier VALUES (1),(2),(3),(4),(5),(100);

SELECT stat_stdev(val) AS stdev,  -- inflated by outlier
       stat_mad(val)   AS mad     -- less affected by outlier
FROM with_outlier;

-- Using MAD as a criterion for outlier detection via Modified Z-score
-- Modified Z-score = 0.6745 * (x - median) / MAD
-- Consider as outlier if |Modified Z-score| > 3.5
SELECT val,
       0.6745 * (val - (SELECT stat_median(val) FROM data))
               / (SELECT stat_mad(val) FROM data) AS modified_z
FROM data;
```

---

### stat_mad_scaled

Computes the **scaled MAD**. MAD corrected to be on the same scale as the standard deviation under a normal distribution.

$$MAD_{scaled} = 1.4826 \times MAD$$

**Syntax**: `stat_mad_scaled(column)`

```sql
-- Under a normal distribution, close to stat_sample_stddev
SELECT stat_sample_stddev(val) AS stddev,
       stat_mad_scaled(val)    AS mad_scaled
FROM normal_data;

-- When outliers are present, MAD_scaled is more robust
SELECT stat_sample_stddev(val) AS stddev,     -- inflated by outliers
       stat_mad_scaled(val)    AS mad_scaled   -- robust
FROM data_with_outliers;

-- Use in quality control
SELECT batch_id,
       stat_median(weight)     AS center,
       stat_mad_scaled(weight) AS spread
FROM production
GROUP BY batch_id;
```

---

### stat_hodges_lehmann

Computes the **Hodges-Lehmann estimator**. The median of all pairwise means. A robust estimator of location.

$$\hat{\theta}_{HL} = \text{median}\left\{\frac{x_i + x_j}{2} : 1 \le i \le j \le n\right\}$$

**Syntax**: `stat_hodges_lehmann(column)`

```sql
-- Hodges-Lehmann estimator (robust location estimate)
SELECT stat_hodges_lehmann(val) AS hl_estimate FROM data;

-- Comparison with mean and median
SELECT stat_mean(val)            AS mean,
       stat_median(val)          AS median,
       stat_hodges_lehmann(val)  AS hodges_lehmann
FROM data;

-- Comparison when outliers are present
CREATE TABLE hl_test (val REAL);
INSERT INTO hl_test VALUES (1),(2),(3),(4),(5),(100);

SELECT stat_mean(val)            AS mean,           -- 19.17 (pulled by outlier)
       stat_median(val)          AS median,          -- 3.5
       stat_hodges_lehmann(val)  AS hodges_lehmann   -- robustness between mean and median
FROM hl_test;

-- Also used as the point estimate for the Wilcoxon signed-rank test
SELECT stat_hodges_lehmann(val) AS location_estimate
FROM treatment_effects;
```

---
