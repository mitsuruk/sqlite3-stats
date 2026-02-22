# 基本集約関数（24関数）

1カラムの集約関数。`SELECT stat_xxx(column) FROM table` の形式で使用する。`GROUP BY`、`HAVING`、サブクエリなど SQLite3 の標準的な集約関数が使えるすべての文脈で利用可能。

← [関数リファレンス](../function_reference-ja.md) に戻る

---

## 基本統計量

### stat_mean

**算術平均**を計算する。

$$\bar{x} = \frac{1}{n}\sum_{i=1}^{n} x_i$$

**構文**: `stat_mean(column)`

```sql
-- 基本的な使い方
SELECT stat_mean(score) FROM students;

-- 部門ごとの平均給与
SELECT department, stat_mean(salary) AS avg_salary
FROM employees
GROUP BY department;

-- サブクエリで平均との差分を計算
SELECT name, score,
       score - (SELECT stat_mean(score) FROM students) AS diff_from_mean
FROM students;

-- HAVING で平均が一定以上のグループを抽出
SELECT class, stat_mean(score) AS avg_score
FROM students
GROUP BY class
HAVING stat_mean(score) >= 70;

-- SQLite3 組み込み AVG() との比較
SELECT stat_mean(val) AS stat_mean_result,
       AVG(val)        AS sqlite_avg_result
FROM data;
-- → 両者は同じ結果を返す
```

---

### stat_median

**中央値**を計算する。データをソートしたときの中央の値。偶数個の場合は中央2値の平均。

**構文**: `stat_median(column)`

```sql
-- 基本的な使い方
SELECT stat_median(price) FROM products;

-- 平均と中央値を比較して分布の偏りを確認
SELECT stat_mean(income)   AS mean_income,
       stat_median(income) AS median_income
FROM households;
-- mean > median → 右に歪んだ分布（高所得者の影響）
-- mean ≈ median → 対称に近い分布

-- カテゴリごとの中央値
SELECT region,
       stat_median(sales) AS median_sales
FROM store_data
GROUP BY region
ORDER BY median_sales DESC;

-- 中央値による外れ値に強い代表値
-- 例: [1, 2, 3, 4, 1000] → mean=202, median=3
CREATE TABLE outlier_data (val REAL);
INSERT INTO outlier_data VALUES (1),(2),(3),(4),(1000);

SELECT stat_mean(val)   AS mean,    -- 202.0
       stat_median(val) AS median   -- 3.0
FROM outlier_data;
```

---

### stat_mode

**最頻値**（最も頻繁に出現する値）を返す。複数の最頻値がある場合は最小値を返す。

**構文**: `stat_mode(column)`

```sql
-- 基本的な使い方
SELECT stat_mode(shoe_size) FROM customers;

-- 最頻値と平均・中央値の比較
SELECT stat_mode(rating)   AS mode_rating,
       stat_mean(rating)   AS mean_rating,
       stat_median(rating) AS median_rating
FROM reviews;

-- グループごとの最頻値
SELECT category, stat_mode(color) AS most_common_color
FROM products
GROUP BY category;

-- 全値がユニークな場合は最小値を返す
CREATE TABLE unique_data (val REAL);
INSERT INTO unique_data VALUES (5),(3),(1),(4),(2);

SELECT stat_mode(val) FROM unique_data;
-- → 1.0（全値が1回ずつなので最小値）
```

---

### stat_geometric_mean

**幾何平均**を計算する。成長率や比率の平均に適する。

$$\bar{x}_g = \left(\prod_{i=1}^{n} x_i\right)^{1/n}$$

**構文**: `stat_geometric_mean(column)`

> **制約**: すべての値が正の数でなければならない。

```sql
-- 投資の年間平均成長率
-- 3年間の成長率: 1.10 (10%), 1.05 (5%), 1.20 (20%)
CREATE TABLE growth (rate REAL);
INSERT INTO growth VALUES (1.10),(1.05),(1.20);

SELECT stat_geometric_mean(rate) AS avg_growth_rate;
-- → 1.11535... (平均 +11.5%)

-- 算術平均との違い
SELECT stat_mean(rate)           AS arithmetic_mean,  -- 1.1167
       stat_geometric_mean(rate) AS geometric_mean     -- 1.1154
FROM growth;
-- 幾何平均は常に算術平均以下（AM-GM不等式）

-- 細菌の増殖倍率の平均
SELECT stat_geometric_mean(multiplication_factor) AS avg_factor
FROM bacteria_growth;
```

---

### stat_harmonic_mean

**調和平均**を計算する。速度や比率の平均に適する。

$$\bar{x}_h = \frac{n}{\sum_{i=1}^{n} \frac{1}{x_i}}$$

**構文**: `stat_harmonic_mean(column)`

> **制約**: すべての値が非ゼロでなければならない。

```sql
-- 往復の平均速度
-- 行き: 60 km/h、帰り: 40 km/h
CREATE TABLE speed (kmph REAL);
INSERT INTO speed VALUES (60),(40);

SELECT stat_harmonic_mean(kmph) AS avg_speed;
-- → 48.0 km/h（算術平均 50 より小さい、これが正しい平均速度）

-- 3つの平均の比較
SELECT stat_mean(val)           AS arithmetic,
       stat_geometric_mean(val) AS geometric,
       stat_harmonic_mean(val)  AS harmonic
FROM positive_data;
-- 常に: harmonic ≤ geometric ≤ arithmetic

-- 株価収益率 (P/E比) の平均
SELECT stat_harmonic_mean(pe_ratio) AS avg_pe
FROM stocks
WHERE pe_ratio > 0;
```

---

## 散布度・ばらつき

### stat_range

**範囲**（最大値 − 最小値）を計算する。

**構文**: `stat_range(column)`

```sql
-- 基本的な使い方
SELECT stat_range(temperature) AS temp_range FROM weather;

-- グループごとの範囲を比較
SELECT sensor_id,
       MIN(reading)        AS min_val,
       MAX(reading)        AS max_val,
       stat_range(reading) AS range_val
FROM sensor_data
GROUP BY sensor_id;

-- 品質管理: 範囲が基準値を超えるバッチを検出
SELECT batch_id, stat_range(measurement) AS r
FROM quality_data
GROUP BY batch_id
HAVING stat_range(measurement) > 5.0;
```

---

### stat_var

**分散**を計算する（母分散、ddof=0）。

$$\sigma^2 = \frac{1}{n}\sum_{i=1}^{n}(x_i - \bar{x})^2$$

**構文**: `stat_var(column)`

> **注意**: `stat_var` は母分散（ddof=0）を返す。標本分散（不偏分散）が必要な場合は `stat_sample_variance` を使用する。

```sql
-- 母分散
SELECT stat_var(score) AS variance FROM exam;

-- 母分散と標本分散の比較
SELECT stat_var(score)             AS pop_var,   -- N で割る
       stat_sample_variance(score) AS sample_var  -- N-1 で割る
FROM exam;
```

---

### stat_population_variance

**母分散**を計算する。`stat_var` と同等。

$$\sigma^2 = \frac{1}{N}\sum_{i=1}^{N}(x_i - \mu)^2$$

**構文**: `stat_population_variance(column)`

```sql
-- 全数調査（母集団全体のデータ）の場合に使用
SELECT stat_population_variance(height) FROM census_data;

-- 工場の全製品の品質指標
SELECT production_line,
       stat_population_variance(weight) AS var_weight
FROM all_products
GROUP BY production_line;
```

---

### stat_sample_variance

**標本分散**（不偏分散）を計算する。

$$s^2 = \frac{1}{n-1}\sum_{i=1}^{n}(x_i - \bar{x})^2$$

**構文**: `stat_sample_variance(column)`

> **最小データ数**: 2

```sql
-- 標本から母集団の分散を推定する場合に使用
SELECT stat_sample_variance(weight) FROM sample_products;

-- 2グループの分散を比較
SELECT group_name,
       stat_sample_variance(score) AS var_score
FROM experiment
GROUP BY group_name;

-- 分散が最も大きいカテゴリを特定
SELECT category, stat_sample_variance(price) AS var_price
FROM products
GROUP BY category
ORDER BY var_price DESC
LIMIT 1;
```

---

### stat_stdev

**標準偏差**を計算する（母標準偏差、ddof=0）。

$$\sigma = \sqrt{\frac{1}{n}\sum_{i=1}^{n}(x_i - \bar{x})^2}$$

**構文**: `stat_stdev(column)`

> **注意**: `stat_stdev` は母標準偏差（ddof=0）を返す。標本標準偏差が必要な場合は `stat_sample_stddev` を使用する。

```sql
-- 母標準偏差
SELECT stat_stdev(score) AS pop_stddev FROM exam;

-- 平均 ± 1σ の範囲にある割合を確認
SELECT
    CAST(COUNT(CASE
        WHEN score BETWEEN
            (SELECT stat_mean(score) FROM exam) - (SELECT stat_stdev(score) FROM exam)
        AND (SELECT stat_mean(score) FROM exam) + (SELECT stat_stdev(score) FROM exam)
        THEN 1 END) AS REAL) / COUNT(*) * 100 AS pct_within_1sigma
FROM exam;
-- 正規分布なら約 68%
```

---

### stat_population_stddev

**母標準偏差**を計算する。`stat_stdev` と同等。

**構文**: `stat_population_stddev(column)`

```sql
SELECT stat_population_stddev(measurement) FROM all_measurements;
```

---

### stat_sample_stddev

**標本標準偏差**を計算する。

$$s = \sqrt{\frac{1}{n-1}\sum_{i=1}^{n}(x_i - \bar{x})^2}$$

**構文**: `stat_sample_stddev(column)`

> **最小データ数**: 2

```sql
-- 標本標準偏差
SELECT stat_sample_stddev(weight) FROM sample_data;

-- 平均と標準偏差を一度に取得
SELECT stat_mean(score)          AS mean,
       stat_sample_stddev(score) AS stddev
FROM test_scores;

-- グループごとの記述統計を一括計算
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

**変動係数** (Coefficient of Variation) を計算する。標本標準偏差を平均の絶対値で割った値で、単位の異なるデータのばらつきを比較可能。

$$CV = \frac{s}{|\bar{x}|}$$

**構文**: `stat_cv(column)`

> **最小データ数**: 2

```sql
-- 変動係数
SELECT stat_cv(height) AS cv_height,
       stat_cv(weight) AS cv_weight
FROM people;
-- 身長と体重のどちらが相対的にばらつきが大きいか比較可能

-- CV が大きいほどばらつきが大きい
SELECT product_type,
       stat_mean(price) AS mean_price,
       stat_cv(price)   AS cv_price
FROM products
GROUP BY product_type
ORDER BY cv_price DESC;

-- 安定性の指標として利用
-- CV < 0.1: 非常に安定、CV > 0.3: ばらつきが大きい
SELECT sensor_id, stat_cv(reading) AS stability
FROM readings
GROUP BY sensor_id
HAVING stat_cv(reading) > 0.3;
```

---

### stat_iqr

**四分位範囲** (Interquartile Range) を計算する。Q3 − Q1。

**構文**: `stat_iqr(column)`

```sql
-- 四分位範囲
SELECT stat_iqr(score) AS iqr FROM exam;

-- 外れ値の検出基準としての IQR
-- 一般的な基準: Q1 - 1.5*IQR 未満 または Q3 + 1.5*IQR 超過
-- ※ Q1, Q3 の計算は stat_quartile を参照

-- 中央値と IQR で分布を要約（外れ値に強い記述統計）
SELECT stat_median(price) AS median_price,
       stat_iqr(price)    AS iqr_price
FROM listings;

-- グループごとの IQR 比較
SELECT category,
       stat_iqr(duration) AS iqr_duration
FROM events
GROUP BY category;
```

---

### stat_mad_mean

**平均偏差**（平均絶対偏差、Mean Absolute Deviation from mean）を計算する。

$$MAD_{mean} = \frac{1}{n}\sum_{i=1}^{n}|x_i - \bar{x}|$$

**構文**: `stat_mad_mean(column)`

```sql
-- 平均偏差
SELECT stat_mad_mean(score) AS mad_from_mean FROM exam;

-- 標準偏差との比較
-- 平均偏差は外れ値の影響を受けにくい
SELECT stat_mad_mean(val) AS mad_mean,
       stat_stdev(val)    AS stdev
FROM data;

-- グループごとのばらつき比較
SELECT region,
       stat_mad_mean(delivery_time) AS avg_deviation
FROM orders
GROUP BY region;
```

---

### stat_geometric_stddev

**幾何標準偏差** (Geometric Standard Deviation) を計算する。対数正規分布に従うデータのばらつきの指標。

**構文**: `stat_geometric_stddev(column)`

> **制約**: すべての値が正の数でなければならない。

```sql
-- 幾何標準偏差
SELECT stat_geometric_mean(concentration) AS geo_mean,
       stat_geometric_stddev(concentration) AS geo_stddev
FROM environmental_samples;

-- 対数正規分布のデータ（粒子径、濃度、所得など）に有用
-- 幾何平均 × 幾何標準偏差^±1 が概ね68%の範囲に対応
SELECT stat_geometric_mean(particle_size) AS gm,
       stat_geometric_stddev(particle_size) AS gsd
FROM aerosol_data;
```

---

## 分布の形状

### stat_population_skewness

**母歪度**を計算する。分布の非対称性の指標。

$$g_1 = \frac{\frac{1}{N}\sum_{i=1}^{N}(x_i - \mu)^3}{\sigma^3}$$

**構文**: `stat_population_skewness(column)`

> **最小データ数**: 3

```sql
-- 歪度の解釈
SELECT stat_population_skewness(income) AS skewness FROM survey;
-- skewness > 0: 右に裾が長い（正の歪み） — 所得分布に多い
-- skewness = 0: 対称（正規分布に近い）
-- skewness < 0: 左に裾が長い（負の歪み）

-- 分布の形状をまとめて確認
SELECT stat_mean(val)                 AS mean,
       stat_median(val)               AS median,
       stat_population_skewness(val)  AS skewness,
       stat_population_kurtosis(val)  AS kurtosis
FROM measurements;
```

---

### stat_skewness

**標本歪度**（Fisher の補正付き）を計算する。

$$G_1 = \frac{n}{(n-1)(n-2)} \cdot \frac{\sum_{i=1}^{n}(x_i - \bar{x})^3}{s^3}$$

**構文**: `stat_skewness(column)`

> **最小データ数**: 3

```sql
-- 標本歪度（標本から母集団の歪度を推定）
SELECT stat_skewness(response_time) AS skewness FROM api_logs;

-- 母歪度と標本歪度の比較
SELECT stat_population_skewness(val) AS pop_skew,
       stat_skewness(val)            AS sample_skew
FROM data;
-- n が大きいほど両者は近づく

-- 正規性の簡易チェック
-- |skewness| < 2 かつ |kurtosis| < 7 なら概ね正規分布と見なせる
SELECT stat_skewness(val)  AS skew,
       stat_kurtosis(val)  AS kurt
FROM data;
```

---

### stat_population_kurtosis

**母尖度**（超過尖度）を計算する。正規分布を基準 (0) とした尖り具合の指標。

$$g_2 = \frac{\frac{1}{N}\sum_{i=1}^{N}(x_i - \mu)^4}{\sigma^4} - 3$$

**構文**: `stat_population_kurtosis(column)`

> **最小データ数**: 4

```sql
-- 尖度の解釈
SELECT stat_population_kurtosis(val) AS kurtosis FROM data;
-- kurtosis > 0: 尖った分布（裾が重い） — leptokurtic
-- kurtosis = 0: 正規分布と同じ — mesokurtic
-- kurtosis < 0: 平坦な分布（裾が軽い） — platykurtic

-- 金融データのリスク評価
-- 尖度が高い → 極端な値（暴落・暴騰）が起きやすい
SELECT asset_name,
       stat_population_kurtosis(daily_return) AS kurtosis
FROM returns
GROUP BY asset_name
ORDER BY kurtosis DESC;
```

---

### stat_kurtosis

**標本尖度**（超過尖度、Fisher の補正付き）を計算する。

**構文**: `stat_kurtosis(column)`

> **最小データ数**: 4

```sql
-- 標本尖度
SELECT stat_kurtosis(val) AS kurtosis FROM sample_data;

-- 分布の形状の完全な記述統計
SELECT stat_mean(val)     AS mean,
       stat_median(val)   AS median,
       stat_stdev(val)    AS stddev,
       stat_skewness(val) AS skewness,
       stat_kurtosis(val) AS kurtosis
FROM sample_data;
```

---

## 推定

### stat_se

**標準誤差** (Standard Error of the Mean) を計算する。平均値の推定精度の指標。

$$SE = \frac{s}{\sqrt{n}}$$

**構文**: `stat_se(column)`

> **最小データ数**: 2

```sql
-- 標準誤差
SELECT stat_se(score) AS se FROM exam;

-- 平均値の信頼区間の概算（95% CI ≈ mean ± 1.96 * SE）
SELECT stat_mean(val) AS mean,
       stat_se(val)   AS se,
       stat_mean(val) - 1.96 * stat_se(val) AS ci_lower_95,
       stat_mean(val) + 1.96 * stat_se(val) AS ci_upper_95
FROM large_sample;
-- 注: これは n が十分大きい場合の近似。正確な CI は stat_ci_mean を参照。

-- サンプルサイズと標準誤差の関係
SELECT COUNT(val) AS n,
       stat_sample_stddev(val) AS stddev,
       stat_se(val)            AS se
FROM data;
-- n が4倍になると SE は半分になる

-- グループごとの推定精度比較
SELECT group_name,
       COUNT(score) AS n,
       stat_mean(score) AS mean,
       stat_se(score)   AS se
FROM experiment
GROUP BY group_name;
```

---

## ロバスト統計

### stat_mad

**中央絶対偏差** (Median Absolute Deviation) を計算する。外れ値に対して非常に頑健なばらつきの指標。

$$MAD = \text{median}(|x_i - \tilde{x}|)$$

ここで $\tilde{x}$ はデータの中央値。

**構文**: `stat_mad(column)`

```sql
-- MAD（外れ値に強いばらつきの指標）
SELECT stat_mad(val) AS mad FROM data;

-- 標準偏差との比較（外れ値がある場合）
CREATE TABLE with_outlier (val REAL);
INSERT INTO with_outlier VALUES (1),(2),(3),(4),(5),(100);

SELECT stat_stdev(val) AS stdev,  -- 外れ値で大きく膨らむ
       stat_mad(val)   AS mad     -- 外れ値の影響が小さい
FROM with_outlier;

-- Modified Z-score による外れ値検出の基準値として利用
-- Modified Z-score = 0.6745 * (x - median) / MAD
-- |Modified Z-score| > 3.5 なら外れ値とみなす
SELECT val,
       0.6745 * (val - (SELECT stat_median(val) FROM data))
               / (SELECT stat_mad(val) FROM data) AS modified_z
FROM data;
```

---

### stat_mad_scaled

**スケーリングされた MAD** を計算する。正規分布のもとで標準偏差と同等のスケールになるよう補正した値。

$$MAD_{scaled} = 1.4826 \times MAD$$

**構文**: `stat_mad_scaled(column)`

```sql
-- 正規分布なら stat_sample_stddev と近い値になる
SELECT stat_sample_stddev(val) AS stddev,
       stat_mad_scaled(val)    AS mad_scaled
FROM normal_data;

-- 外れ値がある場合、MAD_scaled のほうが頑健
SELECT stat_sample_stddev(val) AS stddev,     -- 外れ値で膨らむ
       stat_mad_scaled(val)    AS mad_scaled   -- 頑健
FROM data_with_outliers;

-- 品質管理での利用
SELECT batch_id,
       stat_median(weight)     AS center,
       stat_mad_scaled(weight) AS spread
FROM production
GROUP BY batch_id;
```

---

### stat_hodges_lehmann

**Hodges-Lehmann 推定量**を計算する。全ペアワイズ平均の中央値。ロバストな位置の推定量。

$$\hat{\theta}_{HL} = \text{median}\left\{\frac{x_i + x_j}{2} : 1 \le i \le j \le n\right\}$$

**構文**: `stat_hodges_lehmann(column)`

```sql
-- Hodges-Lehmann 推定量（ロバストな位置推定）
SELECT stat_hodges_lehmann(val) AS hl_estimate FROM data;

-- 平均・中央値との比較
SELECT stat_mean(val)            AS mean,
       stat_median(val)          AS median,
       stat_hodges_lehmann(val)  AS hodges_lehmann
FROM data;

-- 外れ値がある場合の比較
CREATE TABLE hl_test (val REAL);
INSERT INTO hl_test VALUES (1),(2),(3),(4),(5),(100);

SELECT stat_mean(val)            AS mean,           -- 19.17（外れ値に引かれる）
       stat_median(val)          AS median,          -- 3.5
       stat_hodges_lehmann(val)  AS hodges_lehmann   -- 平均と中央値の中間的なロバスト性
FROM hl_test;

-- Wilcoxon 符号付順位検定の点推定量としても使われる
SELECT stat_hodges_lehmann(val) AS location_estimate
FROM treatment_effects;
```

---
