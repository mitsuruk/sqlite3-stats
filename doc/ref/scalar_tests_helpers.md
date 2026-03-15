# Scalar Functions — Tests / Helpers (40 Functions)

Database-independent scalar functions. Computations are based solely on parameters. Includes distribution functions, special functions, proportion tests, multiple testing corrections, and more.

← [Function Reference](../function_reference.md)

---

## Distribution Functions (Normal, Chi-Square, t, F)

Scalar functions. Computations are based solely on parameters.

### Normal Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_normal_pdf` | `stat_normal_pdf(x [,mu, sigma])` | Normal distribution probability density function |
| `stat_normal_cdf` | `stat_normal_cdf(x [,mu, sigma])` | Normal distribution cumulative distribution function |
| `stat_normal_quantile` | `stat_normal_quantile(p [,mu, sigma])` | Normal distribution quantile (inverse CDF) |
| `stat_normal_rand` | `stat_normal_rand([mu, sigma])` | Normal distribution random number generation |

```sql
-- Standard normal distribution
SELECT stat_normal_pdf(0);        -- → 0.3989...
SELECT stat_normal_cdf(1.96);     -- → 0.9750...
SELECT stat_normal_quantile(0.975); -- → 1.9599...

-- With parameters N(100, 15)
SELECT stat_normal_pdf(115, 100, 15);
SELECT stat_normal_cdf(130, 100, 15);
```

### Chi-Square Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_chisq_pdf` | `stat_chisq_pdf(x, df)` | Chi-square distribution PDF |
| `stat_chisq_cdf` | `stat_chisq_cdf(x, df)` | Chi-square distribution CDF |
| `stat_chisq_quantile` | `stat_chisq_quantile(p, df)` | Chi-square distribution quantile |
| `stat_chisq_rand` | `stat_chisq_rand(df)` | Chi-square distribution random number generation |

```sql
SELECT stat_chisq_cdf(3.84, 1);      -- → 0.9499... (critical value for α=0.05)
SELECT stat_chisq_quantile(0.95, 1);  -- → 3.8414...
```

### t Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_t_pdf` | `stat_t_pdf(x, df)` | t distribution PDF |
| `stat_t_cdf` | `stat_t_cdf(x, df)` | t distribution CDF |
| `stat_t_quantile` | `stat_t_quantile(p, df)` | t distribution quantile |
| `stat_t_rand` | `stat_t_rand(df)` | t distribution random number generation |

```sql
SELECT stat_t_quantile(0.975, 10);  -- → 2.2281... (two-tailed 5% critical value, df=10)
SELECT stat_t_cdf(2.0, 30);         -- → 0.9726...
```

### F Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_f_pdf` | `stat_f_pdf(x, df1, df2)` | F distribution PDF |
| `stat_f_cdf` | `stat_f_cdf(x, df1, df2)` | F distribution CDF |
| `stat_f_quantile` | `stat_f_quantile(p, df1, df2)` | F distribution quantile |
| `stat_f_rand` | `stat_f_rand(df1, df2)` | F distribution random number generation |

```sql
SELECT stat_f_quantile(0.95, 3, 20);  -- → 3.0984... (critical value for ANOVA)
SELECT stat_f_cdf(4.0, 5, 10);        -- → 0.9729...
```

> **Note**: Random functions (`*_rand`) are registered as non-deterministic functions and return a different value on each call.

---

## Special Functions

| Function | Syntax | Description |
|---|---|---|
| `stat_betainc` | `stat_betainc(a, b, x)` | Regularized incomplete beta function |
| `stat_betaincinv` | `stat_betaincinv(a, b, p)` | Inverse regularized incomplete beta function |
| `stat_norm_cdf` | `stat_norm_cdf(x)` | Standard normal CDF (Φ(x)) |
| `stat_norm_quantile` | `stat_norm_quantile(p)` | Standard normal inverse CDF (Φ⁻¹(p)) |
| `stat_gammainc_lower` | `stat_gammainc_lower(a, x)` | Lower regularized incomplete gamma function |
| `stat_gammainc_upper` | `stat_gammainc_upper(a, x)` | Upper regularized incomplete gamma function |
| `stat_gammainc_lower_inv` | `stat_gammainc_lower_inv(a, p)` | Inverse lower incomplete gamma function |

```sql
SELECT stat_norm_cdf(1.96);        -- → 0.9750...
SELECT stat_norm_quantile(0.975);  -- → 1.9599...
SELECT stat_betainc(2, 5, 0.3);    -- Regularized incomplete beta function
```

---

## Proportion Tests

### stat_z_test_prop

Returns a **one-sample proportion z-test** result as JSON.

**Syntax**: `stat_z_test_prop(x, n, p0)`

| Parameter | Description |
|---|---|
| `x` | Number of successes |
| `n` | Number of trials |
| `p0` | Null hypothesis proportion |

```sql
SELECT stat_z_test_prop(55, 100, 0.5);
-- → {"statistic":1.0,"p_value":0.3173...}
```

---

### stat_z_test_prop2

Returns a **two-sample proportion z-test** result as JSON.

**Syntax**: `stat_z_test_prop2(x1, n1, x2, n2)`

```sql
SELECT stat_z_test_prop2(45, 100, 55, 120);
```

---

## Multiple Testing Corrections

| Function | Syntax | Description |
|---|---|---|
| `stat_bonferroni` | `stat_bonferroni(p, m)` | Bonferroni correction (p × m) |
| `stat_bh_correction` | `stat_bh_correction(p, rank, total)` | Benjamini-Hochberg correction |
| `stat_holm_correction` | `stat_holm_correction(p, rank, total)` | Holm correction |

```sql
-- Bonferroni: adjust p-value by the number of tests
SELECT stat_bonferroni(0.01, 10);  -- → 0.1

-- BH correction: used in combination with window functions
SELECT p_value,
       stat_bh_correction(p_value,
           ROW_NUMBER() OVER (ORDER BY p_value),
           COUNT(*) OVER ()) AS bh_adjusted
FROM test_results
ORDER BY p_value;
```

---

## Categorical Analysis (Scalar)

### stat_fisher_exact

Returns a **Fisher's exact test** result as JSON. Pass the cell values of a 2×2 contingency table directly.

**Syntax**: `stat_fisher_exact(a, b, c, d)`

```sql
-- 2×2 contingency table: [[10, 5], [3, 12]]
SELECT stat_fisher_exact(10, 5, 3, 12);
```

---

| Function | Syntax | Description |
|---|---|---|
| `stat_odds_ratio` | `stat_odds_ratio(a, b, c, d)` | Odds ratio |
| `stat_relative_risk` | `stat_relative_risk(a, b, c, d)` | Relative risk |
| `stat_risk_difference` | `stat_risk_difference(a, b, c, d)` | Risk difference |
| `stat_nnt` | `stat_nnt(a, b, c, d)` | Number needed to treat (NNT) |

```sql
-- 2×2 contingency table: [[a, b], [c, d]]
SELECT stat_odds_ratio(30, 70, 10, 90);       -- Odds ratio
SELECT stat_relative_risk(30, 70, 10, 90);     -- Relative risk
SELECT stat_risk_difference(30, 70, 10, 90);   -- Risk difference
SELECT stat_nnt(30, 70, 10, 90);               -- NNT
```

---

## Proportion Confidence Intervals

| Function | Syntax | Description |
|---|---|---|
| `stat_ci_prop` | `stat_ci_prop(x, n [,confidence])` | Proportion CI (Wald method) |
| `stat_ci_prop_wilson` | `stat_ci_prop_wilson(x, n [,confidence])` | Proportion CI (Wilson method) |
| `stat_ci_prop_diff` | `stat_ci_prop_diff(x1, n1, x2, n2 [,confidence])` | CI for proportion difference |

```sql
-- 60 successes out of 100, 95% confidence interval
SELECT stat_ci_prop(60, 100);
SELECT stat_ci_prop_wilson(60, 100, 0.99);  -- 99% confidence interval

-- Confidence interval for the difference between two proportions
SELECT stat_ci_prop_diff(45, 100, 55, 120);
```

---

## Model Selection

| Function | Syntax | Description |
|---|---|---|
| `stat_aic` | `stat_aic(log_likelihood, k)` | Akaike information criterion |
| `stat_aicc` | `stat_aicc(log_likelihood, n, k)` | Corrected AIC |
| `stat_bic` | `stat_bic(log_likelihood, n, k)` | Bayesian information criterion |

| Parameter | Description |
|---|---|
| `log_likelihood` | Log-likelihood |
| `k` | Number of parameters |
| `n` | Sample size |

```sql
SELECT stat_aic(-150.0, 3);         -- AIC
SELECT stat_aicc(-150.0, 50, 3);    -- AICc (small-sample correction)
SELECT stat_bic(-150.0, 100, 3);    -- BIC
```

---

## Data Transformation

### stat_boxcox

Applies a **Box-Cox transformation**.

**Syntax**: `stat_boxcox(x, lambda)`

```sql
SELECT stat_boxcox(10.0, 0.5);   -- λ=0.5
SELECT stat_boxcox(10.0, 0);     -- λ=0 is log(x)
SELECT stat_boxcox(10.0, 1);     -- λ=1 is x-1
```

---
