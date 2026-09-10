# Complex Aggregates (41 Functions)

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

## Group-Column Tests

Take a value column and a group column, exactly like `stat_anova1`. Groups are
numbered 0, 1, 2, … in ascending order of the group column's values.

| Function | Syntax | Description |
|---|---|---|
| `stat_kruskal_wallis` | `(val, grp)` | Kruskal-Wallis test |
| `stat_levene` | `(val, grp)` | Levene test (homogeneity) |
| `stat_bartlett` | `(val, grp)` | Bartlett test (homogeneity) |
| `stat_cohens_f` | `(val, grp)` | Cohen's f (ANOVA effect size) |

`stat_kruskal_wallis` is the nonparametric counterpart of one-way ANOVA,
used when normality is doubtful. Results match R's `kruskal.test()`.

`stat_levene` and `stat_bartlett` test the equal-variance assumption that
`stat_anova1` and `stat_t_test2` rely on. Levene uses the **median-based
Brown-Forsythe** form, which is robust to non-normality and matches the default
of R's `car::leveneTest()`. Bartlett assumes normality and is more powerful when
that holds, matching R's `bartlett.test()`.

All three return `{"statistic", "p_value", "df"}` and yield NULL when fewer than
two groups are present. NULL rows are excluded before grouping.

```sql
-- Check the equal-variance assumption before running ANOVA
SELECT stat_levene(score, class_id)  AS levene,
       stat_bartlett(score, class_id) AS bartlett,
       stat_anova1(score, class_id)   AS anova
FROM exam_results;

-- Use the nonparametric test when normality is doubtful
SELECT stat_kruskal_wallis(score, class_id) FROM exam_results;
```

---

## Post-hoc Tests

| Function | Syntax | Scope |
|---|---|---|
| `stat_tukey_hsd` | `(val, grp [,alpha])` | all pairs |
| `stat_bonferroni_posthoc` | `(val, grp [,alpha])` | all pairs |
| `stat_scheffe_posthoc` | `(val, grp [,alpha])` | all pairs |
| `stat_dunnett_posthoc` | `(val, grp [,ctrl, alpha])` | vs. control |

Run after `stat_anova1` reports a significant difference, to identify which
groups differ. One-way ANOVA is computed internally, so these are called on the
raw value and group columns. `alpha` defaults to 0.05; `ctrl` (the control
group index for Dunnett) defaults to 0.

Conservativeness increases Tukey < Bonferroni < Scheffe. Dunnett compares every
group against one control only, giving k-1 comparisons instead of k(k-1)/2.

**Return value**:

```json
{"method": "Tukey HSD", "alpha": 0.05, "mse": 2.5, "df_error": 12,
 "comparisons": [
   {"group1": 0, "group2": 1, "mean_diff": -9.0, "se": 0.707,
    "statistic": 12.73, "p_value": 3.08e-06,
    "lower": -11.67, "upper": -6.33, "significant": true}]}
```

> **Sign convention**: `mean_diff` is group1 - group2, where group1 always has
> the lower index. R's `TukeyHSD()` reports the opposite direction ("2-1"), so
> the mean difference and both interval bounds appear negated relative to R.
> Magnitudes and p-values are identical.
>
> **Note**: `stat_dunnett_posthoc` uses a Bonferroni approximation rather than
> the exact multivariate t distribution, so it is slightly conservative
> compared with R's `multcomp::glht()`.

```sql
-- Identify which classes differ after a significant ANOVA
SELECT stat_tukey_hsd(score, class_id) FROM exam_results;

-- Extract only the significant pairs
SELECT json_extract(c.value, '$.group1') AS g1,
       json_extract(c.value, '$.group2') AS g2,
       json_extract(c.value, '$.p_value') AS p
FROM (SELECT stat_tukey_hsd(score, class_id) AS j FROM exam_results) t,
     json_each(t.j, '$.comparisons') c
WHERE json_extract(c.value, '$.significant');

-- Compare every treatment against the control group (index 0)
SELECT stat_dunnett_posthoc(score, class_id, 0, 0.05) FROM exam_results;
```

---

## Stratified Sampling

| Function | Syntax |
|---|---|
| `stat_stratified_sample` | `(val, grp [,ratio])` |

Draws the same **proportion** from each stratum, so the group composition of the
sample matches the population. `ratio` is in (0, 1] and defaults to 0.5.
Returns a JSON array of the sampled values.

```sql
-- Take 30% of each region, preserving regional proportions
SELECT stat_stratified_sample(revenue, region_id, 0.3) FROM sales;
```

> **Note**: the sample is random and is not currently reproducible; there is no
> seed control yet.

---
