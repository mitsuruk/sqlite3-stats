# Complex Aggregates (32 Functions)

Aggregate functions returning JSON results, two-sample tests, survival analysis, and more. These functions take multiple columns as input and return complex statistical results.

← [Function Reference](../function_reference.md)

---

## Basic Statistics (Multiple Results)

### stat_modes

Returns **all modes** as a JSON array.

**Syntax**: `stat_modes(column)`

```sql
SELECT stat_modes(score) FROM students;
-- → [85.0, 90.0] (when there are multiple modes)
```

---

### stat_five_number_summary

Returns a **five-number summary** (minimum, Q1, median, Q3, maximum) as JSON.

**Syntax**: `stat_five_number_summary(column)`

```sql
SELECT stat_five_number_summary(val) FROM data;
-- → {"min":1.0,"q1":3.0,"median":5.0,"q3":7.0,"max":10.0}
```

---

## Frequency Distributions

### stat_frequency_table

Returns a **frequency table** as JSON.

**Syntax**: `stat_frequency_table(column)`

```sql
SELECT stat_frequency_table(grade) FROM students;
```

---

### stat_frequency_count

Returns the **frequency count** for each value as JSON.

**Syntax**: `stat_frequency_count(column)`

```sql
SELECT stat_frequency_count(category) FROM products;
```

---

### stat_relative_frequency

Returns **relative frequencies** as JSON.

**Syntax**: `stat_relative_frequency(column)`

```sql
SELECT stat_relative_frequency(rating) FROM reviews;
```

---

### stat_cumulative_frequency

Returns **cumulative frequencies** as JSON.

**Syntax**: `stat_cumulative_frequency(column)`

```sql
SELECT stat_cumulative_frequency(score) FROM exam;
```

---

### stat_cumulative_relative_frequency

Returns **cumulative relative frequencies** as JSON.

**Syntax**: `stat_cumulative_relative_frequency(column)`

```sql
SELECT stat_cumulative_relative_frequency(score) FROM exam;
```

---

## Two-Sample Tests

### stat_t_test2

Performs a **two-sample t-test** (pooled variance). Pass the values of each group in two columns.

**Syntax**: `stat_t_test2(group1, group2)`

```sql
SELECT stat_t_test2(before_score, after_score) FROM experiment;
-- → {"statistic":...,"p_value":...,"df":...}
```

---

### stat_t_test_welch

Performs **Welch's t-test** (a two-sample test that does not assume equal variances).

**Syntax**: `stat_t_test_welch(group1, group2)`

```sql
SELECT stat_t_test_welch(control, treatment) FROM trial;
```

---

### stat_chisq_independence

Performs a **chi-square test of independence**. Pass two categorical variables.

**Syntax**: `stat_chisq_independence(col1, col2)`

```sql
SELECT stat_chisq_independence(gender, preference) FROM survey;
```

---

### stat_f_test

Performs an **F-test** (comparison of variances between two groups).

**Syntax**: `stat_f_test(group1, group2)`

```sql
SELECT stat_f_test(method_a, method_b) FROM quality;
```

---

### stat_mann_whitney

Performs a **Mann-Whitney U test** (nonparametric two-sample test).

**Syntax**: `stat_mann_whitney(group1, group2)`

```sql
SELECT stat_mann_whitney(drug, placebo) FROM trial;
```

---

## Analysis of Variance

### stat_anova1

Performs a **one-way ANOVA**. Pass values as the first argument and group labels as the second.

**Syntax**: `stat_anova1(value, group)`

```sql
SELECT stat_anova1(score, class) FROM students;
-- → {"f_statistic":...,"p_value":...,"df_between":...,"df_within":...}
```

---

## Categorical

### stat_contingency_table

Creates a **contingency table** as JSON. Pass two categorical variables.

**Syntax**: `stat_contingency_table(col1, col2)`

```sql
SELECT stat_contingency_table(treatment, outcome) FROM patients;
```

---

## Effect Size (Two-Sample)

### stat_cohens_d2

Computes **Cohen's d (two-sample)**.

**Syntax**: `stat_cohens_d2(group1, group2)`

```sql
SELECT stat_cohens_d2(control, treatment) FROM experiment;
```

---

### stat_hedges_g2

Computes **Hedges' g (two-sample)** (with small-sample correction).

**Syntax**: `stat_hedges_g2(group1, group2)`

```sql
SELECT stat_hedges_g2(control, treatment) FROM experiment;
```

---

### stat_glass_delta

Computes **Glass's delta** (standardized by the control group's standard deviation).

**Syntax**: `stat_glass_delta(control, treatment)`

```sql
SELECT stat_glass_delta(control, treatment) FROM experiment;
```

---

## Confidence Intervals for Two-Sample Differences

### stat_ci_mean_diff

Returns a **CI for two-sample mean difference** (pooled variance) as JSON.

**Syntax**: `stat_ci_mean_diff(group1, group2)`

```sql
SELECT stat_ci_mean_diff(before_val, after_val) FROM study;
```

---

### stat_ci_mean_diff_welch

Returns a **CI for two-sample mean difference (Welch)** as JSON.

**Syntax**: `stat_ci_mean_diff_welch(group1, group2)`

```sql
SELECT stat_ci_mean_diff_welch(control, treatment) FROM trial;
```

---

## Survival Analysis

### stat_kaplan_meier

Returns the **Kaplan-Meier survival curve** as JSON.

**Syntax**: `stat_kaplan_meier(time, event)`

| Parameter | Description |
|---|---|
| `time` | Survival time |
| `event` | Event occurred (1) / Censored (0) |

```sql
SELECT stat_kaplan_meier(survival_time, event_flag) FROM patients;
```

---

### stat_nelson_aalen

Returns the **Nelson-Aalen cumulative hazard** estimate as JSON.

**Syntax**: `stat_nelson_aalen(time, event)`

```sql
SELECT stat_nelson_aalen(time, event) FROM survival_data;
```

---

### stat_logrank

Returns the **log-rank test** (comparison of survival curves between two groups) as JSON. Takes three columns.

**Syntax**: `stat_logrank(time, event, group)`

| Parameter | Description |
|---|---|
| `time` | Survival time |
| `event` | Event occurred (1) / Censored (0) |
| `group` | Group label |

```sql
SELECT stat_logrank(time, event, treatment_group) FROM clinical_trial;
```

---

## Resampling

### stat_bootstrap

Returns **general bootstrap** estimates as JSON.

**Syntax**: `stat_bootstrap(column, n_bootstrap)`

```sql
SELECT stat_bootstrap(val, 1000) FROM data;
```

---

### stat_bootstrap_bca

Returns **BCa (bias-corrected and accelerated) bootstrap** results as JSON.

**Syntax**: `stat_bootstrap_bca(column, n_bootstrap)`

```sql
SELECT stat_bootstrap_bca(val, 1000) FROM data;
```

---

### stat_bootstrap_sample

Generates a **bootstrap sample** as a JSON array.

**Syntax**: `stat_bootstrap_sample(column)`

```sql
SELECT stat_bootstrap_sample(val) FROM data;
```

---

### stat_permutation_test2

Returns a **two-sample permutation test** result as JSON.

**Syntax**: `stat_permutation_test2(group1, group2)`

```sql
SELECT stat_permutation_test2(control, treatment) FROM experiment;
```

---

### stat_permutation_paired

Returns a **paired permutation test** result as JSON.

**Syntax**: `stat_permutation_paired(x, y)`

```sql
SELECT stat_permutation_paired(before, after) FROM paired_data;
```

---

### stat_permutation_corr

Returns a **correlation permutation test** result as JSON.

**Syntax**: `stat_permutation_corr(x, y)`

```sql
SELECT stat_permutation_corr(study_hours, test_score) FROM students;
```

---

## Time Series

### stat_acf

Returns the **autocorrelation function** (ACF) as a JSON array.

**Syntax**: `stat_acf(column, max_lag)`

```sql
SELECT stat_acf(price, 20) FROM stock_daily;
```

---

### stat_pacf

Returns the **partial autocorrelation function** (PACF) as a JSON array.

**Syntax**: `stat_pacf(column, max_lag)`

```sql
SELECT stat_pacf(price, 20) FROM stock_daily;
```

---

## Sampling

### stat_sample_replace

Returns **sampling with replacement** (duplicates allowed) as a JSON array.

**Syntax**: `stat_sample_replace(column, n)`

```sql
SELECT stat_sample_replace(val, 10) FROM data;
```

---

### stat_sample

Returns **sampling without replacement** (no duplicates) as a JSON array.

**Syntax**: `stat_sample(column, n)`

```sql
SELECT stat_sample(val, 5) FROM data;
```

---
