# 2カラム集約関数（27関数）

2カラムの集約関数。`SELECT stat_xxx(column_x, column_y) FROM table` の形式で使用する。`GROUP BY`、`HAVING`、サブクエリなど SQLite3 の標準的な集約関数が使えるすべての文脈で利用可能。

← [関数リファレンス](../function_reference-ja.md) に戻る

---

## 相関・共分散

### stat_population_covariance

**母共分散**を計算する。

$$\text{Cov}_{pop}(X,Y) = \frac{1}{n}\sum_{i=1}^{n}(x_i - \bar{x})(y_i - \bar{y})$$

**構文**: `stat_population_covariance(column_x, column_y)`

> **最小データ数**: 1

```sql
-- 2つの変数の母共分散
SELECT stat_population_covariance(height, weight) FROM students;

-- グループごとの共分散
SELECT department,
       stat_population_covariance(experience, salary) AS cov
FROM employees
GROUP BY department;
```

---

### stat_covariance

**標本共分散**（不偏共分散）を計算する。

$$\text{Cov}(X,Y) = \frac{1}{n-1}\sum_{i=1}^{n}(x_i - \bar{x})(y_i - \bar{y})$$

**構文**: `stat_covariance(column_x, column_y)`

> **最小データ数**: 2

```sql
SELECT stat_covariance(x, y) FROM measurements;
```

---

### stat_pearson_r

**ピアソン積率相関係数**を計算する。

$$r = \frac{\text{Cov}(X,Y)}{s_X \cdot s_Y}$$

**構文**: `stat_pearson_r(column_x, column_y)`

> **最小データ数**: 2
> **戻り値の範囲**: -1.0 〜 1.0

```sql
-- 身長と体重の相関
SELECT stat_pearson_r(height, weight) FROM students;

-- 相関行列的な分析
SELECT 'height-weight' AS pair, stat_pearson_r(height, weight) AS r FROM data
UNION ALL
SELECT 'height-age', stat_pearson_r(height, age) FROM data
UNION ALL
SELECT 'weight-age', stat_pearson_r(weight, age) FROM data;

-- グループごとの相関
SELECT region,
       stat_pearson_r(advertising, sales) AS correlation
FROM marketing
GROUP BY region;
```

---

### stat_spearman_r

**スピアマン順位相関係数**を計算する。データを順位に変換してからピアソン相関を計算する。外れ値や非線形な単調関係に対してロバスト。

**構文**: `stat_spearman_r(column_x, column_y)`

> **最小データ数**: 2
> **戻り値の範囲**: -1.0 〜 1.0

```sql
-- 順位相関（外れ値に強い）
SELECT stat_spearman_r(rank_math, rank_english) FROM students;

-- ピアソンとスピアマンを比較（非線形性の検出）
SELECT stat_pearson_r(x, y) AS pearson,
       stat_spearman_r(x, y) AS spearman
FROM data;
```

---

### stat_kendall_tau

**ケンドール順位相関係数**（タウb）を計算する。同順位（タイ）を考慮した順位相関。

**構文**: `stat_kendall_tau(column_x, column_y)`

> **最小データ数**: 2
> **戻り値の範囲**: -1.0 〜 1.0

```sql
-- 順位相関（タイ処理あり）
SELECT stat_kendall_tau(preference_a, preference_b) FROM survey;

-- 3種類の相関係数を比較
SELECT stat_pearson_r(x, y) AS pearson,
       stat_spearman_r(x, y) AS spearman,
       stat_kendall_tau(x, y) AS kendall
FROM data;
```

---

### stat_weighted_covariance

**重み付き共分散**を計算する。重みが等しい場合は標本共分散と一致する。

**構文**: `stat_weighted_covariance(column_values, column_weights)`

> **最小データ数**: 2

```sql
-- 信頼度で重み付けした共分散
SELECT stat_weighted_covariance(measurement, confidence) FROM results;
```

---

## 重み付き統計量

### stat_weighted_mean

**重み付き平均**を計算する。

$$\bar{x}_w = \frac{\sum w_i x_i}{\sum w_i}$$

**構文**: `stat_weighted_mean(column_values, column_weights)`

> **最小データ数**: 1

```sql
-- 信頼度で重み付けした平均
SELECT stat_weighted_mean(score, confidence) FROM evaluations;

-- 人口重み付き平均所得
SELECT stat_weighted_mean(income, population) FROM regions;

-- グループ別の重み付き平均
SELECT category,
       stat_weighted_mean(price, quantity) AS weighted_avg_price
FROM products
GROUP BY category;
```

---

### stat_weighted_harmonic_mean

**重み付き調和平均**を計算する。速度や比率の加重平均に適切。

**構文**: `stat_weighted_harmonic_mean(column_values, column_weights)`

> **最小データ数**: 1（全値が正であること）

```sql
-- 距離で重み付けした速度の調和平均
SELECT stat_weighted_harmonic_mean(speed, distance) FROM trips;
```

---

### stat_weighted_variance

**重み付き分散**を計算する。

**構文**: `stat_weighted_variance(column_values, column_weights)`

> **最小データ数**: 2

```sql
SELECT stat_weighted_variance(return_rate, market_cap) FROM stocks;
```

---

### stat_weighted_stddev

**重み付き標準偏差**を計算する（重み付き分散の平方根）。

**構文**: `stat_weighted_stddev(column_values, column_weights)`

> **最小データ数**: 2

```sql
SELECT stat_weighted_stddev(score, sample_size) FROM studies;
```

---

### stat_weighted_median

**重み付き中央値**を計算する。

**構文**: `stat_weighted_median(column_values, column_weights)`

> **最小データ数**: 1

```sql
-- 人口重み付き中央値所得
SELECT stat_weighted_median(median_income, population) FROM cities;
```

---

### stat_weighted_percentile

**重み付きパーセンタイル**を計算する。

**構文**: `stat_weighted_percentile(column_values, column_weights, p)`

| パラメータ | 説明 | 範囲 |
|---|---|---|
| `p` | パーセンタイル位置 | 0.0〜1.0 |

> **最小データ数**: 1

```sql
-- 重み付き 25パーセンタイル
SELECT stat_weighted_percentile(income, population, 0.25) FROM regions;

-- 重み付き中央値（= weighted_median と同等）
SELECT stat_weighted_percentile(income, population, 0.5) FROM regions;
```

---

## 線形回帰

### stat_simple_regression

**単回帰分析**を実行する。最小二乗法による回帰直線の各種統計量を JSON で返す。

$$y = a + bx$$

**構文**: `stat_simple_regression(column_x, column_y)`

**戻り値**: JSON

```json
{
  "intercept": 切片 a,
  "slope": 傾き b,
  "r_squared": 決定係数 R²,
  "adj_r_squared": 自由度調整済み R²,
  "intercept_se": 切片の標準誤差,
  "slope_se": 傾きの標準誤差,
  "intercept_t": 切片の t 値,
  "slope_t": 傾きの t 値,
  "intercept_p": 切片の p 値,
  "slope_p": 傾きの p 値,
  "f_statistic": F 統計量,
  "f_p_value": F 検定の p 値,
  "residual_se": 残差標準誤差
}
```

> **最小データ数**: 3

```sql
-- 単回帰分析
SELECT stat_simple_regression(study_hours, score) FROM students;

-- 傾きと p 値を抽出
SELECT json_extract(stat_simple_regression(x, y), '$.slope') AS slope,
       json_extract(stat_simple_regression(x, y), '$.slope_p') AS p_value
FROM data;

-- グループごとの回帰
SELECT department,
       stat_simple_regression(experience, salary) AS regression
FROM employees
GROUP BY department;
```

---

### stat_r_squared

**決定係数** (R²) を計算する。実測値と予測値の適合度を評価する。

$$R^2 = 1 - \frac{SS_{res}}{SS_{tot}}$$

**構文**: `stat_r_squared(column_actual, column_predicted)`

> **最小データ数**: 2
> **戻り値の範囲**: 通常 0.0〜1.0（負になる場合もある）

```sql
-- モデルの適合度
SELECT stat_r_squared(actual_sales, predicted_sales) FROM forecasts;
```

---

### stat_adjusted_r_squared

**自由度調整済み決定係数** を計算する。説明変数の数（1と仮定）で補正した R²。

**構文**: `stat_adjusted_r_squared(column_actual, column_predicted)`

> **最小データ数**: 3

```sql
-- 調整済み R² と R² を比較
SELECT stat_r_squared(actual, predicted) AS r2,
       stat_adjusted_r_squared(actual, predicted) AS adj_r2
FROM model_results;
```

---

## 対応のある検定・2標本検定

### stat_t_test_paired

**対応のある t 検定**を実行する。差 d = x - y に対して母平均が 0 かを検定する。

$$t = \frac{\bar{d}}{s_d / \sqrt{n}}, \quad df = n - 1$$

**構文**: `stat_t_test_paired(column_x, column_y)`

**戻り値**: JSON `{"statistic": t値, "p_value": ..., "df": ...}`

> **最小データ数**: 2

```sql
-- 薬の効果（投薬前 vs 投薬後）
SELECT stat_t_test_paired(before_treatment, after_treatment) FROM patients;

-- p 値のみ抽出
SELECT json_extract(
    stat_t_test_paired(pre_score, post_score), '$.p_value'
) AS p_value
FROM training_data;
```

---

### stat_chisq_gof

**カイ二乗適合度検定**を実行する。観測値が期待値に適合するかを検定する。

$$\chi^2 = \sum_{i=1}^{k}\frac{(O_i - E_i)^2}{E_i}, \quad df = k - 1$$

**構文**: `stat_chisq_gof(column_observed, column_expected)`

**戻り値**: JSON `{"statistic": χ²値, "p_value": ..., "df": ...}`

> **最小データ数**: 2

```sql
-- 期待分布との適合検定
SELECT stat_chisq_gof(observed_count, expected_count) FROM frequency_data;
```

---

## 誤差指標

### stat_mae

**平均絶対誤差** (MAE) を計算する。

$$\text{MAE} = \frac{1}{n}\sum_{i=1}^{n}|y_i - \hat{y}_i|$$

**構文**: `stat_mae(column_actual, column_predicted)`

> **最小データ数**: 1

```sql
-- 予測誤差の評価
SELECT stat_mae(actual, predicted) AS mae FROM forecasts;

-- 複数モデルの比較
SELECT 'model_A' AS model, stat_mae(actual, pred_a) AS mae FROM results
UNION ALL
SELECT 'model_B', stat_mae(actual, pred_b) FROM results;
```

---

### stat_mse

**平均二乗誤差** (MSE) を計算する。

$$\text{MSE} = \frac{1}{n}\sum_{i=1}^{n}(y_i - \hat{y}_i)^2$$

**構文**: `stat_mse(column_actual, column_predicted)`

> **最小データ数**: 1

```sql
SELECT stat_mse(actual, predicted) AS mse FROM forecasts;
```

---

### stat_rmse

**二乗平均平方根誤差** (RMSE) を計算する。MSE の平方根。

$$\text{RMSE} = \sqrt{\text{MSE}}$$

**構文**: `stat_rmse(column_actual, column_predicted)`

> **最小データ数**: 1

```sql
SELECT stat_rmse(actual, predicted) AS rmse FROM forecasts;
```

---

### stat_mape

**平均絶対パーセント誤差** (MAPE) を計算する。パーセンテージ（%）で返す。

$$\text{MAPE} = \frac{100}{n}\sum_{i=1}^{n}\left|\frac{y_i - \hat{y}_i}{y_i}\right|$$

**構文**: `stat_mape(column_actual, column_predicted)`

> **最小データ数**: 1（actual が 0 の行は除外）

```sql
SELECT stat_mape(actual_sales, predicted_sales) AS mape_pct FROM forecasts;
```

---

## 距離指標

### stat_euclidean_dist

**ユークリッド距離** (L2 ノルム) を計算する。

$$d = \sqrt{\sum_{i=1}^{n}(a_i - b_i)^2}$$

**構文**: `stat_euclidean_dist(column_a, column_b)`

> **最小データ数**: 1

```sql
-- 2つのベクトル間のユークリッド距離
SELECT stat_euclidean_dist(feature_a, feature_b) FROM data;
```

---

### stat_manhattan_dist

**マンハッタン距離** (L1 ノルム) を計算する。

$$d = \sum_{i=1}^{n}|a_i - b_i|$$

**構文**: `stat_manhattan_dist(column_a, column_b)`

> **最小データ数**: 1

```sql
SELECT stat_manhattan_dist(x, y) FROM coordinates;
```

---

### stat_cosine_sim

**コサイン類似度**を計算する。

$$\text{sim} = \frac{\mathbf{a} \cdot \mathbf{b}}{|\mathbf{a}||\mathbf{b}|}$$

**構文**: `stat_cosine_sim(column_a, column_b)`

> **最小データ数**: 1
> **戻り値の範囲**: -1.0〜1.0

```sql
-- ベクトルの類似度
SELECT stat_cosine_sim(user_a_rating, user_b_rating) FROM item_ratings;
```

---

### stat_cosine_dist

**コサイン距離**を計算する。`1 - cosine_similarity`。

**構文**: `stat_cosine_dist(column_a, column_b)`

> **最小データ数**: 1
> **戻り値の範囲**: 0.0〜2.0

```sql
SELECT stat_cosine_dist(vec_a, vec_b) FROM embeddings;
```

---

### stat_minkowski_dist

**ミンコフスキー距離** (Lp ノルム) を計算する。

$$d = \left(\sum_{i=1}^{n}|a_i - b_i|^p\right)^{1/p}$$

**構文**: `stat_minkowski_dist(column_a, column_b, p)`

| パラメータ | 説明 | 典型値 |
|---|---|---|
| `p` | 次数パラメータ | p=1: マンハッタン, p=2: ユークリッド |

> **最小データ数**: 1

```sql
-- L3 ノルム
SELECT stat_minkowski_dist(x, y, 3.0) FROM data;
```

---

### stat_chebyshev_dist

**チェビシェフ距離** (L∞ ノルム) を計算する。要素間の差の最大値。

$$d = \max_i |a_i - b_i|$$

**構文**: `stat_chebyshev_dist(column_a, column_b)`

> **最小データ数**: 1

```sql
SELECT stat_chebyshev_dist(x, y) FROM data;
```

---
