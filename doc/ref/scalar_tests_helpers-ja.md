# スカラー関数 — 検定補助（40関数）

DB非依存のスカラー関数。パラメータのみで計算が完結する。分布関数、特殊関数、比率検定、多重検定補正等。

← [関数リファレンス](../function_reference-ja.md) に戻る

---

## 分布関数（正規・カイ二乗・t・F）

スカラー関数。パラメータのみで計算が完結する。

### 正規分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_normal_pdf` | `stat_normal_pdf(x [,mu, sigma])` | 正規分布の確率密度関数 |
| `stat_normal_cdf` | `stat_normal_cdf(x [,mu, sigma])` | 正規分布の累積分布関数 |
| `stat_normal_quantile` | `stat_normal_quantile(p [,mu, sigma])` | 正規分布の分位点（逆CDF） |
| `stat_normal_rand` | `stat_normal_rand([mu, sigma])` | 正規分布からの乱数生成 |

```sql
-- 標準正規分布
SELECT stat_normal_pdf(0);        -- → 0.3989...
SELECT stat_normal_cdf(1.96);     -- → 0.9750...
SELECT stat_normal_quantile(0.975); -- → 1.9599...

-- パラメータ指定 N(100, 15)
SELECT stat_normal_pdf(115, 100, 15);
SELECT stat_normal_cdf(130, 100, 15);
```

### カイ二乗分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_chisq_pdf` | `stat_chisq_pdf(x, df)` | カイ二乗分布 PDF |
| `stat_chisq_cdf` | `stat_chisq_cdf(x, df)` | カイ二乗分布 CDF |
| `stat_chisq_quantile` | `stat_chisq_quantile(p, df)` | カイ二乗分布分位点 |
| `stat_chisq_rand` | `stat_chisq_rand(df)` | カイ二乗分布乱数 |

```sql
SELECT stat_chisq_cdf(3.84, 1);      -- → 0.9499... (α=0.05 の臨界値)
SELECT stat_chisq_quantile(0.95, 1);  -- → 3.8414...
```

### t 分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_t_pdf` | `stat_t_pdf(x, df)` | t 分布 PDF |
| `stat_t_cdf` | `stat_t_cdf(x, df)` | t 分布 CDF |
| `stat_t_quantile` | `stat_t_quantile(p, df)` | t 分布分位点 |
| `stat_t_rand` | `stat_t_rand(df)` | t 分布乱数 |

```sql
SELECT stat_t_quantile(0.975, 10);  -- → 2.2281... (自由度10の両側5%臨界値)
SELECT stat_t_cdf(2.0, 30);         -- → 0.9726...
```

### F 分布

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_f_pdf` | `stat_f_pdf(x, df1, df2)` | F 分布 PDF |
| `stat_f_cdf` | `stat_f_cdf(x, df1, df2)` | F 分布 CDF |
| `stat_f_quantile` | `stat_f_quantile(p, df1, df2)` | F 分布分位点 |
| `stat_f_rand` | `stat_f_rand(df1, df2)` | F 分布乱数 |

```sql
SELECT stat_f_quantile(0.95, 3, 20);  -- → 3.0984... (ANOVA用臨界値)
SELECT stat_f_cdf(4.0, 5, 10);        -- → 0.9729...
```

> **備考**: 乱数関数（`*_rand`）は非決定的関数として登録されており、呼び出すたびに異なる値を返す。

---

## 特殊関数

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_betainc` | `stat_betainc(a, b, x)` | 正則化不完全ベータ関数 |
| `stat_betaincinv` | `stat_betaincinv(a, b, p)` | 正則化不完全ベータ逆関数 |
| `stat_norm_cdf` | `stat_norm_cdf(x)` | 標準正規分布 CDF（Φ(x)） |
| `stat_norm_quantile` | `stat_norm_quantile(p)` | 標準正規分布逆 CDF（Φ⁻¹(p)） |
| `stat_gammainc_lower` | `stat_gammainc_lower(a, x)` | 下側正則化不完全ガンマ関数 |
| `stat_gammainc_upper` | `stat_gammainc_upper(a, x)` | 上側正則化不完全ガンマ関数 |
| `stat_gammainc_lower_inv` | `stat_gammainc_lower_inv(a, p)` | 下側不完全ガンマ逆関数 |

```sql
SELECT stat_norm_cdf(1.96);        -- → 0.9750...
SELECT stat_norm_quantile(0.975);  -- → 1.9599...
SELECT stat_betainc(2, 5, 0.3);    -- 正則化不完全ベータ関数
```

---

## 比率検定

### stat_z_test_prop

**1標本比率 z 検定**を JSON で返す。

**構文**: `stat_z_test_prop(x, n, p0)`

| パラメータ | 説明 |
|---|---|
| `x` | 成功数 |
| `n` | 試行数 |
| `p0` | 帰無仮説の比率 |

```sql
SELECT stat_z_test_prop(55, 100, 0.5);
-- → {"statistic":1.0,"p_value":0.3173...}
```

---

### stat_z_test_prop2

**2標本比率 z 検定**を JSON で返す。

**構文**: `stat_z_test_prop2(x1, n1, x2, n2)`

```sql
SELECT stat_z_test_prop2(45, 100, 55, 120);
```

---

## 多重検定補正

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_bonferroni` | `stat_bonferroni(p, m)` | Bonferroni 補正（p × m） |
| `stat_bh_correction` | `stat_bh_correction(p, rank, total)` | Benjamini-Hochberg 補正 |
| `stat_holm_correction` | `stat_holm_correction(p, rank, total)` | Holm 補正 |

```sql
-- Bonferroni: p値を検定数で補正
SELECT stat_bonferroni(0.01, 10);  -- → 0.1

-- BH補正: ウィンドウ関数と組み合わせて使用
SELECT p_value,
       stat_bh_correction(p_value,
           ROW_NUMBER() OVER (ORDER BY p_value),
           COUNT(*) OVER ()) AS bh_adjusted
FROM test_results
ORDER BY p_value;
```

---

## カテゴリカル分析（スカラー）

### stat_fisher_exact

**Fisher 正確確率検定**を JSON で返す。2×2 分割表のセル値を直接渡す。

**構文**: `stat_fisher_exact(a, b, c, d)`

```sql
-- 2×2分割表: [[10, 5], [3, 12]]
SELECT stat_fisher_exact(10, 5, 3, 12);
```

---

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_odds_ratio` | `stat_odds_ratio(a, b, c, d)` | オッズ比 |
| `stat_relative_risk` | `stat_relative_risk(a, b, c, d)` | 相対リスク |
| `stat_risk_difference` | `stat_risk_difference(a, b, c, d)` | リスク差 |
| `stat_nnt` | `stat_nnt(a, b, c, d)` | 治療必要数 (NNT) |

```sql
-- 2×2分割表: [[a, b], [c, d]]
SELECT stat_odds_ratio(30, 70, 10, 90);       -- オッズ比
SELECT stat_relative_risk(30, 70, 10, 90);     -- 相対リスク
SELECT stat_risk_difference(30, 70, 10, 90);   -- リスク差
SELECT stat_nnt(30, 70, 10, 90);               -- NNT
```

---

## 比率の信頼区間

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_ci_prop` | `stat_ci_prop(x, n [,confidence])` | 比率の信頼区間（Wald 法） |
| `stat_ci_prop_wilson` | `stat_ci_prop_wilson(x, n [,confidence])` | 比率の信頼区間（Wilson 法） |
| `stat_ci_prop_diff` | `stat_ci_prop_diff(x1, n1, x2, n2 [,confidence])` | 2標本比率差の信頼区間 |

```sql
-- 100人中60人成功、95%信頼区間
SELECT stat_ci_prop(60, 100);
SELECT stat_ci_prop_wilson(60, 100, 0.99);  -- 99%信頼区間

-- 2群の比率差の信頼区間
SELECT stat_ci_prop_diff(45, 100, 55, 120);
```

---

## モデル選択

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_aic` | `stat_aic(log_likelihood, k)` | 赤池情報量規準 |
| `stat_aicc` | `stat_aicc(log_likelihood, n, k)` | 補正 AIC |
| `stat_bic` | `stat_bic(log_likelihood, n, k)` | ベイズ情報量規準 |

| パラメータ | 説明 |
|---|---|
| `log_likelihood` | 対数尤度 |
| `k` | パラメータ数 |
| `n` | サンプルサイズ |

```sql
SELECT stat_aic(-150.0, 3);         -- AIC
SELECT stat_aicc(-150.0, 50, 3);    -- AICc（小標本補正）
SELECT stat_bic(-150.0, 100, 3);    -- BIC
```

---

## データ変換

### stat_boxcox

**Box-Cox 変換**を適用する。

**構文**: `stat_boxcox(x, lambda)`

```sql
SELECT stat_boxcox(10.0, 0.5);   -- λ=0.5
SELECT stat_boxcox(10.0, 0);     -- λ=0 は log(x)
SELECT stat_boxcox(10.0, 1);     -- λ=1 は x-1
```

---
