# SQLite3 Loadable Extension Guide

SQLite3のランタイム拡張 (Loadable Extension) の実装ガイド。
本プロジェクトでは C++ (`ext_funcs.cpp`) で拡張ライブラリを実装している。

---

## 目次

1. [Loadable Extension とは](#1-loadable-extension-とは)
2. [プロジェクト構成](#2-プロジェクト構成)
3. [ビルドと実行](#3-ビルドと実行)
4. [Loadable Extension の仕組み](#4-loadable-extension-の仕組み)
5. [拡張関数の実装パターン](#5-拡張関数の実装パターン)
6. [SQLite C API リファレンス](#6-sqlite-c-api-リファレンス)
7. [C++版の実装テクニック](#7-c版の実装テクニック)
8. [ダイナミックライブラリとしての考慮事項](#8-ダイナミックライブラリとしての考慮事項)

---

## 1. Loadable Extension とは

### 概要

SQLite3 Loadable Extension は、SQLite にカスタム機能を**ランタイムで動的に追加**する仕組みである。
C++ で記述したダイナミックライブラリ (`.dylib` / `.so` / `.dll`) を
`sqlite3_load_extension()` で読み込むことで、SQLite 本体を再コンパイルすることなく機能を拡張できる。

### できること

| 機能 | 説明 | 例 |
|---|---|---|
| **スカラー関数** | 入力値に対して1つの値を返す関数 | `square(x)`, `upper(s)` |
| **集約関数** | 複数行を集計して1つの値を返す関数 | `my_sum(x)`, `sha256_agg(s)` |
| **ウィンドウ関数** | OVER句と組み合わせる集約関数 (SQLite 3.25+) | カスタム移動平均 |
| **照合順序 (Collation)** | 文字列の比較ルールを定義する | 日本語ソート順 |
| **仮想テーブル (Virtual Table)** | 外部データソースをテーブルとして公開する | CSV, JSON, REST API |
| **VFS (Virtual File System)** | ファイルI/O層をカスタマイズする | 暗号化, メモリFS |
| **既存関数の上書き** | 組み込み関数を独自実装で置き換える | `upper()` の多言語対応化 |
| **複数関数の一括登録** | 1つの拡張から複数の関数をまとめて登録 | 本プロジェクトの実装 |

### できないこと

| 制約 | 説明 |
|---|---|
| **SQL 構文の変更** | 新しい SQL キーワードや構文を追加することはできない |
| **ストレージエンジンの変更** | SQLite の内部的な B-Tree 構造やページフォーマットは変更できない |
| **トリガーやビューの自動作成** | DDL を暗黙的に実行する仕組みはない (明示的な `sqlite3_exec` は可能) |
| **マルチスレッドの保証** | SQLite のスレッドモードに依存する。拡張側で独自にスレッドを作成すべきではない |
| **永続的な自動ロード** | DB ファイルに拡張情報は保存されない。接続のたびにロードが必要 |
| **サンドボックス実行** | 拡張はホストプロセスと同じ権限で動作する。任意のシステムコールを実行可能 |
| **他言語からの直接記述** | C/C++ の関数ポインタインターフェイスが必須。他言語は FFI 経由になる |

### セキュリティに関する注意

- 拡張はホストプロセスと**同一のアドレス空間**で実行されるため、バグや悪意あるコードはプロセス全体に影響する
- `sqlite3_enable_load_extension(db, 1)` を呼ばない限り、拡張ロードは**デフォルトで無効**
- SQL 文中の `load_extension()` 関数はさらに制限が厳しく、`sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL)` で別途有効化が必要
- 信頼できない拡張をロードしてはならない

---

## 2. プロジェクト構成

```text
sqlite3StatisticalLibrary/
├── CMakeLists.txt          # ビルド設定
├── README.md
├── cmake/
│   ├── sqlite3.cmake       # SQLite3 のダウンロード設定
│   └── statcpp.cmake       # statcpp のダウンロード設定
├── doc/                    # ドキュメント
├── download/               # ダウンロードされた依存ライブラリ
│   ├── sqlite3/            # SQLite3 ソース
│   └── statcpp/            # statcpp ヘッダーオンリーライブラリ
└── src/
    ├── main.cpp             # 拡張をロードしてテストする C++ プログラム
    ├── ext_funcs.cpp        # 拡張ライブラリ (C++) — 統計関数の実装
    └── include/             # （現在空）
```

### 外部ライブラリ

#### statcpp

- **概要**: C++ ヘッダーオンリーの統計計算ライブラリ
- **配置先**: `download/statcpp/statcpp-install/include/statcpp/`
- **利用方法**: CMake の `statcpp.cmake` により自動ダウンロード・配置される

本プロジェクトでは `ext_funcs.cpp` で `#include <statcpp/statcpp.hpp>` としてインクルードし、
統計計算のロジック部分を statcpp に委譲している。

### 提供する SQL 関数

#### 基本集約関数（24関数）

| 関数 | 引数 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_mean(col)` | 数値 1 | 数値 | 算術平均 |
| `stat_median(col)` | 数値 1 | 数値 | 中央値 |
| `stat_mode(col)` | 数値 1 | 数値 | 最頻値（複数ある場合は最小値） |
| `stat_geometric_mean(col)` | 数値 1 | 数値 | 幾何平均 |
| `stat_harmonic_mean(col)` | 数値 1 | 数値 | 調和平均 |
| `stat_range(col)` | 数値 1 | 数値 | 範囲（最大値 − 最小値） |
| `stat_var(col)` | 数値 1 | 数値 | 分散（母分散, ddof=0） |
| `stat_population_variance(col)` | 数値 1 | 数値 | 母分散 |
| `stat_sample_variance(col)` | 数値 1 | 数値 | 標本分散（不偏分散） |
| `stat_stdev(col)` | 数値 1 | 数値 | 標準偏差（母, ddof=0） |
| `stat_population_stddev(col)` | 数値 1 | 数値 | 母標準偏差 |
| `stat_sample_stddev(col)` | 数値 1 | 数値 | 標本標準偏差 |
| `stat_cv(col)` | 数値 1 | 数値 | 変動係数 |
| `stat_iqr(col)` | 数値 1 | 数値 | 四分位範囲 |
| `stat_mad_mean(col)` | 数値 1 | 数値 | 平均偏差 |
| `stat_geometric_stddev(col)` | 数値 1 | 数値 | 幾何標準偏差 |
| `stat_population_skewness(col)` | 数値 1 | 数値 | 母歪度 |
| `stat_skewness(col)` | 数値 1 | 数値 | 標本歪度 |
| `stat_population_kurtosis(col)` | 数値 1 | 数値 | 母尖度（超過尖度） |
| `stat_kurtosis(col)` | 数値 1 | 数値 | 標本尖度（超過尖度） |
| `stat_se(col)` | 数値 1 | 数値 | 標準誤差 |
| `stat_mad(col)` | 数値 1 | 数値 | 中央絶対偏差 (MAD) |
| `stat_mad_scaled(col)` | 数値 1 | 数値 | スケーリングされた MAD |
| `stat_hodges_lehmann(col)` | 数値 1 | 数値 | Hodges-Lehmann 推定量 |

#### パラメータ付き集約関数（20関数）

| 関数 | 引数 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_trimmed_mean(col, proportion)` | 数値, 数値 | 数値 | トリム平均 |
| `stat_quartile(col)` | 数値 1 | JSON | 四分位数 (Q1,Q2,Q3) |
| `stat_percentile(col, p)` | 数値, 数値 | 数値 | パーセンタイル |
| `stat_z_test(col, mu0, sigma)` | 数値, 数値, 数値 | JSON | 1標本 z 検定 |
| `stat_t_test(col, mu0)` | 数値, 数値 | JSON | 1標本 t 検定 |
| `stat_chisq_gof_uniform(col)` | 数値 1 | JSON | カイ二乗適合度検定（均等） |
| `stat_shapiro_wilk(col)` | 数値 1 | JSON | Shapiro-Wilk 検定 |
| `stat_ks_test(col)` | 数値 1 | JSON | Lilliefors 検定（正規性） |
| `stat_wilcoxon(col, mu0)` | 数値, 数値 | JSON | Wilcoxon 符号付順位検定 |
| `stat_ci_mean(col, confidence)` | 数値, 数値 | JSON | 平均の信頼区間（t 分布） |
| `stat_ci_mean_z(col, sigma, confidence)` | 数値, 数値, 数値 | JSON | 平均の信頼区間（z 分布） |
| `stat_ci_var(col, confidence)` | 数値, 数値 | JSON | 分散の信頼区間 |
| `stat_moe_mean(col, confidence)` | 数値, 数値 | 数値 | 平均の誤差マージン |
| `stat_cohens_d(col, mu0)` | 数値, 数値 | 数値 | Cohen's d（1標本） |
| `stat_hedges_g(col, mu0)` | 数値, 数値 | 数値 | Hedges' g（1標本） |
| `stat_acf_lag(col, lag)` | 数値, 数値 | 数値 | 自己相関係数 |
| `stat_biweight_midvar(col, c)` | 数値, 数値 | 数値 | Biweight Midvariance |
| `stat_bootstrap_mean(col, n)` | 数値, 数値 | JSON | 平均のブートストラップ |
| `stat_bootstrap_median(col, n)` | 数値, 数値 | JSON | 中央値のブートストラップ |
| `stat_bootstrap_stddev(col, n)` | 数値, 数値 | JSON | 標準偏差のブートストラップ |

#### 2カラム集約関数（27関数）

| 関数 | 引数 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_population_covariance(x, y)` | 数値, 数値 | 数値 | 母共分散 |
| `stat_covariance(x, y)` | 数値, 数値 | 数値 | 標本共分散 |
| `stat_pearson_r(x, y)` | 数値, 数値 | 数値 | ピアソン相関係数 |
| `stat_spearman_r(x, y)` | 数値, 数値 | 数値 | スピアマン順位相関 |
| `stat_kendall_tau(x, y)` | 数値, 数値 | 数値 | ケンドール順位相関 |
| `stat_weighted_covariance(val, wt)` | 数値, 数値 | 数値 | 重み付き共分散 |
| `stat_weighted_mean(val, wt)` | 数値, 数値 | 数値 | 重み付き平均 |
| `stat_weighted_harmonic_mean(val, wt)` | 数値, 数値 | 数値 | 重み付き調和平均 |
| `stat_weighted_variance(val, wt)` | 数値, 数値 | 数値 | 重み付き分散 |
| `stat_weighted_stddev(val, wt)` | 数値, 数値 | 数値 | 重み付き標準偏差 |
| `stat_weighted_median(val, wt)` | 数値, 数値 | 数値 | 重み付き中央値 |
| `stat_weighted_percentile(val, wt, p)` | 数値, 数値, 数値 | 数値 | 重み付きパーセンタイル |
| `stat_simple_regression(x, y)` | 数値, 数値 | JSON | 単回帰分析 |
| `stat_r_squared(actual, pred)` | 数値, 数値 | 数値 | 決定係数 R² |
| `stat_adjusted_r_squared(actual, pred)` | 数値, 数値 | 数値 | 自由度調整済み R² |
| `stat_t_test_paired(x, y)` | 数値, 数値 | JSON | 対応のある t 検定 |
| `stat_chisq_gof(obs, exp)` | 数値, 数値 | JSON | カイ二乗適合度検定 |
| `stat_mae(actual, pred)` | 数値, 数値 | 数値 | 平均絶対誤差 |
| `stat_mse(actual, pred)` | 数値, 数値 | 数値 | 平均二乗誤差 |
| `stat_rmse(actual, pred)` | 数値, 数値 | 数値 | RMSE |
| `stat_mape(actual, pred)` | 数値, 数値 | 数値 | 平均絶対パーセント誤差 |
| `stat_euclidean_dist(a, b)` | 数値, 数値 | 数値 | ユークリッド距離 |
| `stat_manhattan_dist(a, b)` | 数値, 数値 | 数値 | マンハッタン距離 |
| `stat_cosine_sim(a, b)` | 数値, 数値 | 数値 | コサイン類似度 |
| `stat_cosine_dist(a, b)` | 数値, 数値 | 数値 | コサイン距離 |
| `stat_minkowski_dist(a, b, p)` | 数値, 数値, 数値 | 数値 | ミンコフスキー距離 |
| `stat_chebyshev_dist(a, b)` | 数値, 数値 | 数値 | チェビシェフ距離 |

#### ウィンドウ関数（23関数）

ウィンドウ関数は全行スキャン型で実装されている。各行に対して1つの値を返す。`OVER()` 句なしでも使用可能。

| 関数 | 引数 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_rolling_mean(col, window)` | 数値, 整数 | 数値/行 | ローリング平均 |
| `stat_rolling_std(col, window)` | 数値, 整数 | 数値/行 | ローリング標準偏差 |
| `stat_rolling_min(col, window)` | 数値, 整数 | 数値/行 | ローリング最小値 |
| `stat_rolling_max(col, window)` | 数値, 整数 | 数値/行 | ローリング最大値 |
| `stat_rolling_sum(col, window)` | 数値, 整数 | 数値/行 | ローリング合計 |
| `stat_moving_avg(col, window)` | 数値, 整数 | 数値/行 | 単純移動平均 |
| `stat_ema(col, span)` | 数値, 整数 | 数値/行 | 指数移動平均（α=2/(span+1)） |
| `stat_rank(col)` | 数値 1 | 数値/行 | 順位変換 |
| `stat_fillna_mean(col)` | 数値 1 | 数値/行 | 平均値補完 |
| `stat_fillna_median(col)` | 数値 1 | 数値/行 | 中央値補完 |
| `stat_fillna_ffill(col)` | 数値 1 | 数値/行 | 前方補完 |
| `stat_fillna_bfill(col)` | 数値 1 | 数値/行 | 後方補完 |
| `stat_fillna_interp(col)` | 数値 1 | 数値/行 | 線形補間 |
| `stat_label_encode(col)` | 数値 1 | 数値/行 | ラベルエンコーディング |
| `stat_bin_width(col, n_bins)` | 数値, 整数 | 数値/行 | 等幅ビニング |
| `stat_bin_freq(col, n_bins)` | 数値, 整数 | 数値/行 | 等頻度ビニング |
| `stat_lag(col, k)` | 数値, 整数 | 数値/行 | ラグ（k 行前の値） |
| `stat_diff(col, order)` | 数値, 整数 | 数値/行 | 差分 |
| `stat_seasonal_diff(col, period)` | 数値, 整数 | 数値/行 | 季節差分 |
| `stat_outliers_iqr(col)` | 数値 1 | 数値/行 | 外れ値検出（IQR 法、1.0/0.0） |
| `stat_outliers_zscore(col)` | 数値 1 | 数値/行 | 外れ値検出（Z スコア法） |
| `stat_outliers_mzscore(col)` | 数値 1 | 数値/行 | 外れ値検出（修正 Z スコア法） |
| `stat_winsorize(col, pct)` | 数値, 整数 | 数値/行 | ウィンザライズ |

#### 複合集約関数（32関数）

結果がJSON（複数値）を返す集約関数、または2標本検定・生存時間解析などの複合集約関数。

| 関数 | 引数 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_modes(col)` | 数値 1 | JSON | 最頻値（すべて） |
| `stat_five_number_summary(col)` | 数値 1 | JSON | 五数要約 (min, Q1, median, Q3, max) |
| `stat_frequency_table(col)` | 数値 1 | JSON | 度数表 |
| `stat_frequency_count(col)` | 数値 1 | JSON | 各値の度数 |
| `stat_relative_frequency(col)` | 数値 1 | JSON | 相対度数 |
| `stat_cumulative_frequency(col)` | 数値 1 | JSON | 累積度数 |
| `stat_cumulative_relative_frequency(col)` | 数値 1 | JSON | 累積相対度数 |
| `stat_t_test2(grp1, grp2)` | 数値, 数値 | JSON | 2標本 t 検定（プール分散） |
| `stat_t_test_welch(grp1, grp2)` | 数値, 数値 | JSON | Welch t 検定 |
| `stat_chisq_independence(col1, col2)` | 数値, 数値 | JSON | カイ二乗独立性検定 |
| `stat_f_test(grp1, grp2)` | 数値, 数値 | JSON | F 検定（分散比較） |
| `stat_mann_whitney(grp1, grp2)` | 数値, 数値 | JSON | Mann-Whitney U 検定 |
| `stat_anova1(val, grp)` | 数値, 数値 | JSON | 一元配置分散分析 |
| `stat_contingency_table(col1, col2)` | 数値, 数値 | JSON | 分割表作成 |
| `stat_cohens_d2(grp1, grp2)` | 数値, 数値 | 数値 | Cohen's d（2標本） |
| `stat_hedges_g2(grp1, grp2)` | 数値, 数値 | 数値 | Hedges' g（2標本） |
| `stat_glass_delta(grp1, grp2)` | 数値, 数値 | 数値 | Glass's Delta |
| `stat_ci_mean_diff(grp1, grp2)` | 数値, 数値 | JSON | 2標本平均差の信頼区間 |
| `stat_ci_mean_diff_welch(grp1, grp2)` | 数値, 数値 | JSON | 2標本平均差の信頼区間（Welch） |
| `stat_kaplan_meier(time, event)` | 数値, 数値 | JSON | Kaplan-Meier 生存曲線 |
| `stat_nelson_aalen(time, event)` | 数値, 数値 | JSON | Nelson-Aalen 累積ハザード推定 |
| `stat_logrank(time, event, grp)` | 数値, 数値, 数値 | JSON | Log-rank 検定 |
| `stat_bootstrap(col, n)` | 数値, 数値 | JSON | 汎用ブートストラップ推定 |
| `stat_bootstrap_bca(col, n)` | 数値, 数値 | JSON | BCa ブートストラップ |
| `stat_bootstrap_sample(col)` | 数値 1 | JSON | ブートストラップサンプル生成 |
| `stat_permutation_test2(grp1, grp2)` | 数値, 数値 | JSON | 2標本置換検定 |
| `stat_permutation_paired(x, y)` | 数値, 数値 | JSON | 対応のある置換検定 |
| `stat_permutation_corr(x, y)` | 数値, 数値 | JSON | 相関の置換検定 |
| `stat_acf(col, max_lag)` | 数値, 数値 | JSON | 自己相関関数 |
| `stat_pacf(col, max_lag)` | 数値, 数値 | JSON | 偏自己相関関数 |
| `stat_sample_replace(col, n)` | 数値, 数値 | JSON | 復元抽出 |
| `stat_sample(col, n)` | 数値, 数値 | JSON | 非復元抽出 |

#### スカラー関数 — 検定補助（40関数）

パラメータのみで計算が完結するスカラー関数。検定の p 値計算や信頼区間の導出に必須。

| 関数 | 引数 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_normal_pdf(x [,mu, sigma])` | 1〜3 | 数値 | 正規分布 PDF |
| `stat_normal_cdf(x [,mu, sigma])` | 1〜3 | 数値 | 正規分布 CDF |
| `stat_normal_quantile(p [,mu, sigma])` | 1〜3 | 数値 | 正規分布分位点 |
| `stat_normal_rand([mu, sigma])` | 0〜2 | 数値 | 正規分布乱数 |
| `stat_chisq_pdf(x, df)` | 2 | 数値 | カイ二乗分布 PDF |
| `stat_chisq_cdf(x, df)` | 2 | 数値 | カイ二乗分布 CDF |
| `stat_chisq_quantile(p, df)` | 2 | 数値 | カイ二乗分布分位点 |
| `stat_chisq_rand(df)` | 1 | 数値 | カイ二乗分布乱数 |
| `stat_t_pdf(x, df)` | 2 | 数値 | t 分布 PDF |
| `stat_t_cdf(x, df)` | 2 | 数値 | t 分布 CDF |
| `stat_t_quantile(p, df)` | 2 | 数値 | t 分布分位点 |
| `stat_t_rand(df)` | 1 | 数値 | t 分布乱数 |
| `stat_f_pdf(x, df1, df2)` | 3 | 数値 | F 分布 PDF |
| `stat_f_cdf(x, df1, df2)` | 3 | 数値 | F 分布 CDF |
| `stat_f_quantile(p, df1, df2)` | 3 | 数値 | F 分布分位点 |
| `stat_f_rand(df1, df2)` | 2 | 数値 | F 分布乱数 |
| `stat_betainc(a, b, x)` | 3 | 数値 | 正則化不完全ベータ関数 |
| `stat_betaincinv(a, b, p)` | 3 | 数値 | 正則化不完全ベータ逆関数 |
| `stat_norm_cdf(x)` | 1 | 数値 | 標準正規分布 CDF |
| `stat_norm_quantile(p)` | 1 | 数値 | 標準正規分布逆 CDF |
| `stat_gammainc_lower(a, x)` | 2 | 数値 | 下側不完全ガンマ関数 |
| `stat_gammainc_upper(a, x)` | 2 | 数値 | 上側不完全ガンマ関数 |
| `stat_gammainc_lower_inv(a, p)` | 2 | 数値 | 下側不完全ガンマ逆関数 |
| `stat_z_test_prop(x, n, p0)` | 3 | JSON | 1標本比率 z 検定 |
| `stat_z_test_prop2(x1, n1, x2, n2)` | 4 | JSON | 2標本比率 z 検定 |
| `stat_bonferroni(p, m)` | 2 | 数値 | Bonferroni 補正 |
| `stat_bh_correction(p, rank, total)` | 3 | 数値 | Benjamini-Hochberg 補正 |
| `stat_holm_correction(p, rank, total)` | 3 | 数値 | Holm 補正 |
| `stat_fisher_exact(a, b, c, d)` | 4 | JSON | Fisher 正確確率検定 |
| `stat_odds_ratio(a, b, c, d)` | 4 | 数値 | オッズ比 |
| `stat_relative_risk(a, b, c, d)` | 4 | 数値 | 相対リスク |
| `stat_risk_difference(a, b, c, d)` | 4 | 数値 | リスク差 |
| `stat_nnt(a, b, c, d)` | 4 | 数値 | 治療必要数 (NNT) |
| `stat_ci_prop(x, n [,confidence])` | 2〜3 | JSON | 比率の信頼区間（Wald 法） |
| `stat_ci_prop_wilson(x, n [,confidence])` | 2〜3 | JSON | 比率の信頼区間（Wilson 法） |
| `stat_ci_prop_diff(x1, n1, x2, n2 [,conf])` | 4〜5 | JSON | 2標本比率差の信頼区間 |
| `stat_aic(log_likelihood, k)` | 2 | 数値 | AIC |
| `stat_aicc(log_likelihood, n, k)` | 3 | 数値 | 補正 AIC (AICc) |
| `stat_bic(log_likelihood, n, k)` | 3 | 数値 | BIC |
| `stat_boxcox(x, lambda)` | 2 | 数値 | Box-Cox 変換 |

#### スカラー関数 — 分布・変換（83関数）

パラメータのみで計算が完結する電卓的スカラー関数。分布関数、効果量変換、検出力分析など。

| 関数 | 引数 | 戻り値 | 説明 |
|---|---|---|---|
| `stat_uniform_pdf(x [,a, b])` | 1〜3 | 数値 | 一様分布 PDF |
| `stat_uniform_cdf(x [,a, b])` | 1〜3 | 数値 | 一様分布 CDF |
| `stat_uniform_quantile(p [,a, b])` | 1〜3 | 数値 | 一様分布分位点 |
| `stat_uniform_rand([a, b])` | 0〜2 | 数値 | 一様分布乱数 |
| `stat_exponential_pdf(x [,lambda])` | 1〜2 | 数値 | 指数分布 PDF |
| `stat_exponential_cdf(x [,lambda])` | 1〜2 | 数値 | 指数分布 CDF |
| `stat_exponential_quantile(p [,lambda])` | 1〜2 | 数値 | 指数分布分位点 |
| `stat_exponential_rand([lambda])` | 0〜1 | 数値 | 指数分布乱数 |
| `stat_gamma_pdf(x, shape, scale)` | 3 | 数値 | ガンマ分布 PDF |
| `stat_gamma_cdf(x, shape, scale)` | 3 | 数値 | ガンマ分布 CDF |
| `stat_gamma_quantile(p, shape, scale)` | 3 | 数値 | ガンマ分布分位点 |
| `stat_gamma_rand(shape, scale)` | 2 | 数値 | ガンマ分布乱数 |
| `stat_beta_pdf(x, alpha, beta)` | 3 | 数値 | ベータ分布 PDF |
| `stat_beta_cdf(x, alpha, beta)` | 3 | 数値 | ベータ分布 CDF |
| `stat_beta_quantile(p, alpha, beta)` | 3 | 数値 | ベータ分布分位点 |
| `stat_beta_rand(alpha, beta)` | 2 | 数値 | ベータ分布乱数 |
| `stat_lognormal_pdf(x [,mu, sigma])` | 1〜3 | 数値 | 対数正規分布 PDF |
| `stat_lognormal_cdf(x [,mu, sigma])` | 1〜3 | 数値 | 対数正規分布 CDF |
| `stat_lognormal_quantile(p [,mu, sigma])` | 1〜3 | 数値 | 対数正規分布分位点 |
| `stat_lognormal_rand([mu, sigma])` | 0〜2 | 数値 | 対数正規分布乱数 |
| `stat_weibull_pdf(x, shape, scale)` | 3 | 数値 | ワイブル分布 PDF |
| `stat_weibull_cdf(x, shape, scale)` | 3 | 数値 | ワイブル分布 CDF |
| `stat_weibull_quantile(p, shape, scale)` | 3 | 数値 | ワイブル分布分位点 |
| `stat_weibull_rand(shape, scale)` | 2 | 数値 | ワイブル分布乱数 |
| `stat_binomial_pmf(k, n, p)` | 3 | 数値 | 二項分布 PMF |
| `stat_binomial_cdf(k, n, p)` | 3 | 数値 | 二項分布 CDF |
| `stat_binomial_quantile(q, n, p)` | 3 | 数値 | 二項分布分位点 |
| `stat_binomial_rand(n, p)` | 2 | 数値 | 二項分布乱数 |
| `stat_poisson_pmf(k, lambda)` | 2 | 数値 | ポアソン分布 PMF |
| `stat_poisson_cdf(k, lambda)` | 2 | 数値 | ポアソン分布 CDF |
| `stat_poisson_quantile(q, lambda)` | 2 | 数値 | ポアソン分布分位点 |
| `stat_poisson_rand(lambda)` | 1 | 数値 | ポアソン分布乱数 |
| `stat_geometric_pmf(k, p)` | 2 | 数値 | 幾何分布 PMF |
| `stat_geometric_cdf(k, p)` | 2 | 数値 | 幾何分布 CDF |
| `stat_geometric_quantile(q, p)` | 2 | 数値 | 幾何分布分位点 |
| `stat_geometric_rand(p)` | 1 | 数値 | 幾何分布乱数 |
| `stat_nbinom_pmf(k, r, p)` | 3 | 数値 | 負の二項分布 PMF |
| `stat_nbinom_cdf(k, r, p)` | 3 | 数値 | 負の二項分布 CDF |
| `stat_nbinom_quantile(q, r, p)` | 3 | 数値 | 負の二項分布分位点 |
| `stat_nbinom_rand(r, p)` | 2 | 数値 | 負の二項分布乱数 |
| `stat_hypergeom_pmf(k, N, K, n)` | 4 | 数値 | 超幾何分布 PMF |
| `stat_hypergeom_cdf(k, N, K, n)` | 4 | 数値 | 超幾何分布 CDF |
| `stat_hypergeom_quantile(q, N, K, n)` | 4 | 数値 | 超幾何分布分位点 |
| `stat_hypergeom_rand(N, K, n)` | 3 | 数値 | 超幾何分布乱数 |
| `stat_bernoulli_pmf(k, p)` | 2 | 数値 | ベルヌーイ分布 PMF |
| `stat_bernoulli_cdf(k, p)` | 2 | 数値 | ベルヌーイ分布 CDF |
| `stat_bernoulli_quantile(q, p)` | 2 | 数値 | ベルヌーイ分布分位点 |
| `stat_bernoulli_rand(p)` | 1 | 数値 | ベルヌーイ分布乱数 |
| `stat_duniform_pmf(k, a, b)` | 3 | 数値 | 離散一様分布 PMF |
| `stat_duniform_cdf(k, a, b)` | 3 | 数値 | 離散一様分布 CDF |
| `stat_duniform_quantile(q, a, b)` | 3 | 数値 | 離散一様分布分位点 |
| `stat_duniform_rand(a, b)` | 2 | 数値 | 離散一様分布乱数 |
| `stat_binomial_coef(n, k)` | 2 | 整数 | 二項係数 |
| `stat_log_binomial_coef(n, k)` | 2 | 数値 | 対数二項係数 |
| `stat_log_factorial(n)` | 1 | 数値 | 対数階乗 |
| `stat_lgamma(x)` | 1 | 数値 | 対数ガンマ関数 |
| `stat_tgamma(x)` | 1 | 数値 | ガンマ関数 |
| `stat_beta_func(a, b)` | 2 | 数値 | ベータ関数 |
| `stat_lbeta(a, b)` | 2 | 数値 | 対数ベータ関数 |
| `stat_erf(x)` | 1 | 数値 | 誤差関数 |
| `stat_erfc(x)` | 1 | 数値 | 相補誤差関数 |
| `stat_logarithmic_mean(a, b)` | 2 | 数値 | 対数平均 |
| `stat_hedges_j(n)` | 1 | 数値 | Hedges 補正係数 |
| `stat_t_to_r(t, df)` | 2 | 数値 | t値 → 相関係数変換 |
| `stat_d_to_r(d)` | 1 | 数値 | Cohen's d → 相関係数変換 |
| `stat_r_to_d(r)` | 1 | 数値 | 相関係数 → Cohen's d 変換 |
| `stat_eta_squared_ef(ss_effect, ss_total)` | 2 | 数値 | イータ二乗（効果量） |
| `stat_partial_eta_sq(F, df1, df2)` | 3 | 数値 | 偏イータ二乗 |
| `stat_omega_squared_ef(ss_effect, ss_total, ms_error, df_effect)` | 4 | 数値 | オメガ二乗（効果量） |
| `stat_cohens_h(p1, p2)` | 2 | 数値 | Cohen's h（比率差） |
| `stat_interpret_d(d)` | 1 | テキスト | Cohen's d の解釈 |
| `stat_interpret_r(r)` | 1 | テキスト | 相関係数の解釈 |
| `stat_interpret_eta2(eta2)` | 1 | テキスト | イータ二乗の解釈 |
| `stat_power_t1(d, n, alpha)` | 3 | 数値 | 1標本 t 検定の検出力 |
| `stat_n_t1(d, power, alpha)` | 3 | 数値 | 1標本 t 検定の必要サンプルサイズ |
| `stat_power_t2(d, n1, n2, alpha)` | 4 | 数値 | 2標本 t 検定の検出力 |
| `stat_n_t2(d, power, alpha)` | 3 | 数値 | 2標本 t 検定の必要サンプルサイズ |
| `stat_power_prop(p1, p2, n, alpha)` | 4 | 数値 | 比率検定の検出力 |
| `stat_n_prop(p1, p2, power, alpha)` | 4 | 数値 | 比率検定の必要サンプルサイズ |
| `stat_moe_prop(x, n [,confidence])` | 2〜3 | 数値 | 比率の誤差マージン |
| `stat_moe_prop_worst(n [,confidence])` | 1〜2 | 数値 | 最悪ケースの誤差マージン |
| `stat_n_moe_prop(moe [,confidence [,p]])` | 1〜3 | 数値 | 比率推定のサンプルサイズ |
| `stat_n_moe_mean(moe, sigma [,confidence])` | 2〜3 | 数値 | 平均推定のサンプルサイズ |

すべての集約関数は `NULL` 入力行を無視する（SQLite3 の集約関数の慣例に準拠）。
ウィンドウ関数は `NULL` 入力を追跡し、結果で `NULL` を返すか補完する（関数依存）。
スカラー関数は引数に `NULL` が含まれる場合 `NULL` を返す。
すべての行が `NULL` または結果セットが空の場合は `NULL` を返す。
計算結果が `NaN` または `Inf` の場合は `NULL` を返す。
乱数関数（`*_rand`）は非決定的（`SQLITE_DETERMINISTIC` フラグなし）で登録される。

---

## 3. ビルドと実行

```bash
# ビルド
cmake -B build
cmake --build build

# 実行 (C++ テストプログラム)
./build/a.out
```

### sqlite3 CLI からの利用

ビルドした拡張ライブラリは、`sqlite3` コマンドラインツールからも利用できる。

#### 前提条件

`sqlite3` CLI 自体が `SQLITE_ENABLE_LOAD_EXTENSION` 付きでビルドされている必要がある。
macOS 標準の `sqlite3` は有効になっている。

#### 方法1: `.load` ドットコマンド（推奨）

```bash
cd build
sqlite3 test.db
```

```sql
sqlite> .load ./ext_funcs sqlite3_ext_funcs_init
sqlite> SELECT stat_mean(val), stat_median(val) FROM data;
5.5|5.5
sqlite> SELECT stat_sample_stddev(val) FROM data;
3.02765035409749
sqlite> SELECT category, stat_mean(score) FROM exam GROUP BY category;
```

`.load` の第1引数がライブラリパス、第2引数がエントリポイント名。
拡張子 (`.dylib`) は省略可能（SQLite が自動補完する）。

#### 方法2: `load_extension()` SQL 関数

```sql
SELECT load_extension('./ext_funcs', 'sqlite3_ext_funcs_init');
SELECT square(7);
```

この方法は `.load` より制限が厳しく、
`sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL)` で
別途有効化されている必要がある。通常は `.load` を使う方が確実。

#### エントリポイント名の指定が必要な理由

SQLite はファイル名からエントリポイント名を自動推定するが、
`ext_funcs` → `sqlite3_extfuncs_init` のように `_` が除去される。
実際のエントリポイント名は `sqlite3_ext_funcs_init` なので、
第2引数で明示的に指定する必要がある。

---

## 4. Loadable Extension の仕組み

### 全体の流れ

```text
1. main.cpp: sqlite3_enable_load_extension(db, 1)
   └─ 拡張ロードを有効化

2. main.cpp: sqlite3_load_extension(db, "ext_funcs.dylib", "sqlite3_ext_funcs_init", ...)
   └─ dyld がダイナミックライブラリをロード
   └─ エントリポイント関数を呼び出す

3. ext_funcs: sqlite3_ext_funcs_init(db, ...)
   └─ SQLITE_EXTENSION_INIT2(pApi) で API ルーチンを初期化
   └─ sqlite3_create_function() でカスタム関数を登録

4. main.cpp: SELECT square(5), upper('hello'), sha256_agg(content), ...
   └─ SQLite が登録済みのコールバック関数を呼び出す
```

### エントリポイントの命名規則

SQLite はライブラリファイル名からエントリポイント名を推測する:

```text
ext_funcs.dylib → sqlite3_ext_funcs_init
```

ファイル名に `-` や `.` が含まれる場合は自動推測が失敗するため、
`sqlite3_load_extension()` の第3引数で明示的に指定する。

### 必須マクロ

```cpp
#include <sqlite3ext.h>

SQLITE_EXTENSION_INIT1              // グローバルスコープに1回
// ...
extern "C"
int sqlite3_ext_funcs_init(...) {
    SQLITE_EXTENSION_INIT2(pApi);   // エントリポイントの先頭に1回
    // ...
}
```

- `SQLITE_EXTENSION_INIT1`: SQLite API のグローバルポインタを宣言する
- `SQLITE_EXTENSION_INIT2(pApi)`: ホストアプリから渡された API ポインタを設定する
- `extern "C"`: C++ でコンパイルする場合、エントリポイントに C リンケージを指定する

これがないと `sqlite3_create_function` 等の API を呼び出せない。

### コンパイルフラグ

ホスト側 (SQLite 本体) で `SQLITE_ENABLE_LOAD_EXTENSION` を有効にする必要がある:

```cmake
target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_LOAD_EXTENSION)
```

---

## 5. 拡張関数の実装パターン

### 5.1. 関数登録 (sqlite3_create_function)

```cpp
int sqlite3_create_function(
    sqlite3 *db,              // データベース接続
    const char *zFunctionName,// SQL関数名
    int nArg,                 // 引数の数 (-1 で可変長)
    int eTextRep,             // テキストエンコーディング + フラグ
    void *pApp,               // コールバックに渡すユーザデータ (任意)
    void (*xFunc)(...),       // スカラー関数コールバック
    void (*xStep)(...),       // 集約関数の1行ごとの処理 (nullptrならスカラー)
    void (*xFinal)(...)       // 集約関数の最終処理 (nullptrならスカラー)
);
```

#### eTextRep に指定するフラグ

| フラグ | 意味 |
|---|---|
| `SQLITE_UTF8` | UTF-8 テキスト (通常はこれ) |
| `SQLITE_UTF16` | UTF-16 テキスト |
| `SQLITE_DETERMINISTIC` | 同じ入力に対して常に同じ結果を返す (最適化ヒント) |
| `SQLITE_INNOCUOUS` | 副作用がなく安全 (SQLite 3.31+) |
| `SQLITE_DIRECTONLY` | SQL内からのみ呼び出し可能 (SQLite 3.30+) |

### 5.2. コールバック関数のシグネチャ

```cpp
void callback(sqlite3_context *ctx, int argc, sqlite3_value **argv);
```

- `ctx`: 結果の返却やエラー通知に使うコンテキスト
- `argc`: 引数の数
- `argv`: 引数の配列

### 5.3. 引数の取得 (sqlite3_value_*)

| 関数 | 戻り値の型 | 説明 |
|---|---|---|
| `sqlite3_value_type(v)` | `int` | 値の型を返す (下記の型定数) |
| `sqlite3_value_int(v)` | `int` | 32bit 整数として取得 |
| `sqlite3_value_int64(v)` | `sqlite3_int64` | 64bit 整数として取得 |
| `sqlite3_value_double(v)` | `double` | 浮動小数点として取得 |
| `sqlite3_value_text(v)` | `const unsigned char *` | UTF-8 文字列として取得 |
| `sqlite3_value_text16(v)` | `const void *` | UTF-16 文字列として取得 |
| `sqlite3_value_blob(v)` | `const void *` | BLOB データとして取得 |
| `sqlite3_value_bytes(v)` | `int` | text/blob のバイト数 |

#### 型定数 (sqlite3_value_type の戻り値)

| 定数 | 値 | 説明 |
|---|---|---|
| `SQLITE_INTEGER` | 1 | 整数 |
| `SQLITE_FLOAT` | 2 | 浮動小数点 |
| `SQLITE_TEXT` | 3 | テキスト |
| `SQLITE_BLOB` | 4 | バイナリ |
| `SQLITE_NULL` | 5 | NULL |

### 5.4. 結果の返却 (sqlite3_result_*)

#### スカラー値を返す

| 関数 | 返す型 | 対応する C++ 型 |
|---|---|---|
| `sqlite3_result_int(ctx, int)` | 32bit 整数 | `int` |
| `sqlite3_result_int64(ctx, sqlite3_int64)` | 64bit 整数 | `sqlite3_int64` |
| `sqlite3_result_double(ctx, double)` | 浮動小数点 | `double` |
| `sqlite3_result_text(ctx, str, len, destructor)` | UTF-8 文字列 | `const char *` |
| `sqlite3_result_text16(ctx, str, len, destructor)` | UTF-16 文字列 | `const void *` |
| `sqlite3_result_blob(ctx, data, len, destructor)` | バイナリデータ | `const void *` |
| `sqlite3_result_null(ctx)` | NULL | -- |

#### 特殊な返却

| 関数 | 用途 |
|---|---|
| `sqlite3_result_zeroblob(ctx, n)` | n バイトのゼロ埋め BLOB を返す |
| `sqlite3_result_value(ctx, sqlite3_value*)` | 入力値をそのまま返す (型を保持) |
| `sqlite3_result_pointer(ctx, ptr, type, destructor)` | ポインタを受け渡す (SQLite 3.20+) |
| `sqlite3_result_subtype(ctx, unsigned int)` | JSON 等のサブタイプ情報を付加する |

#### エラーを返す

| 関数 | 用途 |
|---|---|
| `sqlite3_result_error(ctx, msg, len)` | エラーメッセージを返す |
| `sqlite3_result_error_nomem(ctx)` | メモリ不足エラーを返す |
| `sqlite3_result_error_toobig(ctx)` | データが大きすぎるエラーを返す |
| `sqlite3_result_error_code(ctx, int)` | 任意のエラーコードを返す |

#### destructor 引数

`_text`, `_blob` 系の最後の引数に渡すメモリ管理の指定:

| 値 | 意味 |
|---|---|
| `SQLITE_STATIC` | データは呼び出し側が管理。SQLite はコピーしない |
| `SQLITE_TRANSIENT` | SQLite が内部でコピーを作成する。ローカル変数でも安全 |
| 関数ポインタ (例: `sqlite3_free`) | SQLite が使用後にデストラクタを呼ぶ |

### 5.5. メモリ管理

拡張内では標準の `malloc`/`free` ではなく SQLite 提供の関数を使うことが推奨される:

| 関数 | 説明 |
|---|---|
| `sqlite3_malloc(n)` | n バイトのメモリを確保 |
| `sqlite3_realloc(ptr, n)` | メモリを再確保 |
| `sqlite3_free(ptr)` | メモリを解放 |
| `sqlite3_mprintf(fmt, ...)` | printf 形式でメモリ確保付き文字列生成 |

`sqlite3_result_text` の destructor に `sqlite3_free` を渡すことで、
SQLite がバッファの解放を管理する:

```cpp
auto *result = static_cast<char *>(sqlite3_malloc(len + 1));
// ... result にデータを書き込む ...
sqlite3_result_text(ctx, result, len, sqlite3_free);
// → SQLite が result を使い終わったら sqlite3_free(result) を呼ぶ
```

### 5.6. 実装テンプレート

```cpp
static void my_func(sqlite3_context *ctx, int /*argc*/, sqlite3_value **argv) {
    // 1. NULL チェック
    if (sqlite3_value_type(argv[0]) == SQLITE_NULL) {
        sqlite3_result_null(ctx);
        return;
    }

    // 2. 引数の取得
    const char *input = reinterpret_cast<const char *>(sqlite3_value_text(argv[0]));
    int len = static_cast<int>(std::strlen(input));

    // 3. 処理
    auto *result = static_cast<char *>(sqlite3_malloc(len + 1));
    if (!result) {
        sqlite3_result_error_nomem(ctx);
        return;
    }
    // ... result を構築 ...

    // 4. 結果の返却
    sqlite3_result_text(ctx, result, len, sqlite3_free);
}
```

---

## 6. SQLite C API リファレンス

### 拡張のロードに関する API

| 関数 | 説明 |
|---|---|
| `sqlite3_enable_load_extension(db, onoff)` | 拡張ロードの有効/無効を切り替える |
| `sqlite3_load_extension(db, file, proc, errmsg)` | 拡張ライブラリをロードする |
| `sqlite3_auto_extension(xEntryPoint)` | 新しい DB 接続時に自動ロードする拡張を登録 |
| `sqlite3_cancel_auto_extension(xEntryPoint)` | 自動ロード登録を解除 |
| `sqlite3_reset_auto_extension()` | 全自動ロード登録を解除 |

### pApp (ユーザデータ) の活用

`sqlite3_create_function` の第5引数 `pApp` に任意のポインタを渡せる。
コールバック内で `sqlite3_user_data(ctx)` を使って取り出す:

```cpp
// 登録時
int multiplier = 10;
sqlite3_create_function(db, "scale", 1, SQLITE_UTF8, &multiplier,
                         scale_func, nullptr, nullptr);

// コールバック内
static void scale_func(sqlite3_context *ctx, int /*argc*/, sqlite3_value **argv) {
    auto *mul = static_cast<int *>(sqlite3_user_data(ctx));
    double x = sqlite3_value_double(argv[0]);
    sqlite3_result_double(ctx, x * (*mul));
}
```

### 集約関数の実装

集約関数 (SUM, AVG のような関数) を実装するには `xStep` と `xFinal` を使う。
本プロジェクトでは、ヒープ上に状態オブジェクトを確保し、`sqlite3_aggregate_context` にはポインタのみを格納するパターンを採用している:

```cpp
struct SingleColumnState {
    std::vector<double> values;
};

// xStep: 各行で呼ばれる（NULL 行は無視）
static void xStep(sqlite3_context *ctx, int /*argc*/, sqlite3_value **argv) {
    if (sqlite3_value_type(argv[0]) == SQLITE_NULL) return;
    auto **pp = static_cast<SingleColumnState**>(
        sqlite3_aggregate_context(ctx, sizeof(SingleColumnState*)));
    if (!pp) return;
    if (!*pp) *pp = new (std::nothrow) SingleColumnState();
    if (*pp) (*pp)->values.push_back(sqlite3_value_double(argv[0]));
}

// xFinal: 最後に1回呼ばれる
static void xFinal(sqlite3_context *ctx) {
    auto **pp = static_cast<SingleColumnState**>(
        sqlite3_aggregate_context(ctx, 0));
    if (!pp || !*pp || (*pp)->values.empty()) {
        sqlite3_result_null(ctx);
    } else {
        double result = statcpp::mean((*pp)->values.begin(), (*pp)->values.end());
        if (std::isnan(result) || std::isinf(result)) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_double(ctx, result);
        }
    }
    if (pp && *pp) { delete *pp; *pp = nullptr; }
}

// 登録 (xFunc = nullptr, xStep と xFinal を指定)
sqlite3_create_function_v2(db, "stat_mean", 1,
    SQLITE_UTF8 | SQLITE_DETERMINISTIC,
    nullptr, nullptr, xStep, xFinal, nullptr);
```

---

## 7. C++版の実装テクニック

### コールバックの3つの記述方式

本プロジェクトの `ext_funcs.cpp` では `sqlite3_create_function` に渡すコールバックを3つの方式で記述している:

#### 方法1: 通常の static 関数

```cpp
// 関数定義
static void reverse_func(sqlite3_context *ctx, int, sqlite3_value **argv) {
    // ...
}

// 登録
sqlite3_create_function(db, "reverse", 1, ..., reverse_func, nullptr, nullptr);
```

#### 方法2: auto 変数にラムダを代入

```cpp
// ラムダを名前付き変数に格納
static auto repeat_func = [](sqlite3_context *ctx, int, sqlite3_value **argv) {
    // ...
};

// 登録 — + で関数ポインタに変換して渡す
sqlite3_create_function(db, "repeat", 2, ..., +repeat_func, nullptr, nullptr);
```

#### 方法3: インラインラムダ

```cpp
// 登録時に直接記述
sqlite3_create_function(db, "is_palindrome", 1, ...,
    +[](sqlite3_context *ctx, int, sqlite3_value **argv) {
        // ...
    },
    nullptr, nullptr);
```

3つの方式はいずれもコンパイル後は同一の関数ポインタになるため、実行時の違いはない。

### テンプレートヘルパーによる簡潔な登録

本プロジェクトでは6種類のテンプレートパターンを使い、統計計算ロジックだけを差し替える設計を採用している。

```cpp
// --- テンプレート A: 基本集約関数 ---
static double calc_mean(const std::vector<double>& v) {
    return statcpp::mean(v.begin(), v.end());
}
rc = SingleColumnAggregate<calc_mean>::register_func(db, "stat_mean");

// --- テンプレート B: パラメータ付き集約関数 ---
rc = SingleColumnParamAggregate<1, calc_percentile>::register_func(db, "stat_percentile");
rc = SingleColumnParamAggregateText<1, calc_t_test>::register_func(db, "stat_t_test");

// --- テンプレート C: 2カラム集約関数 ---
rc = TwoColumnAggregate<calc_pearson_r>::register_func(db, "stat_pearson_r");
rc = TwoColumnAggregateText<calc_simple_regression>::register_func(db, "stat_simple_regression");
rc = TwoColumnParamAggregate<1, calc_weighted_percentile>::register_func(db, "stat_weighted_percentile");

// --- テンプレート D: ウィンドウ関数 ---
rc = FullScanWindowFunction<wf_rolling_mean>::register_func_2(db, "stat_rolling_mean");  // 2引数
rc = FullScanWindowFunction<wf_rank>::register_func_1(db, "stat_rank");                   // 1引数

// --- テンプレート E: 複合集約関数 ---
rc = SingleColumnAggregateText<calc_modes>::register_func(db, "stat_modes");
rc = TwoColumnAggregateText<calc_t_test2>::register_func(db, "stat_t_test2");

// --- テンプレート F: スカラー関数 ---
rc = register_scalar(db, "stat_normal_pdf", -1, sf_normal_pdf);     // 決定的
rc = register_scalar_nd(db, "stat_normal_rand", -1, sf_normal_rand); // 非決定的（乱数）
```

### ラムダ式の仕組み

```cpp
+[](sqlite3_context *ctx, int, sqlite3_value **argv) { ... }
```

- キャプチャなしラムダ (`[]`) は**コンパイル時に**通常の関数ポインタに変換される
- 先頭の `+` はラムダを関数ポインタへ明示的に変換する慣用句
- 生成されるコードは `static void func(...)` と**同一**
- `std::function` は関数ポインタではないため `sqlite3_create_function` に直接渡せない

### テンプレートの仕組み

```cpp
template <double (*Fn)(double)>
int register_double_func(sqlite3 *db, const char *name) { ... }
```

- テンプレートは**コンパイル時に展開**される
- `Fn(x)` の呼び出しは**インライン化**され、間接呼び出しにはならない

### オーバーロード時の注意

同名関数のオーバーロードがある場合、テンプレート引数で曖昧になる:

```cpp
static double cube(double x) { return x * x * x; }
static long   cube(long x)   { return x * x * x; }

// 明示的にキャストしてオーバーロード解決
register_double_func<static_cast<double(*)(double)>(cube)>(db, "cube");
```

SQLite の値は内部的に動的型なので `double` で統一するのが最もシンプル。

---

## 8. ダイナミックライブラリとしての考慮事項

### エントリポイントのシンボル

`extern "C"` によりマングリングが抑制され、SQLite が期待するシンボル名がエクスポートされる。

```bash
nm -gU ext_funcs.dylib | grep sqlite3_ext_funcs_init
# → _sqlite3_ext_funcs_init
```

ラムダ・テンプレートは内部リンケージ (`static` 相当) のため `.dylib` の公開シンボルテーブルには現れない。

### C++ ランタイム依存

```bash
otool -L ext_funcs.dylib
```

`std::string`, `std::reverse` 等の使用により `libc++` への動的リンクが追加される。

### 本プロジェクトでの libc++ ロードコスト

本プロジェクトでは `main.cpp` が C++でコンパイル・リンクされるため、
プロセス起動時に `dyld` が `libc++.1.dylib` を既にロードしている。

```text
1. main 起動
   └─ dyld が libc++.1.dylib をロード (main が C++ なので必須)

2. sqlite3_load_extension() で ext_funcs.dylib をロード
   └─ ext_funcs.dylib も libc++ に依存
   └─ しかし libc++ は既にメモリ上にある
   └─ → 参照カウントが増えるだけ、再ロードは発生しない
```

macOS の `dyld` は同一ライブラリの二重ロードはしない。
既にマップ済みのアドレスを共有するだけなので、コストはほぼゼロ。

### libc++ 依存を排除する方法

1. `std::string` を避け、`sqlite3_malloc` + 手書きループを使う
2. `<algorithm>` のヘッダオンリー関数のみ使用する (インライン展開される)
3. ビルド時に例外と RTTI を無効化する:

```cmake
target_compile_options(ext_funcs PRIVATE -fno-exceptions -fno-rtti)
```

### 実行時オーバーヘッド

| 要素 | オーバーヘッド | 説明 |
|---|---|---|
| ラムダ → 関数ポインタ | なし | コンパイル時変換 |
| テンプレート展開 | なし | コンパイル時展開、インライン化 |
| `extern "C"` | なし | リンケージ指定のみ |
| `std::string` + `SQLITE_TRANSIENT` | 微小 | コピーが 1回増える (reverse 関数等) |
