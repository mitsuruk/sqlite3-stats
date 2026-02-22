# statcpp.cmake リファレンス

## 概要

`statcpp.cmake` は statcpp ライブラリの自動ダウンロードと設定を行う CMake 設定ファイルです。
CMake の `file(DOWNLOAD)` を使用して GitHub からリポジトリアーカイブをダウンロードし、`download/` ディレクトリにキャッシュすることで冗長なダウンロードを回避します。

statcpp はモダンなヘッダーオンリー C++17 統計ライブラリです。31 モジュール・524 の公開関数を提供し、記述統計、仮説検定、回帰分析、分散分析、確率分布、リサンプリング、クラスタリングなどをカバーします。すべての関数は STL スタイルのイテレータベース API とプロジェクションをサポートしています。

statcpp はヘッダーオンリーであるため、コンパイルやリンクは不要です。インクルードパスの設定のみが必要です。

## ファイル情報

| 項目 | 詳細 |
|------|------|
| インストールディレクトリ | `${CMAKE_CURRENT_SOURCE_DIR}/download/statcpp/statcpp-install` |
| ダウンロード URL | https://github.com/mitsuruk/statcpp/archive/refs/heads/main.tar.gz |
| バージョン | 0.1.0 |
| ライセンス | MIT License |

---

## インクルードガード

```cmake
include_guard(GLOBAL)
```

このファイルは `include_guard(GLOBAL)` を使用して、複数回インクルードされても一度だけ実行されるようにしています。

**必要な理由：**

- configure 時の `file(DOWNLOAD)` の重複呼び出しを防止
- `target_include_directories` の重複呼び出しを防止

---

## ディレクトリ構造

```
project/
├── cmake/
│   ├── statcpp.cmake         # この設定ファイル
│   ├── statcppCmake.md       # 英語版ドキュメント
│   └── statcppCmake-jp.md    # このドキュメント
├── download/statcpp/
│   ├── statcpp-main.tar.gz   # キャッシュされたアーカイブ
│   └── statcpp-install/      # インストールされたヘッダー
│       └── include/
│           └── statcpp/
│               ├── statcpp.hpp               # マスターインクルード
│               ├── basic_statistics.hpp       # 平均、中央値、最頻値、...
│               ├── parametric_tests.hpp       # t 検定、z 検定、F 検定、...
│               ├── linear_regression.hpp      # 単回帰・重回帰
│               ├── anova.hpp                  # 一元配置、二元配置、反復測定
│               ├── continuous_distributions.hpp
│               └── ...（全 31 ヘッダーファイル）
├── src/
│   └── main.cpp
├── build/
└── CMakeLists.txt
```

## 使い方

### CMakeLists.txt への追加

```cmake
include("./cmake/statcpp.cmake")
```

### ビルド

```bash
mkdir build && cd build
cmake ..
make
```

---

## 処理の流れ

### 1. ディレクトリパスの設定

```cmake
set(STATCPP_DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/download/statcpp)
set(STATCPP_INSTALL_DIR ${STATCPP_DOWNLOAD_DIR}/statcpp-install)
set(STATCPP_VERSION "0.1.0")
set(STATCPP_BRANCH "main")
set(STATCPP_URL "https://github.com/mitsuruk/statcpp/archive/refs/heads/${STATCPP_BRANCH}.tar.gz")
```

### 2. キャッシュチェックと条件付きダウンロード

```cmake
if(EXISTS ${STATCPP_INSTALL_DIR}/include/statcpp/statcpp.hpp)
    message(STATUS "statcpp already installed")
else()
    # ダウンロードとインストール ...
endif()
```

キャッシュのロジックは以下の通りです：

| 条件 | アクション |
|------|----------|
| `statcpp-install/include/statcpp/statcpp.hpp` が存在 | すべてスキップ（キャッシュを使用） |
| `statcpp-main.tar.gz` が存在（インストールなし） | ダウンロードをスキップ、展開してインストール |
| 何も存在しない | GitHub からダウンロード、展開してインストール |

### 3. ダウンロード（必要な場合）

```cmake
file(DOWNLOAD
    ${STATCPP_URL}
    ${STATCPP_CACHED_ARCHIVE}
    SHOW_PROGRESS
    TIMEOUT 120
    INACTIVITY_TIMEOUT 30
    STATUS DOWNLOAD_STATUS
)
```

- GitHub からリポジトリアーカイブ（`.tar.gz`）をダウンロード
- `include/statcpp/` 内の全 31 ヘッダーファイルを含む

### 4. 展開とインストール

```cmake
file(ARCHIVE_EXTRACT
    INPUT ${STATCPP_CACHED_ARCHIVE}
    DESTINATION ${STATCPP_EXTRACT_DIR}
)

file(COPY ${STATCPP_EXTRACTED_DIR}/include/statcpp
    DESTINATION ${STATCPP_INSTALL_DIR}/include
)
```

- アーカイブを一時ディレクトリに展開
- `include/statcpp/` ディレクトリをインストール先にコピー
- 一時展開ディレクトリをクリーンアップ
- コンパイル手順は不要（ヘッダーオンリーライブラリ）

### 5. インクルードパスの設定

```cmake
target_include_directories(${PROJECT_NAME} PRIVATE
    ${STATCPP_INSTALL_DIR}/include
)
```

statcpp はヘッダーオンリーです。`add_library`、`target_link_libraries`、静的ライブラリの作成は不要です。

---

## statcpp ライブラリ

statcpp はモジュールに分類された 31 のヘッダーファイルで構成されます：

### ヘッダーファイル

| カテゴリ | ヘッダー | 説明 |
|---------|--------|------|
| **記述統計** | `basic_statistics.hpp` | 平均、中央値、最頻値、トリム平均など |
| | `order_statistics.hpp` | 四分位数、パーセンタイル、分位数 |
| | `dispersion_spread.hpp` | 分散、標準偏差、範囲、四分位範囲 |
| | `shape_of_distribution.hpp` | 歪度、尖度 |
| | `correlation_covariance.hpp` | ピアソン、スピアマン、ケンドール相関 |
| | `frequency_distribution.hpp` | ヒストグラム、度数表 |
| **確率** | `special_functions.hpp` | ガンマ関数、ベータ関数、誤差関数 |
| | `random_engine.hpp` | 乱数生成 |
| | `continuous_distributions.hpp` | 正規分布、t 分布、カイ二乗分布、F 分布など |
| | `discrete_distributions.hpp` | 二項分布、ポアソン分布など |
| **推測統計** | `estimation.hpp` | 信頼区間 |
| | `parametric_tests.hpp` | t 検定、z 検定、F 検定 |
| | `nonparametric_tests.hpp` | ウィルコクソン検定、マン・ホイットニー検定 |
| | `effect_size.hpp` | コーエンの d、ヘッジズの g |
| | `resampling.hpp` | ブートストラップ、ジャックナイフ、順列検定 |
| | `power_analysis.hpp` | サンプルサイズ・検定力計算 |
| **モデリング** | `linear_regression.hpp` | 単回帰・重回帰 |
| | `anova.hpp` | 一元配置、二元配置、反復測定分散分析 |
| | `glm.hpp` | ロジスティック回帰、一般化線形モデル |
| | `model_selection.hpp` | AIC、BIC、交差検証 |
| **応用解析** | `clustering.hpp` | K 平均法、階層的クラスタリング |
| | `distance_metrics.hpp` | ユークリッド距離、マンハッタン距離、コサイン距離 |
| | `categorical.hpp` | カテゴリカルデータ分析 |
| | `data_wrangling.hpp` | データ操作ユーティリティ |
| | `missing_data.hpp` | 欠測データ処理 |
| | `multivariate.hpp` | 多変量解析 |
| | `robust.hpp` | ロバスト統計 |
| | `survival.hpp` | 生存時間分析 |
| | `time_series.hpp` | 時系列分析 |
| **ユーティリティ** | `numerical_utils.hpp` | 数値計算ヘルパー |
| **マスター** | `statcpp.hpp` | 全モジュールをインクルード |

---

## statcpp の主な機能

| 機能 | 説明 |
|------|------|
| ヘッダーオンリー | コンパイルやリンク不要 |
| C++17 | モダンな C++17 機能を使用 |
| 524 関数 | 統計手法の包括的なカバレッジ |
| STL スタイル API | イテレータベースのインターフェース（`begin`、`end`） |
| プロジェクションサポート | プロジェクションによる構造体メンバーの直接処理 |
| R 検証済み | R 4.4.2 に対して 167 の数値チェックで検証済み |
| 758 のユニットテスト | Google Test フレームワークでテスト済み |
| クロスプラットフォーム | macOS、Linux |

---

## C++ での使用例

### 全モジュールのインクルード

```cpp
#include <statcpp/statcpp.hpp>
```

### 特定モジュールのインクルード

```cpp
#include <statcpp/basic_statistics.hpp>
#include <statcpp/parametric_tests.hpp>
```

### 基本統計

```cpp
#include <statcpp/basic_statistics.hpp>
#include <vector>
#include <iostream>

int main() {
    std::vector<double> data = {2.0, 4.0, 6.0, 8.0, 10.0};

    double m = statcpp::mean(data.begin(), data.end());
    double med = statcpp::median(data.begin(), data.end());

    std::cout << "平均: " << m << "\n";    // 6.0
    std::cout << "中央値: " << med << "\n"; // 6.0

    return 0;
}
```

### 仮説検定

```cpp
#include <statcpp/parametric_tests.hpp>
#include <vector>
#include <iostream>

int main() {
    std::vector<double> group1 = {5.1, 4.9, 5.3, 5.0, 5.2};
    std::vector<double> group2 = {4.8, 4.6, 4.7, 4.5, 4.9};

    auto result = statcpp::t_test_two_sample(
        group1.begin(), group1.end(),
        group2.begin(), group2.end()
    );

    std::cout << "t 統計量: " << result.statistic << "\n";
    std::cout << "p 値: " << result.p_value << "\n";

    return 0;
}
```

### 線形回帰

```cpp
#include <statcpp/linear_regression.hpp>
#include <vector>
#include <iostream>

int main() {
    std::vector<double> x = {1, 2, 3, 4, 5};
    std::vector<double> y = {2.1, 3.9, 6.2, 7.8, 10.1};

    auto result = statcpp::simple_linear_regression(
        x.begin(), x.end(),
        y.begin(), y.end()
    );

    std::cout << "切片: " << result.intercept << "\n";
    std::cout << "傾き: " << result.slope << "\n";
    std::cout << "決定係数: " << result.r_squared << "\n";

    return 0;
}
```

### プロジェクションサポート

```cpp
#include <statcpp/basic_statistics.hpp>
#include <vector>
#include <iostream>

struct Product {
    std::string name;
    double price;
};

int main() {
    std::vector<Product> products = {
        {"Apple", 1.5}, {"Banana", 0.8}, {"Cherry", 3.0}
    };

    double avg = statcpp::mean(
        products.begin(), products.end(),
        [](const Product& p) { return p.price; }
    );

    std::cout << "平均価格: " << avg << "\n"; // 1.76667

    return 0;
}
```

### コンパイル

```bash
g++ -std=c++17 -I download/statcpp/statcpp-install/include example.cpp -o example
./example
```

---

## statcpp API の規約

### 名前空間

すべての機能は `statcpp` 名前空間にあります：

```cpp
double m = statcpp::mean(data.begin(), data.end());
auto result = statcpp::t_test_two_sample(g1.begin(), g1.end(), g2.begin(), g2.end());
```

### イテレータベース API

ほとんどの関数はイテレータペアを受け取ります：

```cpp
statcpp::mean(first, last);                   // 基本形
statcpp::mean(first, last, projection);       // プロジェクション付き
```

### 戻り値の型

| 型 | 使用場面 |
|------|---------|
| `double` | スカラー統計量（平均、分散など） |
| 構造体 | 検定、回帰（`.statistic`、`.p_value`、`.r_squared` など） |

---

## トラブルシューティング

### ダウンロードが失敗する

GitHub に接続できない場合、手動でダウンロードして配置できます：

```bash
curl -L -o download/statcpp/statcpp-main.tar.gz \
    https://github.com/mitsuruk/statcpp/archive/refs/heads/main.tar.gz
```

その後 `cmake ..` を再実行すると、キャッシュされたアーカイブからインストールされます。

### 最初からリビルド

新規のダウンロードとインストールを強制する場合：

```bash
rm -rf download/statcpp
cd build && cmake ..
```

### ヘッダーが見つからない

インクルードディレクトリが正しく設定されていることを確認してください：

```cmake
target_include_directories(${PROJECT_NAME} PRIVATE ${STATCPP_INSTALL_DIR}/include)
```

ヘッダーは以下のようにインクルードします：

```cpp
#include <statcpp/statcpp.hpp>            // OK（全モジュール）
#include <statcpp/basic_statistics.hpp>   // OK（特定モジュール）
// #include "statcpp.hpp"                 // NG（パスが間違い）
```

### C++17 が必要

statcpp は C++17 以降が必要です。CMakeLists.txt に以下を含めてください：

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

---

## 参考資料

- [statcpp GitHub リポジトリ](https://github.com/mitsuruk/statcpp)
- [CMake file(DOWNLOAD) ドキュメント](https://cmake.org/cmake/help/latest/command/file.html#download)
- [CMake file(ARCHIVE_EXTRACT) ドキュメント](https://cmake.org/cmake/help/latest/command/file.html#archive-extract)
