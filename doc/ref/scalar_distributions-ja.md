# スカラー関数 — 分布・変換（83関数）

DB非依存のスカラー関数。追加の確率分布関数（連続・離散）、組合せ論、特殊関数、効果量変換・解釈、検出力分析、サンプルサイズ計算を提供する。

← [関数リファレンス](../function_reference-ja.md) に戻る

---

## 連続分布関数

スカラー関数。各分布について PDF/CDF/Quantile/Rand の4種を提供する。

### 一様分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_uniform_pdf` | `stat_uniform_pdf(x [,a, b])` | 一様分布 PDF（デフォルト [0,1]） |
| `stat_uniform_cdf` | `stat_uniform_cdf(x [,a, b])` | 一様分布 CDF |
| `stat_uniform_quantile` | `stat_uniform_quantile(p [,a, b])` | 一様分布分位点 |
| `stat_uniform_rand` | `stat_uniform_rand([a, b])` | 一様分布乱数 |

### 指数分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_exponential_pdf` | `stat_exponential_pdf(x [,lambda])` | 指数分布 PDF（デフォルト λ=1） |
| `stat_exponential_cdf` | `stat_exponential_cdf(x [,lambda])` | 指数分布 CDF |
| `stat_exponential_quantile` | `stat_exponential_quantile(p [,lambda])` | 指数分布分位点 |
| `stat_exponential_rand` | `stat_exponential_rand([lambda])` | 指数分布乱数 |

### ガンマ分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_gamma_pdf` | `stat_gamma_pdf(x, shape, scale)` | ガンマ分布 PDF |
| `stat_gamma_cdf` | `stat_gamma_cdf(x, shape, scale)` | ガンマ分布 CDF |
| `stat_gamma_quantile` | `stat_gamma_quantile(p, shape, scale)` | ガンマ分布分位点 |
| `stat_gamma_rand` | `stat_gamma_rand(shape, scale)` | ガンマ分布乱数 |

### ベータ分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_beta_pdf` | `stat_beta_pdf(x, alpha, beta)` | ベータ分布 PDF |
| `stat_beta_cdf` | `stat_beta_cdf(x, alpha, beta)` | ベータ分布 CDF |
| `stat_beta_quantile` | `stat_beta_quantile(p, alpha, beta)` | ベータ分布分位点 |
| `stat_beta_rand` | `stat_beta_rand(alpha, beta)` | ベータ分布乱数 |

### 対数正規分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_lognormal_pdf` | `stat_lognormal_pdf(x [,mu, sigma])` | 対数正規分布 PDF（デフォルト μ=0, σ=1） |
| `stat_lognormal_cdf` | `stat_lognormal_cdf(x [,mu, sigma])` | 対数正規分布 CDF |
| `stat_lognormal_quantile` | `stat_lognormal_quantile(p [,mu, sigma])` | 対数正規分布分位点 |
| `stat_lognormal_rand` | `stat_lognormal_rand([mu, sigma])` | 対数正規分布乱数 |

### ワイブル分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_weibull_pdf` | `stat_weibull_pdf(x, shape, scale)` | ワイブル分布 PDF |
| `stat_weibull_cdf` | `stat_weibull_cdf(x, shape, scale)` | ワイブル分布 CDF |
| `stat_weibull_quantile` | `stat_weibull_quantile(p, shape, scale)` | ワイブル分布分位点 |
| `stat_weibull_rand` | `stat_weibull_rand(shape, scale)` | ワイブル分布乱数 |

```sql
-- 使用例
SELECT stat_exponential_cdf(2.0, 0.5);       -- 指数分布 CDF
SELECT stat_gamma_quantile(0.95, 2.0, 1.0);  -- ガンマ分布の95%点
SELECT stat_beta_pdf(0.3, 2, 5);             -- ベータ分布 PDF
SELECT stat_weibull_cdf(3.0, 2.0, 1.5);     -- ワイブル分布 CDF
```

---

## 離散分布関数

各分布について PMF/CDF/Quantile/Rand の4種を提供する。

### 二項分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_binomial_pmf` | `stat_binomial_pmf(k, n, p)` | 二項分布 PMF |
| `stat_binomial_cdf` | `stat_binomial_cdf(k, n, p)` | 二項分布 CDF |
| `stat_binomial_quantile` | `stat_binomial_quantile(q, n, p)` | 二項分布分位点 |
| `stat_binomial_rand` | `stat_binomial_rand(n, p)` | 二項分布乱数 |

### ポアソン分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_poisson_pmf` | `stat_poisson_pmf(k, lambda)` | ポアソン分布 PMF |
| `stat_poisson_cdf` | `stat_poisson_cdf(k, lambda)` | ポアソン分布 CDF |
| `stat_poisson_quantile` | `stat_poisson_quantile(q, lambda)` | ポアソン分布分位点 |
| `stat_poisson_rand` | `stat_poisson_rand(lambda)` | ポアソン分布乱数 |

### 幾何分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_geometric_pmf` | `stat_geometric_pmf(k, p)` | 幾何分布 PMF |
| `stat_geometric_cdf` | `stat_geometric_cdf(k, p)` | 幾何分布 CDF |
| `stat_geometric_quantile` | `stat_geometric_quantile(q, p)` | 幾何分布分位点 |
| `stat_geometric_rand` | `stat_geometric_rand(p)` | 幾何分布乱数 |

### 負の二項分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_nbinom_pmf` | `stat_nbinom_pmf(k, r, p)` | 負の二項分布 PMF |
| `stat_nbinom_cdf` | `stat_nbinom_cdf(k, r, p)` | 負の二項分布 CDF |
| `stat_nbinom_quantile` | `stat_nbinom_quantile(q, r, p)` | 負の二項分布分位点 |
| `stat_nbinom_rand` | `stat_nbinom_rand(r, p)` | 負の二項分布乱数 |

### 超幾何分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_hypergeom_pmf` | `stat_hypergeom_pmf(k, N, K, n)` | 超幾何分布 PMF |
| `stat_hypergeom_cdf` | `stat_hypergeom_cdf(k, N, K, n)` | 超幾何分布 CDF |
| `stat_hypergeom_quantile` | `stat_hypergeom_quantile(q, N, K, n)` | 超幾何分布分位点 |
| `stat_hypergeom_rand` | `stat_hypergeom_rand(N, K, n)` | 超幾何分布乱数 |

### ベルヌーイ分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_bernoulli_pmf` | `stat_bernoulli_pmf(k, p)` | ベルヌーイ分布 PMF |
| `stat_bernoulli_cdf` | `stat_bernoulli_cdf(k, p)` | ベルヌーイ分布 CDF |
| `stat_bernoulli_quantile` | `stat_bernoulli_quantile(q, p)` | ベルヌーイ分布分位点 |
| `stat_bernoulli_rand` | `stat_bernoulli_rand(p)` | ベルヌーイ分布乱数 |

### 離散一様分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_duniform_pmf` | `stat_duniform_pmf(k, a, b)` | 離散一様分布 PMF |
| `stat_duniform_cdf` | `stat_duniform_cdf(k, a, b)` | 離散一様分布 CDF |
| `stat_duniform_quantile` | `stat_duniform_quantile(q, a, b)` | 離散一様分布分位点 |
| `stat_duniform_rand` | `stat_duniform_rand(a, b)` | 離散一様分布乱数 |

```sql
-- 使用例
SELECT stat_binomial_pmf(3, 10, 0.5);       -- P(X=3) for Bin(10, 0.5)
SELECT stat_poisson_cdf(5, 3.0);             -- P(X≤5) for Poisson(3)
SELECT stat_hypergeom_pmf(4, 50, 10, 8);     -- 超幾何分布
```

---

## 組合せ論

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_binomial_coef` | `stat_binomial_coef(n, k)` | 二項係数 C(n,k)（整数を返す） |
| `stat_log_binomial_coef` | `stat_log_binomial_coef(n, k)` | 対数二項係数 log C(n,k) |
| `stat_log_factorial` | `stat_log_factorial(n)` | 対数階乗 log(n!) |

```sql
SELECT stat_binomial_coef(10, 3);       -- → 120
SELECT stat_log_factorial(100);          -- → 363.739...
```

---

## 特殊関数

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_lgamma` | `stat_lgamma(x)` | 対数ガンマ関数 ln Γ(x) |
| `stat_tgamma` | `stat_tgamma(x)` | ガンマ関数 Γ(x) |
| `stat_beta_func` | `stat_beta_func(a, b)` | ベータ関数 B(a,b) |
| `stat_lbeta` | `stat_lbeta(a, b)` | 対数ベータ関数 ln B(a,b) |
| `stat_erf` | `stat_erf(x)` | 誤差関数 erf(x) |
| `stat_erfc` | `stat_erfc(x)` | 相補誤差関数 erfc(x) |
| `stat_logarithmic_mean` | `stat_logarithmic_mean(a, b)` | 対数平均 |

```sql
SELECT stat_tgamma(5);          -- → 24.0 (= 4!)
SELECT stat_lgamma(10);         -- → 12.8018...
SELECT stat_erf(1.0);           -- → 0.8427...
SELECT stat_logarithmic_mean(2, 8);  -- 対数平均
```

---

## 効果量変換・解釈

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_hedges_j` | `stat_hedges_j(n)` | Hedges 補正係数 J(n) |
| `stat_t_to_r` | `stat_t_to_r(t, df)` | t 値 → 相関係数 r 変換 |
| `stat_d_to_r` | `stat_d_to_r(d)` | Cohen's d → 相関係数 r 変換 |
| `stat_r_to_d` | `stat_r_to_d(r)` | 相関係数 r → Cohen's d 変換 |
| `stat_eta_squared_ef` | `stat_eta_squared_ef(ss_effect, ss_total)` | イータ二乗 η² |
| `stat_partial_eta_sq` | `stat_partial_eta_sq(F, df1, df2)` | 偏イータ二乗 |
| `stat_omega_squared_ef` | `stat_omega_squared_ef(ss_effect, ss_total, ms_error, df_effect)` | オメガ二乗 ω² |
| `stat_cohens_h` | `stat_cohens_h(p1, p2)` | Cohen's h（2比率の効果量） |

### 効果量の解釈

| 関数 | 構文 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_interpret_d` | `stat_interpret_d(d)` | テキスト | Cohen's d の解釈（small/medium/large） |
| `stat_interpret_r` | `stat_interpret_r(r)` | テキスト | 相関係数の解釈 |
| `stat_interpret_eta2` | `stat_interpret_eta2(eta2)` | テキスト | イータ二乗の解釈 |

```sql
SELECT stat_d_to_r(0.5);           -- Cohen's d → r 変換
SELECT stat_interpret_d(0.8);       -- → 'large'
SELECT stat_interpret_r(0.3);       -- → 'medium'
SELECT stat_cohens_h(0.6, 0.4);    -- 2比率の効果量
```

---

## 検出力分析

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_power_t1` | `stat_power_t1(d, n, alpha)` | 1標本 t 検定の検出力 |
| `stat_n_t1` | `stat_n_t1(d, power, alpha)` | 1標本 t 検定の必要サンプルサイズ |
| `stat_power_t2` | `stat_power_t2(d, n1, n2, alpha)` | 2標本 t 検定の検出力 |
| `stat_n_t2` | `stat_n_t2(d, power, alpha)` | 2標本 t 検定の必要サンプルサイズ |
| `stat_power_prop` | `stat_power_prop(p1, p2, n, alpha)` | 比率検定の検出力 |
| `stat_n_prop` | `stat_n_prop(p1, p2, power, alpha)` | 比率検定の必要サンプルサイズ |

```sql
-- 効果量 d=0.5, n=30, α=0.05 での検出力
SELECT stat_power_t1(0.5, 30, 0.05);

-- 効果量 d=0.5, 検出力 80%, α=0.05 の必要 n
SELECT stat_n_t1(0.5, 0.8, 0.05);

-- 2群の比率検定の必要サンプルサイズ
SELECT stat_n_prop(0.5, 0.3, 0.8, 0.05);
```

---

## サンプルサイズ・誤差マージン

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_moe_prop` | `stat_moe_prop(x, n [,confidence])` | 比率の誤差マージン |
| `stat_moe_prop_worst` | `stat_moe_prop_worst(n [,confidence])` | 最悪ケースの誤差マージン（p=0.5） |
| `stat_n_moe_prop` | `stat_n_moe_prop(moe [,confidence [,p]])` | 比率推定の必要サンプルサイズ |
| `stat_n_moe_mean` | `stat_n_moe_mean(moe, sigma [,confidence])` | 平均推定の必要サンプルサイズ |

```sql
-- 成功数 200, n=400 の誤差マージン（95%信頼区間）
SELECT stat_moe_prop(200, 400);

-- 誤差マージン 3% 以内に収めるためのサンプルサイズ
SELECT stat_n_moe_prop(0.03);

-- 平均推定: 誤差マージン 2.0 以内, σ=15
SELECT stat_n_moe_mean(2.0, 15.0);
```

---
