# NULL・NaN・Inf に関する注意事項

SQL 関数が欠損値(`NULL`)、`NaN`、無限大をどう扱うかをまとめる。
内部で使う statcpp は v0.5.0 から R の既定の欠損値処理に従う。本拡張はその上に以下の SQL の慣例を加えているため、
SQL での結果は R や statcpp と常に同じになるわけではない。

例はすべて本バージョンの拡張で実行して確認したものである。

← [関数リファレンス](function_reference-ja.md) に戻る

---

## 1. SQLite が格納できる値

- **NaN は格納できない。** `9e999 - 9e999` のように NaN になる式は、クエリ中でもテーブルへの挿入時でも `NULL` になる。
  したがって列に NaN が入ることはなく、欠損値は `NULL` で表される。
- **Inf は格納できる。** `9e999` と `-9e999` は `+Inf` と `-Inf` として保持され、通常の値として扱われる。

---

## 2. 集約関数

基本集約、パラメータ付き集約、2 カラム集約、グループ列付き集約、複合集約の各関数に当てはまる。

- **`NULL` の行は無視される。** SQLite 自身の `avg()` と同じである。これは R の `na.rm = TRUE` に相当し、
  R の既定(NA を含むデータの `mean()` は NA)とは異なる。
- **2 カラム関数とグループ列付き関数は、いずれかの列が `NULL` の行を無視する**(完全ケース)。たとえば
  `stat_pearson_r(x, y)` は `x` と `y` の両方がある行だけを使い、
  `stat_anova1(val, grp)` は値かグループが `NULL` の行を除く。
- **すべての行が `NULL`、または行がない場合は `NULL` を返す。**
- **結果が NaN または Inf の場合は `NULL` を返す。**
  1、2、`9e999` の平均は数学的には `+Inf` だが、`stat_mean` は `NULL` を返す。
  JSON を返す関数では、NaN や Inf のフィールドは `null` になる。
- **退化したデータは `NULL` ではなくエラーになる。** statcpp が入力を受け付けない場合、文は statcpp のメッセージとともに失敗する:

  ```sql
  SELECT stat_t_test(v, 5)
  FROM (SELECT 5 AS v UNION ALL SELECT 5 UNION ALL SELECT 5);
  -- Error: statcpp::t_test: zero variance
  ```

---

## 3. パラメータ: `NULL` を渡すと `NULL` になる

SQL の慣例に従い、`NULL` のパラメータを渡すと結果は `NULL` になる。0 として読むことも、既定値に置き換えることもしない:

```sql
SELECT stat_percentile(v, NULL) FROM t;         -- NULL
SELECT stat_t_test(v, NULL) FROM t;             -- NULL
SELECT stat_tukey_hsd(val, grp, NULL) FROM g;   -- NULL
```

- **ウィンドウ関数**は、パラメータが `NULL` なら SQL の `lag(v, NULL)` と同じく全行 `NULL` を返す。
  `stat_rolling_mean(v, NULL)` も `stat_lag(v, NULL)` も全行 `NULL` になる。
- **既定値を使うには、パラメータを省略する。** 事後検定の末尾の `alpha` のように、省略できる関数に限る。
  `stat_tukey_hsd(val, grp)` は `alpha` = 0.05 を使う。
- **既定値を持つ関数は、パラメータの 0 を「既定値を使う」と読む。** `stat_tukey_hsd(val, grp, 0)` も
  `alpha` = 0.05 になり、0 以下の窓幅やラグは 1 になる。
- **集約関数は、値が `NULL` でない最初の行からパラメータを読み、以降の行の値は無視する。**
  パラメータには列ではなく定数を渡すこと。

---

## 4. スカラー関数: `NULL` 引数を渡すと `NULL` になる

`abs(NULL)` など SQLite の組み込み関数と同じく、**スカラー関数はいずれかの引数が `NULL` なら `NULL` を返す**:

```sql
SELECT stat_normal_cdf(NULL);              -- NULL
SELECT stat_binomial_pmf(3, 10, NULL);     -- NULL
SELECT stat_normal_pdf(0.5, NULL, 1.0);    -- NULL
SELECT stat_normal_pdf(0.5);               -- 0.352...(mu = 0、sigma = 1)
```

正規分布の関数の `mu` と `sigma` のような省略可能な引数は、省略したときだけ既定値になり、`NULL` のときは既定値にならない。

Inf の引数はそのまま statcpp に渡される。`stat_t_cdf(9e999, 5)` は `1.0` になる。

---

## 5. ウィンドウ関数

ウィンドウ関数は行ごとに値を返し、`OVER` 句が必要である:

```sql
SELECT id, stat_rolling_mean(val, 3) OVER (
    ORDER BY id ROWS BETWEEN UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING)
FROM t;
```

`OVER` なしで呼ぶと集約関数として動き、1 つの値だけを返す。その値に意味はない。

`NULL` の行が結果に与える影響は関数の種類で異なる。例では `val` = 10、20、`NULL`、40、50、30(1〜6 行目)を使う。

| 種類 | `NULL` の行の影響 |
| --- | --- |
| ローリング | その行を含む窓が `NULL` |
| 累積 | 以降のすべての行が `NULL` |
| シフト | 値として移動する |
| 行ごと | その行が `NULL` |
| 補正 | その行が `NULL` |
| 補完 | その行が埋められる |

- ローリング: `stat_rolling_*`、`stat_moving_avg`
- 累積: `stat_ema`
- シフト: `stat_lag`、`stat_diff`、`stat_seasonal_diff`
- 行ごと: `stat_rank`、`stat_label_encode`、`stat_bin_width`、`stat_bin_freq`、`stat_outliers_*`、`stat_winsorize`
- 補正: `stat_bonferroni`、`stat_bh_correction`、`stat_holm_correction`
- 補完: `stat_fillna_*`

- **ローリング。** 行 i にはその行で終わる窓の値が入り、先頭の n - 1 行は `NULL` になる。
  `stat_rolling_mean(val, 3)` は 1〜5 行目が `NULL`、6 行目が 40 になる。
- **累積。** `stat_ema` は行から行へ状態を引き継ぐため、最初の `NULL` 以降はすべての行が `NULL` になる。
  `stat_ema(val, 3)` は 10、15 のあと 3〜6 行目が `NULL` になる。先頭行が `NULL` なら全行が `NULL` になる。
- **シフト。** `stat_lag(val, 1)` は SQL の `lag()` と同じく `NULL`、10、20、`NULL`、40、50 になる。
  `stat_diff(val, 1)` は、どちらかの値が `NULL` の行で `NULL` になる。
- **行ごと。** 統計量は `NULL` でない行から求め、`NULL` の行は `NULL` を返す。
  `stat_rank(val)` は 1、2、`NULL`、4、5、3 になる。
- **補正。** R の `p.adjust()` と同じく、`NULL` の p 値は検定数に数えない。
- **補完。** `stat_fillna_mean` と `stat_fillna_median` は観測値の平均または中央値で埋める。
  `stat_fillna_ffill`、`stat_fillna_bfill`、`stat_fillna_interp` は先頭(ffill、interp)や末尾(bfill、interp)の
  `NULL` を埋められず、そのまま `NULL` になる。

### ローリング窓の Inf

`stat_rolling_mean` と `stat_rolling_sum` は累積和を使うため、
R の `zoo::rollmean()` と同じく Inf 以降の窓はすべて `NULL` になる。
`stat_moving_avg` は R の `stats::filter()` と同じく窓ごとに合計するため、影響は Inf を含む窓だけに限られる:

| `v` | 1 | `9e999` | 2 | 3 |
| --- | --- | --- | --- | --- |
| `stat_rolling_mean(v, 2)` | `NULL` | Inf | Inf | `NULL` |
| `stat_moving_avg(v, 2)` | `NULL` | Inf | Inf | 2.5 |

集約関数と異なり、ウィンドウ関数は Inf を Inf のまま返す。`NULL` になるのは NaN だけである。

---

## 6. statcpp との関係

statcpp 自身の規則は statcpp の `docs-ja/NAN_POLICY.md` にある。
本拡張は集約・2 カラム・スカラーの経路では statcpp を呼ぶ前に
`NULL` を除くため、statcpp の NaN 処理が関わるのは、`NULL` の行を NaN として statcpp に渡すウィンドウ関数だけである。
