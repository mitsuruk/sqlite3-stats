# 複合集約関数（41関数）

JSON 結果を返す集約関数、2標本検定、生存時間解析等。複数カラムを入力に取り、複合的な統計結果を返す。

← [関数リファレンス](../function_reference-ja.md) に戻る

---

## 基本統計量（複数結果）

### stat_modes

**最頻値（すべて）** を JSON 配列で返す。

**構文**: `stat_modes(column)`

```sql
SELECT stat_modes(score) FROM students;
-- → [85.0, 90.0]（複数の最頻値がある場合）
```

---

### stat_five_number_summary

**五数要約**（最小値、Q1、中央値、Q3、最大値）を JSON で返す。

**構文**: `stat_five_number_summary(column)`

```sql
SELECT stat_five_number_summary(val) FROM data;
-- → {"min":1.0,"q1":3.0,"median":5.0,"q3":7.0,"max":10.0}
```

---

## 度数分布

### stat_frequency_table

**度数表**を JSON で返す。

**構文**: `stat_frequency_table(column)`

```sql
SELECT stat_frequency_table(grade) FROM students;
```

---

### stat_frequency_count

**各値の度数**を JSON で返す。

**構文**: `stat_frequency_count(column)`

```sql
SELECT stat_frequency_count(category) FROM products;
```

---

### stat_relative_frequency

**相対度数**を JSON で返す。

**構文**: `stat_relative_frequency(column)`

```sql
SELECT stat_relative_frequency(rating) FROM reviews;
```

---

### stat_cumulative_frequency

**累積度数**を JSON で返す。

**構文**: `stat_cumulative_frequency(column)`

```sql
SELECT stat_cumulative_frequency(score) FROM exam;
```

---

### stat_cumulative_relative_frequency

**累積相対度数**を JSON で返す。

**構文**: `stat_cumulative_relative_frequency(column)`

```sql
SELECT stat_cumulative_relative_frequency(score) FROM exam;
```

---

## 2標本検定

### stat_t_test2

**2標本 t 検定**（併合分散）を実行する。2カラムにそれぞれのグループの値を渡す。

**構文**: `stat_t_test2(group1, group2)`

```sql
SELECT stat_t_test2(before_score, after_score) FROM experiment;
-- → {"statistic":...,"p_value":...,"df":...}
```

---

### stat_t_test_welch

**Welch t 検定**（等分散を仮定しない2標本検定）を実行する。

**構文**: `stat_t_test_welch(group1, group2)`

```sql
SELECT stat_t_test_welch(control, treatment) FROM trial;
```

---

### stat_chisq_independence

**カイ二乗独立性検定**を実行する。2つのカテゴリカル変数を渡す。

**構文**: `stat_chisq_independence(col1, col2)`

```sql
SELECT stat_chisq_independence(gender, preference) FROM survey;
```

---

### stat_f_test

**F 検定**（2群の分散比較）を実行する。

**構文**: `stat_f_test(group1, group2)`

```sql
SELECT stat_f_test(method_a, method_b) FROM quality;
```

---

### stat_mann_whitney

**Mann-Whitney U 検定**（ノンパラメトリックな2標本検定）を実行する。

**構文**: `stat_mann_whitney(group1, group2)`

```sql
SELECT stat_mann_whitney(drug, placebo) FROM trial;
```

---

## 分散分析

### stat_anova1

**一元配置分散分析**を実行する。第1引数に値、第2引数にグループラベルを渡す。

**構文**: `stat_anova1(value, group)`

```sql
SELECT stat_anova1(score, class) FROM students;
-- → {"f_statistic":...,"p_value":...,"df_between":...,"df_within":...}
```

---

## カテゴリカル

### stat_contingency_table

**分割表**を JSON で作成する。2つのカテゴリカル変数を渡す。

**構文**: `stat_contingency_table(col1, col2)`

```sql
SELECT stat_contingency_table(treatment, outcome) FROM patients;
```

---

## 効果量（2標本）

### stat_cohens_d2

**Cohen's d（2標本）** を計算する。

**構文**: `stat_cohens_d2(group1, group2)`

```sql
SELECT stat_cohens_d2(control, treatment) FROM experiment;
```

---

### stat_hedges_g2

**Hedges' g（2標本）** を計算する（小標本補正付き）。

**構文**: `stat_hedges_g2(group1, group2)`

```sql
SELECT stat_hedges_g2(control, treatment) FROM experiment;
```

---

### stat_glass_delta

**Glass's Delta** を計算する（対照群の標準偏差で標準化）。

**構文**: `stat_glass_delta(control, treatment)`

```sql
SELECT stat_glass_delta(control, treatment) FROM experiment;
```

---

## 2標本差の信頼区間

### stat_ci_mean_diff

**2標本平均差の信頼区間**（併合分散）を JSON で返す。

**構文**: `stat_ci_mean_diff(group1, group2)`

```sql
SELECT stat_ci_mean_diff(before_val, after_val) FROM study;
```

---

### stat_ci_mean_diff_welch

**2標本平均差の信頼区間（Welch 法）** を JSON で返す。

**構文**: `stat_ci_mean_diff_welch(group1, group2)`

```sql
SELECT stat_ci_mean_diff_welch(control, treatment) FROM trial;
```

---

## 生存時間解析

### stat_kaplan_meier

**Kaplan-Meier 生存曲線**を JSON で返す。

**構文**: `stat_kaplan_meier(time, event)`

| パラメータ | 説明 |
|---|---|
| `time` | 生存時間 |
| `event` | イベント発生（1）/ 打ち切り（0） |

```sql
SELECT stat_kaplan_meier(survival_time, event_flag) FROM patients;
```

---

### stat_nelson_aalen

**Nelson-Aalen 累積ハザード推定**を JSON で返す。

**構文**: `stat_nelson_aalen(time, event)`

```sql
SELECT stat_nelson_aalen(time, event) FROM survival_data;
```

---

### stat_logrank

**Log-rank 検定**（2群の生存曲線の比較）を JSON で返す。3カラムを受け取る。

**構文**: `stat_logrank(time, event, group)`

| パラメータ | 説明 |
|---|---|
| `time` | 生存時間 |
| `event` | イベント発生（1）/ 打ち切り（0） |
| `group` | グループラベル |

```sql
SELECT stat_logrank(time, event, treatment_group) FROM clinical_trial;
```

---

## リサンプリング

### stat_bootstrap

**汎用ブートストラップ推定**を JSON で返す。

**構文**: `stat_bootstrap(column, n_bootstrap)`

```sql
SELECT stat_bootstrap(val, 1000) FROM data;
```

---

### stat_bootstrap_bca

**BCa（bias-corrected and accelerated）ブートストラップ**を JSON で返す。

**構文**: `stat_bootstrap_bca(column, n_bootstrap)`

```sql
SELECT stat_bootstrap_bca(val, 1000) FROM data;
```

---

### stat_bootstrap_sample

**ブートストラップサンプル**を JSON 配列で生成する。

**構文**: `stat_bootstrap_sample(column)`

```sql
SELECT stat_bootstrap_sample(val) FROM data;
```

---

### stat_permutation_test2

**2標本置換検定**を JSON で返す。

**構文**: `stat_permutation_test2(group1, group2)`

```sql
SELECT stat_permutation_test2(control, treatment) FROM experiment;
```

---

### stat_permutation_paired

**対応のある置換検定**を JSON で返す。

**構文**: `stat_permutation_paired(x, y)`

```sql
SELECT stat_permutation_paired(before, after) FROM paired_data;
```

---

### stat_permutation_corr

**相関の置換検定**を JSON で返す。

**構文**: `stat_permutation_corr(x, y)`

```sql
SELECT stat_permutation_corr(study_hours, test_score) FROM students;
```

---

## 時系列

### stat_acf

**自己相関関数**（ACF）を JSON 配列で返す。

**構文**: `stat_acf(column, max_lag)`

```sql
SELECT stat_acf(price, 20) FROM stock_daily;
```

---

### stat_pacf

**偏自己相関関数**（PACF）を JSON 配列で返す。

**構文**: `stat_pacf(column, max_lag)`

```sql
SELECT stat_pacf(price, 20) FROM stock_daily;
```

---

## サンプリング

### stat_sample_replace

**復元抽出**（重複あり）を JSON 配列で返す。

**構文**: `stat_sample_replace(column, n)`

```sql
SELECT stat_sample_replace(val, 10) FROM data;
```

---

### stat_sample

**非復元抽出**（重複なし）を JSON 配列で返す。

**構文**: `stat_sample(column, n)`

```sql
SELECT stat_sample(val, 5) FROM data;
```

---

## 群列パターンの検定

`stat_anova1` と同じく「値列 + 群列」を取る。群は群列の値の昇順に
0, 1, 2, … と番号が振られる。

| 関数 | 構文 | 説明 |
|---|---|---|
| `stat_kruskal_wallis` | `stat_kruskal_wallis(val, grp)` | Kruskal-Wallis 検定 |
| `stat_levene` | `stat_levene(val, grp)` | Levene 検定（等分散性） |
| `stat_bartlett` | `stat_bartlett(val, grp)` | Bartlett 検定（等分散性） |
| `stat_cohens_f` | `stat_cohens_f(val, grp)` | Cohen's f（分散分析の効果量） |

`stat_kruskal_wallis` は一元配置分散分析のノンパラメトリック版で、正規性が
疑わしい場合に使う。R の `kruskal.test()` と一致する。

`stat_levene` と `stat_bartlett` は、`stat_anova1` や `stat_t_test2` が前提と
する等分散性を検定する。Levene は**中央値基準の Brown-Forsythe 版**で、
正規性からの逸脱に頑健であり、R の `car::leveneTest()` の既定と一致する。
Bartlett は正規分布を前提とし、それが成り立つ場合により検出力が高い。
R の `bartlett.test()` と一致する。

いずれも `{"statistic", "p_value", "df"}` を返す。群が 2 つ未満の場合は
NULL を返す。NULL 行は群分割の前に除外される。

```sql
-- 分散分析の前に等分散性を確認する
SELECT stat_levene(score, class_id)  AS levene,
       stat_bartlett(score, class_id) AS bartlett,
       stat_anova1(score, class_id)   AS anova
FROM exam_results;

-- 正規性が疑わしい場合はノンパラメトリック検定を使う
SELECT stat_kruskal_wallis(score, class_id) FROM exam_results;
```

---

## 事後検定

| 関数 | 構文 |
|---|---|
| `stat_tukey_hsd` | `stat_tukey_hsd(val, grp [,alpha])` |
| `stat_bonferroni_posthoc` | `stat_bonferroni_posthoc(val, grp [,alpha])` |
| `stat_scheffe_posthoc` | `stat_scheffe_posthoc(val, grp [,alpha])` |
| `stat_dunnett_posthoc` | `stat_dunnett_posthoc(val, grp [,ctrl, alpha])` |

`stat_anova1` が有意差を示した後に、どの群同士が異なるかを特定する。
一元配置分散分析は内部で実行されるため、値列と群列をそのまま渡せばよい。
`alpha` の既定値は 0.05、Dunnett の対照群添字 `ctrl` の既定値は 0。

保守性は Tukey < Bonferroni < Scheffe の順に強くなる。Dunnett は対照群との
比較のみを行うため、比較数は k(k-1)/2 ではなく k-1 になる。

**戻り値**:

```json
{"method": "Tukey HSD", "alpha": 0.05, "mse": 2.5, "df_error": 12,
 "comparisons": [
   {"group1": 0, "group2": 1, "mean_diff": -9.0, "se": 0.707,
    "statistic": 12.73, "p_value": 3.08e-06,
    "lower": -11.67, "upper": -6.33, "significant": true}]}
```

> **符号の規約**: `mean_diff` は group1 - group2 であり、group1 には常に
> 小さい方の添字が入る。R の `TukeyHSD()` は逆向き（"2-1"）で報告するため、
> 平均差と信頼区間の上下限が R とは符号反転して見える。絶対値と p 値は同一。
>
> **注意**: `stat_dunnett_posthoc` は厳密な多変量 t 分布ではなく Bonferroni
> 近似を用いるため、R の `multcomp::glht()` よりわずかに保守的になる。

```sql
-- 分散分析で有意差が出た後、どのクラス間で差があるかを調べる
SELECT stat_tukey_hsd(score, class_id) FROM exam_results;

-- 有意なペアだけを抽出する
SELECT json_extract(c.value, '$.group1') AS g1,
       json_extract(c.value, '$.group2') AS g2,
       json_extract(c.value, '$.p_value') AS p
FROM (SELECT stat_tukey_hsd(score, class_id) AS j FROM exam_results) t,
     json_each(t.j, '$.comparisons') c
WHERE json_extract(c.value, '$.significant');

-- 全処理群を対照群（添字 0）と比較する
SELECT stat_dunnett_posthoc(score, class_id, 0, 0.05) FROM exam_results;
```

---

## 層化抽出

| 関数 | 構文 |
|---|---|
| `stat_stratified_sample` | `stat_stratified_sample(val, grp [,ratio])` |

各層から同じ**割合**で抽出するため、標本の層構成が母集団と一致する。
`ratio` は (0, 1] の範囲で、既定値は 0.5。抽出された値の JSON 配列を返す。

```sql
-- 地域構成を保ったまま各地域から 30% を抽出する
SELECT stat_stratified_sample(revenue, region_id, 0.3) FROM sales;
```

> **注意**: 抽出は無作為であり、現時点では再現性がない（シード指定の手段が
> まだ提供されていない）。

---
