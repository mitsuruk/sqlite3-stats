# Scalar Functions — Distributions / Transforms (83 Functions)

Database-independent scalar functions. Provides additional probability distributions (continuous and discrete), combinatorics, special functions, effect size conversions and interpretation, power analysis, and sample size calculations.

← [Function Reference](../function_reference.md)

---

## Continuous Distribution Functions

Scalar functions. For each distribution, four variants are provided: PDF / CDF / Quantile / Rand.

### Uniform Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_uniform_pdf` | `stat_uniform_pdf(x [,a, b])` | Uniform distribution PDF (default [0,1]) |
| `stat_uniform_cdf` | `stat_uniform_cdf(x [,a, b])` | Uniform distribution CDF |
| `stat_uniform_quantile` | `stat_uniform_quantile(p [,a, b])` | Uniform distribution quantile |
| `stat_uniform_rand` | `stat_uniform_rand([a, b])` | Uniform distribution random number |

### Exponential Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_exponential_pdf` | `stat_exponential_pdf(x [,lambda])` | Exponential distribution PDF (default lambda=1) |
| `stat_exponential_cdf` | `stat_exponential_cdf(x [,lambda])` | Exponential distribution CDF |
| `stat_exponential_quantile` | `stat_exponential_quantile(p [,lambda])` | Exponential distribution quantile |
| `stat_exponential_rand` | `stat_exponential_rand([lambda])` | Exponential distribution random number |

### Gamma Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_gamma_pdf` | `stat_gamma_pdf(x, shape, scale)` | Gamma distribution PDF |
| `stat_gamma_cdf` | `stat_gamma_cdf(x, shape, scale)` | Gamma distribution CDF |
| `stat_gamma_quantile` | `stat_gamma_quantile(p, shape, scale)` | Gamma distribution quantile |
| `stat_gamma_rand` | `stat_gamma_rand(shape, scale)` | Gamma distribution random number |

### Beta Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_beta_pdf` | `stat_beta_pdf(x, alpha, beta)` | Beta distribution PDF |
| `stat_beta_cdf` | `stat_beta_cdf(x, alpha, beta)` | Beta distribution CDF |
| `stat_beta_quantile` | `stat_beta_quantile(p, alpha, beta)` | Beta distribution quantile |
| `stat_beta_rand` | `stat_beta_rand(alpha, beta)` | Beta distribution random number |

### Log-Normal Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_lognormal_pdf` | `stat_lognormal_pdf(x [,mu, sigma])` | Log-normal distribution PDF (default mu=0, sigma=1) |
| `stat_lognormal_cdf` | `stat_lognormal_cdf(x [,mu, sigma])` | Log-normal distribution CDF |
| `stat_lognormal_quantile` | `stat_lognormal_quantile(p [,mu, sigma])` | Log-normal distribution quantile |
| `stat_lognormal_rand` | `stat_lognormal_rand([mu, sigma])` | Log-normal distribution random number |

### Weibull Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_weibull_pdf` | `stat_weibull_pdf(x, shape, scale)` | Weibull distribution PDF |
| `stat_weibull_cdf` | `stat_weibull_cdf(x, shape, scale)` | Weibull distribution CDF |
| `stat_weibull_quantile` | `stat_weibull_quantile(p, shape, scale)` | Weibull distribution quantile |
| `stat_weibull_rand` | `stat_weibull_rand(shape, scale)` | Weibull distribution random number |

```sql
-- Usage examples
SELECT stat_exponential_cdf(2.0, 0.5);       -- Exponential distribution CDF
SELECT stat_gamma_quantile(0.95, 2.0, 1.0);  -- Gamma distribution 95th percentile
SELECT stat_beta_pdf(0.3, 2, 5);             -- Beta distribution PDF
SELECT stat_weibull_cdf(3.0, 2.0, 1.5);     -- Weibull distribution CDF
```

---

## Discrete Distribution Functions

For each distribution, four variants are provided: PMF / CDF / Quantile / Rand.

### Binomial Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_binomial_pmf` | `stat_binomial_pmf(k, n, p)` | Binomial distribution PMF |
| `stat_binomial_cdf` | `stat_binomial_cdf(k, n, p)` | Binomial distribution CDF |
| `stat_binomial_quantile` | `stat_binomial_quantile(q, n, p)` | Binomial distribution quantile |
| `stat_binomial_rand` | `stat_binomial_rand(n, p)` | Binomial distribution random number |

### Poisson Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_poisson_pmf` | `stat_poisson_pmf(k, lambda)` | Poisson distribution PMF |
| `stat_poisson_cdf` | `stat_poisson_cdf(k, lambda)` | Poisson distribution CDF |
| `stat_poisson_quantile` | `stat_poisson_quantile(q, lambda)` | Poisson distribution quantile |
| `stat_poisson_rand` | `stat_poisson_rand(lambda)` | Poisson distribution random number |

### Geometric Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_geometric_pmf` | `stat_geometric_pmf(k, p)` | Geometric distribution PMF |
| `stat_geometric_cdf` | `stat_geometric_cdf(k, p)` | Geometric distribution CDF |
| `stat_geometric_quantile` | `stat_geometric_quantile(q, p)` | Geometric distribution quantile |
| `stat_geometric_rand` | `stat_geometric_rand(p)` | Geometric distribution random number |

### Negative Binomial Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_nbinom_pmf` | `stat_nbinom_pmf(k, r, p)` | Negative binomial distribution PMF |
| `stat_nbinom_cdf` | `stat_nbinom_cdf(k, r, p)` | Negative binomial distribution CDF |
| `stat_nbinom_quantile` | `stat_nbinom_quantile(q, r, p)` | Negative binomial distribution quantile |
| `stat_nbinom_rand` | `stat_nbinom_rand(r, p)` | Negative binomial distribution random number |

### Hypergeometric Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_hypergeom_pmf` | `stat_hypergeom_pmf(k, N, K, n)` | Hypergeometric distribution PMF |
| `stat_hypergeom_cdf` | `stat_hypergeom_cdf(k, N, K, n)` | Hypergeometric distribution CDF |
| `stat_hypergeom_quantile` | `stat_hypergeom_quantile(q, N, K, n)` | Hypergeometric distribution quantile |
| `stat_hypergeom_rand` | `stat_hypergeom_rand(N, K, n)` | Hypergeometric distribution random number |

### Bernoulli Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_bernoulli_pmf` | `stat_bernoulli_pmf(k, p)` | Bernoulli distribution PMF |
| `stat_bernoulli_cdf` | `stat_bernoulli_cdf(k, p)` | Bernoulli distribution CDF |
| `stat_bernoulli_quantile` | `stat_bernoulli_quantile(q, p)` | Bernoulli distribution quantile |
| `stat_bernoulli_rand` | `stat_bernoulli_rand(p)` | Bernoulli distribution random number |

### Discrete Uniform Distribution

| Function | Syntax | Description |
|---|---|---|
| `stat_duniform_pmf` | `stat_duniform_pmf(k, a, b)` | Discrete uniform distribution PMF |
| `stat_duniform_cdf` | `stat_duniform_cdf(k, a, b)` | Discrete uniform distribution CDF |
| `stat_duniform_quantile` | `stat_duniform_quantile(q, a, b)` | Discrete uniform distribution quantile |
| `stat_duniform_rand` | `stat_duniform_rand(a, b)` | Discrete uniform distribution random number |

```sql
-- Usage examples
SELECT stat_binomial_pmf(3, 10, 0.5);       -- P(X=3) for Bin(10, 0.5)
SELECT stat_poisson_cdf(5, 3.0);             -- P(X<=5) for Poisson(3)
SELECT stat_hypergeom_pmf(4, 50, 10, 8);     -- Hypergeometric distribution
```

---

## Combinatorics

| Function | Syntax | Description |
|---|---|---|
| `stat_binomial_coef` | `stat_binomial_coef(n, k)` | Binomial coefficient C(n,k) (returns integer) |
| `stat_log_binomial_coef` | `stat_log_binomial_coef(n, k)` | Log binomial coefficient log C(n,k) |
| `stat_log_factorial` | `stat_log_factorial(n)` | Log factorial log(n!) |

```sql
SELECT stat_binomial_coef(10, 3);       -- -> 120
SELECT stat_log_factorial(100);          -- -> 363.739...
```

---

## Special Functions

| Function | Syntax | Description |
|---|---|---|
| `stat_lgamma` | `stat_lgamma(x)` | Log-gamma function ln Gamma(x) |
| `stat_tgamma` | `stat_tgamma(x)` | Gamma function Gamma(x) |
| `stat_beta_func` | `stat_beta_func(a, b)` | Beta function B(a,b) |
| `stat_lbeta` | `stat_lbeta(a, b)` | Log-beta function ln B(a,b) |
| `stat_erf` | `stat_erf(x)` | Error function erf(x) |
| `stat_erfc` | `stat_erfc(x)` | Complementary error function erfc(x) |
| `stat_logarithmic_mean` | `stat_logarithmic_mean(a, b)` | Logarithmic mean |

```sql
SELECT stat_tgamma(5);          -- -> 24.0 (= 4!)
SELECT stat_lgamma(10);         -- -> 12.8018...
SELECT stat_erf(1.0);           -- -> 0.8427...
SELECT stat_logarithmic_mean(2, 8);  -- Logarithmic mean
```

---

## Effect Size Conversions

| Function | Syntax | Description |
|---|---|---|
| `stat_hedges_j` | `stat_hedges_j(n)` | Hedges correction factor J(n) |
| `stat_t_to_r` | `stat_t_to_r(t, df)` | t-value to correlation coefficient r conversion |
| `stat_d_to_r` | `stat_d_to_r(d)` | Cohen's d to correlation coefficient r conversion |
| `stat_r_to_d` | `stat_r_to_d(r)` | Correlation coefficient r to Cohen's d conversion |
| `stat_eta_squared_ef` | `stat_eta_squared_ef(ss_effect, ss_total)` | Eta-squared (effect size) |
| `stat_partial_eta_sq` | `stat_partial_eta_sq(F, df1, df2)` | Partial eta-squared |
| `stat_omega_squared_ef` | `stat_omega_squared_ef(ss_effect, ss_total, ms_error, df_effect)` | Omega-squared (effect size) |
| `stat_cohens_h` | `stat_cohens_h(p1, p2)` | Cohen's h (proportion difference effect size) |

### Effect Size Interpretation

| Function | Syntax | Return | Description |
|---|---|---|---|
| `stat_interpret_d` | `stat_interpret_d(d)` | TEXT | Cohen's d interpretation (small/medium/large) |
| `stat_interpret_r` | `stat_interpret_r(r)` | TEXT | Correlation coefficient interpretation |
| `stat_interpret_eta2` | `stat_interpret_eta2(eta2)` | TEXT | Eta-squared interpretation |

```sql
SELECT stat_d_to_r(0.5);           -- Cohen's d to r conversion
SELECT stat_interpret_d(0.8);       -- -> 'large'
SELECT stat_interpret_r(0.3);       -- -> 'medium'
SELECT stat_cohens_h(0.6, 0.4);    -- Proportion difference effect size
```

---

## Power Analysis

| Function | Syntax | Description |
|---|---|---|
| `stat_power_t1` | `stat_power_t1(d, n, alpha)` | One-sample t-test power |
| `stat_n_t1` | `stat_n_t1(d, power, alpha)` | One-sample t-test required sample size |
| `stat_power_t2` | `stat_power_t2(d, n1, n2, alpha)` | Two-sample t-test power |
| `stat_n_t2` | `stat_n_t2(d, power, alpha)` | Two-sample t-test required sample size |
| `stat_power_prop` | `stat_power_prop(p1, p2, n, alpha)` | Proportion test power |
| `stat_n_prop` | `stat_n_prop(p1, p2, power, alpha)` | Proportion test required sample size |

```sql
-- Power for effect size d=0.5, n=30, alpha=0.05
SELECT stat_power_t1(0.5, 30, 0.05);

-- Required n for effect size d=0.5, power 80%, alpha=0.05
SELECT stat_n_t1(0.5, 0.8, 0.05);

-- Required sample size for a two-group proportion test
SELECT stat_n_prop(0.5, 0.3, 0.8, 0.05);
```

---

## Sample Size / Margin of Error

| Function | Syntax | Description |
|---|---|---|
| `stat_moe_prop` | `stat_moe_prop(x, n [,confidence])` | Margin of error for proportion |
| `stat_moe_prop_worst` | `stat_moe_prop_worst(n [,confidence])` | Worst-case margin of error (p=0.5) |
| `stat_n_moe_prop` | `stat_n_moe_prop(moe [,confidence [,p]])` | Required sample size for proportion estimation |
| `stat_n_moe_mean` | `stat_n_moe_mean(moe, sigma [,confidence])` | Required sample size for mean estimation |

```sql
-- Margin of error for 200 successes out of n=400 (95% confidence interval)
SELECT stat_moe_prop(200, 400);

-- Sample size needed to achieve a margin of error within 3%
SELECT stat_n_moe_prop(0.03);

-- Mean estimation: margin of error within 2.0, sigma=15
SELECT stat_n_moe_mean(2.0, 15.0);
```

---
