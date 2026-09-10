# statcpp 関数カバレッジ分析

対象: sqlite3StatisticalLibrary (SQL関数 249個) / statcpp v0.4.0 (公開関数 384個)
分析日: 2026-09-10

## 略語一覧

|略語|フルスペル|
|---|---|
|BH|Benjamini-Hochberg|
|GLM|Generalized Linear Model|
|PCA|Principal Component Analysis|
|MCAR|Missing Completely At Random|
|PMM|Predictive Mean Matching|
|VIF|Variance Inflation Factor|
|CV|Cross Validation|
|LOOCV|Leave-One-Out Cross Validation|
|IRLS|Iteratively Reweighted Least Squares|

## 1. 結論(サマリ)

statcpp の公開関数 384個(API_REFERENCE.md 掲載, 重複含め389エントリ)のうち,
`statcpp::` として実際に呼ばれているのは **248個**, 未使用は **141個**.

ただし未使用141個は一様ではなく,以下の5群に分解できる.

|群|件数|内容|対応価値|
|---|---|---|---|
|A|16|実質ギャップなし(別名 / SQL組込関数 / 同等実装済 / 内部ヘルパ)|なし|
|B|15|数値内部ユーティリティ(`numerical_utils.hpp`)|低|
|C|17|SQL の `WHERE` / `GROUP BY` / `ORDER BY` で自然に書ける|低|
|D|36|**既存インフラで追加可能**|**高**|
|E|57|**行列(N行×P列)入力が必要 → 新インフラ必須**|**高(最大の障壁)**|

実質的な「取りこぼし」は **D群36個 + E群57個 = 93個**.
そのうち **E群57個は単一の根本原因** に帰着する(§4).

また分析の副産物として,**statcpp を呼ばず独自再実装している箇所に正しさの問題を1件検出**した(§6).

## 2. モジュール別カバレッジ

|モジュール|全数|使用|未使用|主な未使用|
|---|--:|--:|--:|---|
|basic_statistics|14|9|5|argmin, argmax|
|dispersion_spread|15|13|2|(別名のみ)|
|order_statistics|8|4|4|(SQL組込 / 内部ヘルパ)|
|shape_of_distribution|6|4|2|(別名のみ)|
|correlation_covariance|7|6|1|(別名のみ)|
|special_functions|14|13|1|norm_sf|
|random_engine|3|0|3|**set_seed, randomize_seed**|
|continuous_distributions|42|40|2|studentized_range_cdf/quantile|
|discrete_distributions|31|31|0|-|
|estimation|15|14|1|ci_mean_diff_pooled|
|parametric_tests|14|11|3|**bonferroni / BH / holm 補正**|
|nonparametric_tests|11|5|6|**kruskal_wallis, levene, bartlett**|
|effect_size|18|17|1|risk_ratio|
|resampling|9|9|0|-|
|power_analysis|8|6|2|power_analysis_t_one_sample|
|**linear_regression**|11|3|**8**|multiple_linear_regression, VIF, 診断|
|**anova**|13|3|**10**|two_way_anova, 事後検定4種, ANCOVA|
|**glm**|12|0|**12**|logistic_regression, poisson_regression 他全部|
|**model_selection**|15|3|**12**|ridge / lasso / elastic net, CV|
|distance_metrics|7|6|1|mahalanobis_distance|
|numerical_utils|15|0|15|(内部ユーティリティ)|
|**multivariate**|7|0|**7**|PCA, 相関行列, 標準化|
|time_series|12|12|0|-|
|categorical|5|5|0|-|
|survival|4|3|1|median_survival_time|
|robust|10|8|2|cooks_distance, dffits|
|**clustering**|7|2|**5**|kmeans, 階層クラスタリング|
|data_wrangling|39|16|23|(大半は SQL で代替可)|
|**missing_data**|12|0|**12**|欠測パターン分析, 多重代入 全部|

カバレッジ 0% のモジュールは 5つ:
**glm, numerical_utils, multivariate, missing_data, random_engine**.

## 3. D群: 既存インフラで追加可能な36個

新しい仕組みを作らずに,現行の登録テンプレートへ流し込めるもの.

### D-1. スカラー関数 (`register_scalar`) — 8個

`sf_*` 関数を1つ書いて登録行を1行足すだけ.

|statcpp関数|提案SQL名|引数|
|---|---|---|
|norm_sf|stat_norm_sf|(x)|
|studentized_range_cdf|stat_studentized_range_cdf|(q, k, df)|
|studentized_range_quantile|stat_studentized_range_quantile|(p, k, df)|
|risk_ratio|stat_risk_ratio|(a, b, c, d)|
|power_analysis_t_one_sample|stat_power_analysis_t1|(d, n, alpha, alt) → JSON|
|power_analysis_t_one_sample_n|stat_power_t1_n|(d, power, alpha, alt) → JSON|
|set_seed|stat_set_seed|(seed)|
|randomize_seed|stat_randomize_seed|()|

**`set_seed` は特に重要**: 現在 `stat_*_rand` / `stat_bootstrap*` /
`stat_permutation*` / `stat_sample*` の結果は再現不能. シードを固定できないとテストにも解析にも使いにくい.

### D-2. 1列集約 → JSON (`SingleColumnAggregateText`) — 3個

多重比較補正. **現在の実装は誤り**(§6)なので置き換えを兼ねる.

|statcpp関数|提案SQL名|用法|
|---|---|---|
|bonferroni_correction|stat_bonferroni_adjust(p)|p値列を集約 → JSON配列|
|benjamini_hochberg_correction|stat_bh_adjust(p)|同上|
|holm_correction|stat_holm_adjust(p)|同上|

### D-3. 2列集約 (`TwoColumn*Aggregate*`) — 4個

|statcpp関数|提案SQL名|引数|
|---|---|---|
|ci_mean_diff_pooled|stat_ci_mean_diff_pooled|(x, y, conf)|
|median_survival_time|stat_median_survival|(time, event)|
|cooks_distance|stat_cooks_distance|(y, x) → JSON配列|
|dffits|stat_dffits|(y, x) → JSON配列|

### D-4. グループ列パターン — 9個

`stat_anova1(value, group)` と**完全に同型**. `std::map` で群に分割して
`std::vector<std::vector<double>>` を組み立てる既存コードをそのまま再利用できる.

|statcpp関数|提案SQL名|備考|
|---|---|---|
|kruskal_wallis_test|stat_kruskal_wallis(value, group)|ノンパラ版一元配置|
|levene_test|stat_levene(value, group)|等分散性検定|
|bartlett_test|stat_bartlett(value, group)|等分散性検定|
|tukey_hsd|stat_tukey_hsd(value, group, alpha)|内部で one_way_anova を実行|
|bonferroni_posthoc|stat_bonferroni_posthoc(value, group, alpha)|同上|
|dunnett_posthoc|stat_dunnett_posthoc(value, group, ctrl, alpha)|同上|
|scheffe_posthoc|stat_scheffe_posthoc(value, group, alpha)|同上|
|cohens_f|stat_cohens_f(value, group)|同上|
|stratified_sample|stat_stratified_sample(value, group, n)|data_wrangling|

**これが最も費用対効果が高い**. 等分散性検定(levene / bartlett)は t検定・ANOVA の
前提確認に必須で,現状 SQL から一切実行できないのは実用上の穴が大きい.

### D-5. 3列集約 — 5個

`stat_logrank` 用の `ThreeColumnState` が既にあるので拡張のみ.

|statcpp関数|提案SQL名|引数|
|---|---|---|
|two_way_anova|stat_anova2|(value, factorA, factorB)|
|partial_eta_squared_a|(stat_anova2 の JSON に含める)|-|
|partial_eta_squared_b|(同上)|-|
|partial_eta_squared_interaction|(同上)|-|
|one_way_ancova|stat_ancova|(value, covariate, group)|

### D-6. その他 — 7個

argmin, argmax, fillna, one_hot_encode, validate_data, validate_range, get_random_engine.
SQL からの価値は中程度. `argmin`/`argmax` は SQL の他機能で代替しやすい.

## 4. E群: 行列入力が必要な57個 — 最大の障壁

### 根本原因

以下はすべて **`const std::vector<std::vector<double>>&`(N行×P列のデータ行列)** を取る.

```cpp
multiple_linear_regression(const std::vector<std::vector<double>>& X, ...)
glm_fit(const std::vector<std::vector<double>>& X, ...)
pca(const std::vector<std::vector<double>>& data, ...)
kmeans(const std::vector<std::vector<double>>& data, std::size_t k, ...)
test_mcar_simple(const std::vector<std::vector<double>>& data)
ridge_regression(const std::vector<std::vector<double>>& X, ...)
```

一方で現行の集約インフラは **1列 / 2列 / 3列(logrank専用)の固定アリティ**しかない.
列数 P が可変である以上,固定アリティのテンプレートでは表現できない.
これがモジュール5つ(glm, multivariate, missing_data と
linear_regression / model_selection の大半)を
**丸ごと 0% に留めている単一の原因**である.

### 内訳(57個)

|モジュール|件数|
|---|--:|
|glm|12|
|model_selection|12|
|missing_data|12|
|linear_regression|8|
|multivariate|7|
|clustering|5|
|distance_metrics (mahalanobis)|1|

### 解法の比較

**案A: 可変長集約** — `stat_mlr(y, x1, x2, ..., xk)`

- SQL として最も自然で,列名がそのまま使える
- `nArg = -1` は**既に本コードベースで使用実績あり**(`register_scalar(..., -1)`)
- `xStep` で argc から1行を組み立てる処理の追加が必要

**案B: 固定アリティ多重定義** — `stat_mlr2`, `stat_mlr3`, ...

- 実装は単純だが,列数ごとに関数が増殖し上限が生じる

**案C: JSON行列をスカラー引数で受ける** — `stat_pca('[[1,2],[3,4]]')`

- 集約インフラは不要だが,SQL テーブルから JSON を組む手間がかかり
  型安全性・可読性が低い

**推奨は A**. `VariadicColumnAggregate`(状態は `std::vector<std::vector<double>>`)を
1つ追加すれば,E群57個のうち行列を「入力」とするものはすべて解ける.

### 副次課題: モデルオブジェクトの受け渡し

`predict()`, `predict_probability()`, `odds_ratios()`, `pseudo_r_squared_mcfadden()`,
`cut_dendrogram()` などは **fit 済みモデルオブジェクト**(`glm_result` 等)を引数に取る.
SQL 関数は呼び出し間で C++ オブジェクトを保持できないため,以下のいずれかが必要.

- **B-1**: モデルを JSON にシリアライズして返し,予測系はその JSON を引数に取る
  (例: `stat_glm_predict(model_json, x1, x2, ...)`)
- **B-2**: fit 関数の JSON 出力に,派生量(オッズ比, 疑似R², 残差診断)を最初から全部含める

**B-2 を主,B-1 を補助**とするのが簡潔. 新規データへの予測だけ B-1 が要る.

### 段階的な推奨順序

|段階|対象|得られるもの|
|---|---|---|
|1|`VariadicColumnAggregate` 基盤 + multiple_linear_regression|重回帰 + VIF + 残差診断 (8個)|
|2|glm (logistic / poisson) + モデルJSON|GLM 全12個|
|3|multivariate (PCA, 相関行列, 標準化)|7個|
|4|model_selection (ridge / lasso / elastic net / CV)|12個|
|5|clustering + missing_data|17個|

## 5. B群・C群: 対応不要または低優先

### numerical_utils (15個) — 対応不要

`approx_equal`, `is_zero`, `is_finite`, `has_converged*`, `log1p_safe`, `expm1_safe`,
`clamp`, `in_range`, `relative_error`, `safe_divide`, `kahan_sum`, `all_finite`,
`approx_equal_range`.

statcpp 内部の数値計算ヘルパであり,SQL から使う意味が薄い.
例外的に `kahan_sum`(補正加算による高精度合計)は SQL の `SUM()` より精度が高いため,
`stat_kahan_sum` として集約提供する価値はある.

### data_wrangling の SQL 冗長分 (17個) — 対応不要

|statcpp関数|SQL での自然な書き方|
|---|---|
|is_na|`x IS NULL`|
|dropna|`WHERE x IS NOT NULL`|
|filter, filter_rows, filter_range|`WHERE`|
|group_by, group_mean, group_sum, group_count|`GROUP BY` + 集約|
|sort_values, argsort|`ORDER BY`|
|drop_duplicates|`SELECT DISTINCT`|
|value_counts|`GROUP BY x` + `COUNT(*)`|
|get_duplicates|`GROUP BY x HAVING COUNT(*) > 1`|
|log_transform, log1p_transform, sqrt_transform|`LOG(x)`, `LOG(1+x)`, `SQRT(x)`|

これらを SQL 関数として追加すると **SQL 本来の機能と重複し,API を無駄に肥大化させる**.
「statcpp の全関数を露出する」ことより「SQL から使う価値のある関数を露出する」ことを
基準にすべき箇所.

### 別名・同等実装 (16個) — 対応不要

`sum` / `count` → SQL 組込 `SUM()` / `COUNT()`.
`minimum` / `maximum` → SQL 組込 `MIN()` / `MAX()`.
`variance` / `stddev` / `skewness` / `kurtosis` → `sample_*` 版が既に露出済の別名.
`sample_covariance` → `stat_covariance` が同等.
`ks_test_normal` → statcpp 側で **deprecated**. `stat_ks_test` は正しく後継の
`lilliefors_test` を呼んでいる(適切な対応).
`five_number_summary` → `stat_five_number_summary` が `statcpp::quartiles` で同等の結果を返す.
`interpolate_at` / `compute_ranks_with_ties` / `compute_tie_groups` → 内部ヘルパ.

## 6. 独自再実装による正しさの問題(6-1・6-2 は修正済み)

分析中に,**statcpp を呼ばず SQL 側で式を再実装している箇所**を4件検出した.
うち2件は **statcpp / R と結果が一致しない**.

### 6-1. `stat_bh_correction` — 単調性補正が欠落(不正)【修正済み】

修正前の `ext_funcs.cpp`(コミット `62f729e` で削除):

```cpp
// BH: stat_bh_correction(p, rank, total) = min(p * total / rank, 1.0)
double adj = std::min(p * total / rank, 1.0);
```

statcpp の `benjamini_hochberg_correction` は p値降順に走査して
`adj = std::min(adj, prev_adj)` で**単調性を強制**する.
現在の SQL 実装にはこのステップがない.

**反例**: p = (0.040, 0.041, 0.042), n = 3

|p|SQL現状|statcpp / R `p.adjust(method="BH")`|
|---|---|---|
|0.040|**0.120**|0.042|
|0.041|0.0615|0.042|
|0.042|0.042|0.042|

最小の p値に対して 0.120 という**過大な補正値**を返す(本来は 0.042).
有意判定を取りこぼす.

(上表は statcpp v0.4.0 を直接実行し, R の `p.adjust()` と一致することを確認済み)

### 6-2. `stat_holm_correction` — 単調性補正が欠落(不正)【修正済み】

修正前の `ext_funcs.cpp`(コミット `62f729e` で削除):

```cpp
// Holm: stat_holm_correction(p, rank, total) = min(p * (total - rank + 1), 1.0)
double adj = std::min(p * (total - rank + 1), 1.0);
```

statcpp は p値昇順に `adj = std::max(adj, max_adj)` で単調性を強制する.

**反例**: p = (0.040, 0.041, 0.042), n = 3

|p|SQL現状|statcpp / R `p.adjust(method="holm")`|
|---|---|---|
|0.040|0.120|0.120|
|0.041|0.082|0.120|
|0.042|**0.042**|0.120|

最大の p値に対して 0.042 という**過小な補正値**を返す(本来は 0.120).
α = 0.05 で判定すると **偽陽性**になる. こちらの方が実害が大きい.

(上表は statcpp v0.4.0 を直接実行し, R の `p.adjust()` と一致することを確認済み)

さらに両者とも,利用者が `rank` と `total` を SQL 側で自力計算して渡す設計であり,
誤用しやすい. **集約関数化(D-2)すれば,正しさと使いやすさが同時に解決する.**

### 6-3. `stat_bonferroni` — 結果は一致するが再実装

[src/ext_funcs.cpp:2621](../src/ext_funcs.cpp#L2621). `min(p*m, 1.0)` は
statcpp と同式なので結果は正しい. ただし D-2 で集約化すれば `n` を手渡す必要がなくなる.

### 6-4. `stat_boxcox` / `stat_logarithmic_mean` — 結果は一致するが再実装

[src/ext_funcs.cpp:2731](../src/ext_funcs.cpp#L2731),
[src/ext_funcs.cpp:3079](../src/ext_funcs.cpp#L3079).
式は statcpp と同一で結果は正しい. statcpp 呼び出しに統一すれば,
将来 statcpp 側の改良(数値安定性など)が自動的に反映される.

## 7. 推奨アクション(優先度順)

|優先|内容|対象数|工数感|状態|
|---|---|--:|---|---|
|**1**|**§6-1, 6-2 の多重比較補正のバグ修正**(ウィンドウ関数化して statcpp に委譲)|3|小|**完了**|
|**2**|D-4 グループ列パターン(levene, bartlett, kruskal_wallis, 事後検定4種)|9|小|**完了**|
|**3**|D-1 スカラー関数(特に `set_seed` による再現性確保)|8|小|未着手|
|**4**|D-3, D-5, D-6(2列/3列集約, two_way_anova, ANCOVA)|16|中|未着手|
|**5**|**`VariadicColumnAggregate` 基盤 + 重回帰**|8|中|未着手|
|**6**|GLM(モデルJSON設計含む)|12|大|未着手|
|**7**|multivariate, model_selection, clustering, missing_data|36|大|未着手|

優先1〜4 だけで **36個追加 + バグ2件修正**, 249 → 285関数 になり,
既存インフラのみで完結する.
優先5以降が可変長列インフラを要する本丸.

## 8. 数値まとめ

現状(優先1・2 完了時点): SQL 関数 258個, statcpp から 257関数を利用.

```text
statcpp 公開関数            384
├─ SQL から利用可能         257  (66.9%)
└─ 未使用                   127
   ├─ A 実質ギャップなし     16  → 対応不要
   ├─ B 数値ユーティリティ   15  → 対応不要 (kahan_sum のみ検討)
   ├─ C SQL で代替可能       17  → 対応不要
   ├─ D 既存インフラで可能   27  → 優先対応 (36個中 9個 完了)
   └─ E 行列入力が必要       57  → 新インフラ必須

実質カバレッジ = 257 / (384 - 16 - 15 - 17) = 257 / 336 = 76.5%
  (分析時点は 248 / 336 = 73.8%. 優先1・2 で +9)
D 完了時                    = 284 / 336 = 84.5%
D + E 完了時                = 341 / 336 → 100%
```
