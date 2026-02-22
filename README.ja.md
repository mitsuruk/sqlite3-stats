# sqlite3StatisticalLibrary — SQLite3 用統計関数ライブラリ

SQLite3 に **249 個の統計関数** を追加するロード可能な拡張機能です。

[![CI](https://github.com/mitsuruk/sqlite3-stats/actions/workflows/ci.yml/badge.svg)](https://github.com/mitsuruk/sqlite3-stats/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)

[English](README.md)

## 概要

sqlite3StatisticalLibrary は、SQL から直接呼び出せる 249 個の統計関数を提供する SQLite3 のロード可能な拡張機能です。[statcpp](https://github.com/mitsuruk/statcpp)（524 個の関数を持つ C++17 ヘッダーオンリー統計ライブラリ）をベースに構築されており、統計機能の厳選されたサブセットをネイティブ SQL 関数として公開しています。

全 249 関数は 266 件の結合テストで検証済みです。

### 主な特徴

- **249 個の SQL 関数** — 記述統計、仮説検定、分布、リサンプリングなどをカバー
- **ランタイム依存なし** — 単一の共有ライブラリ（`.dylib` / `.so` / `.dll`）のみ
- **標準 SQL インターフェース** — `SELECT`、`GROUP BY`、`HAVING`、サブクエリなどで使用可能
- **ウィンドウ関数対応** — 移動統計量、移動平均、外れ値検出
- **JSON 出力** — 複合的な結果（複数値、度数表、検定結果）を JSON で返却
- **クロスプラットフォーム** — macOS および Linux 対応

### 関数カテゴリ

| カテゴリ | 個数 | 説明 |
| --- | --- | --- |
| 基本集約関数 | 24 | 単一カラム: `SELECT stat_xxx(col) FROM t` |
| パラメータ付き集約関数 | 20 | パラメータ指定: `SELECT stat_xxx(col, param) FROM t` |
| 2 カラム集約関数 | 27 | 2 カラム入力: `SELECT stat_xxx(col1, col2) FROM t` |
| ウィンドウ関数 | 23 | 全行スキャン型ウィンドウ関数（行ごとに値を返却） |
| 複合集約関数 | 32 | JSON 返却の集約関数、2 標本検定、生存時間解析 |
| スカラー関数 — 検定・補助 | 40 | 分布関数、比率検定、多重検定補正 |
| スカラー関数 — 分布・変換 | 83 | 連続・離散分布、効果量変換、検出力分析 |

## クイックスタート

### 前提条件

- CMake 3.20 以上
- C++17 コンパイラ（Apple Clang 17 以上、GCC 13 以上）
- SQLite3（CMake が自動ダウンロード）
- [statcpp](https://github.com/mitsuruk/statcpp)（CMake が自動ダウンロード）

### ビルド

```bash
git clone https://github.com/mitsuruk/sqlite3StatisticalLibrary.git
cd sqlite3StatisticalLibrary
mkdir build && cd build
cmake ..
make
```

以下が生成されます:

- `ext_funcs.dylib`（macOS）または `ext_funcs.so`（Linux） — ロード可能な拡張機能
- `a.out` — テストランナー（266 テスト）

### テストの実行

```bash
./a.out
# [OK] All tests completed (266 tests).
```

### 拡張機能のロード

#### sqlite3 CLI から

```sql
-- .load コマンド（推奨）
.load ./ext_funcs sqlite3_ext_funcs_init

-- または load_extension() を使用
SELECT load_extension('./ext_funcs', 'sqlite3_ext_funcs_init');
```

#### C/C++ から

```cpp
sqlite3_enable_load_extension(db, 1);
sqlite3_load_extension(db, "./ext_funcs.dylib",
                        "sqlite3_ext_funcs_init", &errmsg);
```

> **注意**: エントリポイント `sqlite3_ext_funcs_init` を明示的に指定する必要があります。SQLite の自動検出はアンダースコアを除去するため、自動では見つかりません。

## 使用例

### 記述統計

```sql
SELECT stat_mean(score), stat_stdev(score), stat_median(score)
FROM exam_results;

SELECT class, stat_mean(score), stat_cv(score)
FROM exam_results
GROUP BY class;
```

### 仮説検定

```sql
-- 1 標本 t 検定（H0: mu = 50）
SELECT stat_t_test(score, 50) FROM exam_results;

-- 2 標本 Welch の t 検定（JSON を返却）
SELECT stat_t_test_welch(value, group_id) FROM measurements;
```

### ウィンドウ関数

```sql
SELECT date, price,
       stat_rolling_mean(price, 7) OVER (ORDER BY date) AS ma7,
       stat_ema(price, 0.3)        OVER (ORDER BY date) AS ema
FROM stock_prices;
```

### 分布関数（スカラー）

```sql
-- 正規分布の分位点（95% の z 値）
SELECT stat_normal_quantile(0.975, 0, 1);  -- 1.96

-- 検出力分析: 2 標本 t 検定に必要なサンプルサイズ
SELECT stat_n_t2(0.5, 0.05, 0.80);  -- 効果量=0.5, alpha=0.05, 検出力=0.80
```

## 共通仕様

- **NULL の扱い**: すべての集約関数・ウィンドウ関数で NULL 値は暗黙的に無視されます。
- **データ不足時**: 非 NULL 値が十分でない場合は `NULL` を返します（例: 分散の計算には n >= 2 が必要）。
- **数値精度**: IEEE 754 倍精度（64 ビット）。`NaN` および `Inf` は `NULL` にマッピングされます。
- **JSON 出力**: 複合集約関数の結果は JSON 文字列（オブジェクトまたは配列）として返されます。

## 関数リファレンス

全 249 関数の詳細ドキュメント:

- [関数リファレンス（ハブ）](doc/関数リファレンス.md)
  - [基本集約関数（24）](doc/ref/基本集約関数.md)
  - [パラメータ付き集約関数（20）](doc/ref/パラメータ付き集約関数.md)
  - [2 カラム集約関数（27）](doc/ref/2カラム集約関数.md)
  - [ウィンドウ関数（23）](doc/ref/ウィンドウ関数.md)
  - [複合集約関数（32）](doc/ref/複合集約関数.md)
  - [スカラー関数 — 検定・補助（40）](doc/ref/スカラー関数_検定補助.md)
  - [スカラー関数 — 分布・変換（83）](doc/ref/スカラー関数_分布変換.md)

## プロジェクト構成

```text
sqlite3StatisticalLibrary/
├── CMakeLists.txt
├── README.md                          # 英語版 README
├── README.ja.md                       # このファイル（日本語版）
├── src/
│   ├── ext_funcs.cpp                  # 拡張機能: 249 個の SQL 関数
│   ├── main.cpp                       # テストランナー（266 テスト）
│   └── include/                       # ローカルヘッダー
├── doc/
│   ├── 関数リファレンス.md              # 関数リファレンス（ハブ）
│   ├── sqlite3lib_LOAD_EXTENSION.md   # 拡張機能の実装ガイド
│   └── ref/                           # カテゴリ別の関数詳細
│       ├── 基本集約関数.md
│       ├── パラメータ付き集約関数.md
│       ├── 2カラム集約関数.md
│       ├── ウィンドウ関数.md
│       ├── 複合集約関数.md
│       ├── スカラー関数_検定補助.md
│       └── スカラー関数_分布変換.md
├── cmake/
│   ├── sqlite3.cmake                  # SQLite3 自動ダウンロード
│   └── statcpp.cmake                  # statcpp 自動ダウンロード
└── download/                          # 自動ダウンロードされた依存関係
    ├── sqlite3/
    └── statcpp/
```

## 動作確認環境

- macOS + Apple Clang 17.0.0
- macOS + GCC 15 (Homebrew)

## ライセンス

このプロジェクトは MIT ライセンスの下で公開されています。

## 謝辞

このプロジェクトの開発には以下のツールと AI アシスタントを活用しています:

- **[statcpp](https://github.com/mitsuruk/statcpp)** — C++17 ヘッダーオンリー統計ライブラリ（524 関数）
- **Claude Code for VS Code (Opus 4.5/4.6)** — コード生成、リファクタリング、ドキュメント作成
- **OpenAI ChatGPT 5.2** — ドキュメントレビュー
- **LM Studio google/gemma-2-27b** — ドキュメントレビュー

---

**免責事項**: このライブラリは数値安定性や極端なエッジケースへの対応において、商用の統計ソフトウェアと同等のレベルを保証するものではありません。研究や本番環境で結果を使用する場合は、R や Python/SciPy などの確立されたツールとのクロスバリデーションを推奨します。
