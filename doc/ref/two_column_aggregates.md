# Two-Column Aggregates (27 Functions)

Two-column aggregate functions. Used in the form `SELECT stat_xxx(column_x, column_y) FROM table`. Available in all contexts where SQLite3 standard aggregate functions can be used, including `GROUP BY`, `HAVING`, subqueries, etc.

← [Function Reference](../function_reference.md)

---

## Correlation / Covariance

### stat_population_covariance

Computes the **population covariance**.

$$\text{Cov}_{pop}(X,Y) = \frac{1}{n}\sum_{i=1}^{n}(x_i - \bar{x})(y_i - \bar{y})$$

**Syntax**: `stat_population_covariance(column_x, column_y)`

> **Minimum data count**: 1

```sql
-- Population covariance of two variables
SELECT stat_population_covariance(height, weight) FROM students;

-- Covariance by group
SELECT department,
       stat_population_covariance(experience, salary) AS cov
FROM employees
GROUP BY department;
```

---

### stat_covariance

Computes the **sample covariance** (unbiased covariance).

$$\text{Cov}(X,Y) = \frac{1}{n-1}\sum_{i=1}^{n}(x_i - \bar{x})(y_i - \bar{y})$$

**Syntax**: `stat_covariance(column_x, column_y)`

> **Minimum data count**: 2

```sql
SELECT stat_covariance(x, y) FROM measurements;
```

---

### stat_pearson_r

Computes the **Pearson product-moment correlation coefficient**.

$$r = \frac{\text{Cov}(X,Y)}{s_X \cdot s_Y}$$

**Syntax**: `stat_pearson_r(column_x, column_y)`

> **Minimum data count**: 2
> **Return value range**: -1.0 to 1.0

```sql
-- Correlation between height and weight
SELECT stat_pearson_r(height, weight) FROM students;

-- Correlation matrix-style analysis
SELECT 'height-weight' AS pair, stat_pearson_r(height, weight) AS r FROM data
UNION ALL
SELECT 'height-age', stat_pearson_r(height, age) FROM data
UNION ALL
SELECT 'weight-age', stat_pearson_r(weight, age) FROM data;

-- Correlation by group
SELECT region,
       stat_pearson_r(advertising, sales) AS correlation
FROM marketing
GROUP BY region;
```

---

### stat_spearman_r

Computes the **Spearman rank correlation coefficient**. Converts data to ranks before computing the Pearson correlation. Robust against outliers and nonlinear monotonic relationships.

**Syntax**: `stat_spearman_r(column_x, column_y)`

> **Minimum data count**: 2
> **Return value range**: -1.0 to 1.0

```sql
-- Rank correlation (robust to outliers)
SELECT stat_spearman_r(rank_math, rank_english) FROM students;

-- Compare Pearson and Spearman (detecting nonlinearity)
SELECT stat_pearson_r(x, y) AS pearson,
       stat_spearman_r(x, y) AS spearman
FROM data;
```

---

### stat_kendall_tau

Computes the **Kendall rank correlation coefficient** (tau-b). A rank correlation that accounts for tied ranks.

**Syntax**: `stat_kendall_tau(column_x, column_y)`

> **Minimum data count**: 2
> **Return value range**: -1.0 to 1.0

```sql
-- Rank correlation (with tie handling)
SELECT stat_kendall_tau(preference_a, preference_b) FROM survey;

-- Compare three correlation coefficients
SELECT stat_pearson_r(x, y) AS pearson,
       stat_spearman_r(x, y) AS spearman,
       stat_kendall_tau(x, y) AS kendall
FROM data;
```

---

### stat_weighted_covariance

Computes the **weighted covariance**. When all weights are equal, it matches the sample covariance.

**Syntax**: `stat_weighted_covariance(column_values, column_weights)`

> **Minimum data count**: 2

```sql
-- Covariance weighted by confidence
SELECT stat_weighted_covariance(measurement, confidence) FROM results;
```

---

## Weighted Statistics

### stat_weighted_mean

Computes the **weighted mean**.

$$\bar{x}_w = \frac{\sum w_i x_i}{\sum w_i}$$

**Syntax**: `stat_weighted_mean(column_values, column_weights)`

> **Minimum data count**: 1

```sql
-- Mean weighted by confidence
SELECT stat_weighted_mean(score, confidence) FROM evaluations;

-- Population-weighted mean income
SELECT stat_weighted_mean(income, population) FROM regions;

-- Weighted mean by group
SELECT category,
       stat_weighted_mean(price, quantity) AS weighted_avg_price
FROM products
GROUP BY category;
```

---

### stat_weighted_harmonic_mean

Computes the **weighted harmonic mean**. Appropriate for weighted averages of rates and ratios.

**Syntax**: `stat_weighted_harmonic_mean(column_values, column_weights)`

> **Minimum data count**: 1 (all values must be positive)

```sql
-- Harmonic mean of speed weighted by distance
SELECT stat_weighted_harmonic_mean(speed, distance) FROM trips;
```

---

### stat_weighted_variance

Computes the **weighted variance**.

**Syntax**: `stat_weighted_variance(column_values, column_weights)`

> **Minimum data count**: 2

```sql
SELECT stat_weighted_variance(return_rate, market_cap) FROM stocks;
```

---

### stat_weighted_stddev

Computes the **weighted standard deviation** (square root of the weighted variance).

**Syntax**: `stat_weighted_stddev(column_values, column_weights)`

> **Minimum data count**: 2

```sql
SELECT stat_weighted_stddev(score, sample_size) FROM studies;
```

---

### stat_weighted_median

Computes the **weighted median**.

**Syntax**: `stat_weighted_median(column_values, column_weights)`

> **Minimum data count**: 1

```sql
-- Population-weighted median income
SELECT stat_weighted_median(median_income, population) FROM cities;
```

---

### stat_weighted_percentile

Computes the **weighted percentile**.

**Syntax**: `stat_weighted_percentile(column_values, column_weights, p)`

| Parameter | Description | Range |
|---|---|---|
| `p` | Percentile position | 0.0 to 1.0 |

> **Minimum data count**: 1

```sql
-- Weighted 25th percentile
SELECT stat_weighted_percentile(income, population, 0.25) FROM regions;

-- Weighted median (equivalent to weighted_median)
SELECT stat_weighted_percentile(income, population, 0.5) FROM regions;
```

---

## Linear Regression

### stat_simple_regression

Performs **simple linear regression**. Returns various regression statistics as JSON using the ordinary least squares method.

$$y = a + bx$$

**Syntax**: `stat_simple_regression(column_x, column_y)`

**Return value**: JSON

```json
{
  "intercept": intercept a,
  "slope": slope b,
  "r_squared": coefficient of determination R²,
  "adj_r_squared": adjusted R²,
  "intercept_se": standard error of the intercept,
  "slope_se": standard error of the slope,
  "intercept_t": t-value of the intercept,
  "slope_t": t-value of the slope,
  "intercept_p": p-value of the intercept,
  "slope_p": p-value of the slope,
  "f_statistic": F-statistic,
  "f_p_value": p-value of the F-test,
  "residual_se": residual standard error
}
```

> **Minimum data count**: 3

```sql
-- Simple linear regression
SELECT stat_simple_regression(study_hours, score) FROM students;

-- Extract slope and p-value
SELECT json_extract(stat_simple_regression(x, y), '$.slope') AS slope,
       json_extract(stat_simple_regression(x, y), '$.slope_p') AS p_value
FROM data;

-- Regression by group
SELECT department,
       stat_simple_regression(experience, salary) AS regression
FROM employees
GROUP BY department;
```

---

### stat_r_squared

Computes the **coefficient of determination** (R²). Evaluates the goodness of fit between actual and predicted values.

$$R^2 = 1 - \frac{SS_{res}}{SS_{tot}}$$

**Syntax**: `stat_r_squared(column_actual, column_predicted)`

> **Minimum data count**: 2
> **Return value range**: typically 0.0 to 1.0 (can be negative)

```sql
-- Model goodness of fit
SELECT stat_r_squared(actual_sales, predicted_sales) FROM forecasts;
```

---

### stat_adjusted_r_squared

Computes the **adjusted coefficient of determination**. R² corrected for the number of predictors (assumed to be 1).

**Syntax**: `stat_adjusted_r_squared(column_actual, column_predicted)`

> **Minimum data count**: 3

```sql
-- Compare adjusted R² and R²
SELECT stat_r_squared(actual, predicted) AS r2,
       stat_adjusted_r_squared(actual, predicted) AS adj_r2
FROM model_results;
```

---

## Paired Tests / Two-Sample Tests

### stat_t_test_paired

Performs a **paired t-test**. Tests whether the population mean of the differences d = x - y is 0.

$$t = \frac{\bar{d}}{s_d / \sqrt{n}}, \quad df = n - 1$$

**Syntax**: `stat_t_test_paired(column_x, column_y)`

**Return value**: JSON `{"statistic": t-value, "p_value": ..., "df": ...}`

> **Minimum data count**: 2

```sql
-- Drug efficacy (before vs. after treatment)
SELECT stat_t_test_paired(before_treatment, after_treatment) FROM patients;

-- Extract p-value only
SELECT json_extract(
    stat_t_test_paired(pre_score, post_score), '$.p_value'
) AS p_value
FROM training_data;
```

---

### stat_chisq_gof

Performs a **chi-square goodness-of-fit test**. Tests whether observed values fit the expected values.

$$\chi^2 = \sum_{i=1}^{k}\frac{(O_i - E_i)^2}{E_i}, \quad df = k - 1$$

**Syntax**: `stat_chisq_gof(column_observed, column_expected)`

**Return value**: JSON `{"statistic": chi-square value, "p_value": ..., "df": ...}`

> **Minimum data count**: 2

```sql
-- Goodness-of-fit test against expected distribution
SELECT stat_chisq_gof(observed_count, expected_count) FROM frequency_data;
```

---

## Error Metrics

### stat_mae

Computes the **mean absolute error** (MAE).

$$\text{MAE} = \frac{1}{n}\sum_{i=1}^{n}|y_i - \hat{y}_i|$$

**Syntax**: `stat_mae(column_actual, column_predicted)`

> **Minimum data count**: 1

```sql
-- Evaluate prediction error
SELECT stat_mae(actual, predicted) AS mae FROM forecasts;

-- Compare multiple models
SELECT 'model_A' AS model, stat_mae(actual, pred_a) AS mae FROM results
UNION ALL
SELECT 'model_B', stat_mae(actual, pred_b) FROM results;
```

---

### stat_mse

Computes the **mean squared error** (MSE).

$$\text{MSE} = \frac{1}{n}\sum_{i=1}^{n}(y_i - \hat{y}_i)^2$$

**Syntax**: `stat_mse(column_actual, column_predicted)`

> **Minimum data count**: 1

```sql
SELECT stat_mse(actual, predicted) AS mse FROM forecasts;
```

---

### stat_rmse

Computes the **root mean squared error** (RMSE). Square root of MSE.

$$\text{RMSE} = \sqrt{\text{MSE}}$$

**Syntax**: `stat_rmse(column_actual, column_predicted)`

> **Minimum data count**: 1

```sql
SELECT stat_rmse(actual, predicted) AS rmse FROM forecasts;
```

---

### stat_mape

Computes the **mean absolute percentage error** (MAPE). Returns the result as a percentage (%).

$$\text{MAPE} = \frac{100}{n}\sum_{i=1}^{n}\left|\frac{y_i - \hat{y}_i}{y_i}\right|$$

**Syntax**: `stat_mape(column_actual, column_predicted)`

> **Minimum data count**: 1 (rows where actual is 0 are excluded)

```sql
SELECT stat_mape(actual_sales, predicted_sales) AS mape_pct FROM forecasts;
```

---

## Distance Metrics

### stat_euclidean_dist

Computes the **Euclidean distance** (L2 norm).

$$d = \sqrt{\sum_{i=1}^{n}(a_i - b_i)^2}$$

**Syntax**: `stat_euclidean_dist(column_a, column_b)`

> **Minimum data count**: 1

```sql
-- Euclidean distance between two vectors
SELECT stat_euclidean_dist(feature_a, feature_b) FROM data;
```

---

### stat_manhattan_dist

Computes the **Manhattan distance** (L1 norm).

$$d = \sum_{i=1}^{n}|a_i - b_i|$$

**Syntax**: `stat_manhattan_dist(column_a, column_b)`

> **Minimum data count**: 1

```sql
SELECT stat_manhattan_dist(x, y) FROM coordinates;
```

---

### stat_cosine_sim

Computes the **cosine similarity**.

$$\text{sim} = \frac{\mathbf{a} \cdot \mathbf{b}}{|\mathbf{a}||\mathbf{b}|}$$

**Syntax**: `stat_cosine_sim(column_a, column_b)`

> **Minimum data count**: 1
> **Return value range**: -1.0 to 1.0

```sql
-- Vector similarity
SELECT stat_cosine_sim(user_a_rating, user_b_rating) FROM item_ratings;
```

---

### stat_cosine_dist

Computes the **cosine distance**. `1 - cosine_similarity`.

**Syntax**: `stat_cosine_dist(column_a, column_b)`

> **Minimum data count**: 1
> **Return value range**: 0.0 to 2.0

```sql
SELECT stat_cosine_dist(vec_a, vec_b) FROM embeddings;
```

---

### stat_minkowski_dist

Computes the **Minkowski distance** (Lp norm).

$$d = \left(\sum_{i=1}^{n}|a_i - b_i|^p\right)^{1/p}$$

**Syntax**: `stat_minkowski_dist(column_a, column_b, p)`

| Parameter | Description | Typical values |
|---|---|---|
| `p` | Order parameter | p=1: Manhattan, p=2: Euclidean |

> **Minimum data count**: 1

```sql
-- L3 norm
SELECT stat_minkowski_dist(x, y, 3.0) FROM data;
```

---

### stat_chebyshev_dist

Computes the **Chebyshev distance** (L-infinity norm). The maximum absolute difference among elements.

$$d = \max_i |a_i - b_i|$$

**Syntax**: `stat_chebyshev_dist(column_a, column_b)`

> **Minimum data count**: 1

```sql
SELECT stat_chebyshev_dist(x, y) FROM data;
```

---
