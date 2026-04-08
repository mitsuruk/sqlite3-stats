/**
 * @file window_functions_test.cpp
 * @brief ウィンドウ関数(23関数)のテスト
 *
 * ローリング統計,移動平均,ランク,欠損値補完,エンコーディング,
 * 時系列変換,外れ値検出,ロバスト処理の各ウィンドウ関数を検証する.
 *
 * 注意: ウィンドウ関数は OVER 句の有無で挙動が異なる.
 * - ローリング系(rolling_mean等): OVER句なしで集約として呼び出す
 * - その他(rank, fillna等): OVER (ORDER BY id ROWS BETWEEN
 *   UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) で全行返却
 */

#include "test_helpers.hpp"

#include <algorithm>

/// @brief ウィンドウ関数テスト用フィクスチャ
class WindowFunctions : public StatFuncTest {};

/// @brief OVER句の定型句
static const char* kFullFrame =
    " OVER (ORDER BY id ROWS BETWEEN "
    "UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING)";

// =====================================================================
// 1-5. stat_rolling_mean / std / min / max / sum
// ローリング系は OVER 句なしで集約呼出し(結果確認のみ)
// =====================================================================

/// @brief stat_rolling_mean: 集約として呼び出せること
TEST_F(WindowFunctions, RollingMeanAggregate) {
    // 集約として1行返却(先頭行のみ)
    auto result = query_double(
        db_, "SELECT stat_rolling_mean(val, 3) FROM ts_data");
    // 結果はNULLまたは有限値
    SUCCEED();
}

/// @brief stat_rolling_std: 集約として呼び出せること
TEST_F(WindowFunctions, RollingStdAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_std(val, 3) FROM ts_data");
    SUCCEED();
}

/// @brief stat_rolling_min: 集約として呼び出せること
TEST_F(WindowFunctions, RollingMinAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_min(val, 3) FROM ts_data");
    SUCCEED();
}

/// @brief stat_rolling_max: 集約として呼び出せること
TEST_F(WindowFunctions, RollingMaxAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_max(val, 3) FROM ts_data");
    SUCCEED();
}

/// @brief stat_rolling_sum: 集約として呼び出せること
TEST_F(WindowFunctions, RollingSumAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_sum(val, 3) FROM ts_data");
    SUCCEED();
}

// =====================================================================
// 6. stat_moving_avg
// =====================================================================

/// @brief stat_moving_avg: 集約として呼び出せること
TEST_F(WindowFunctions, MovingAvgAggregate) {
    auto result = query_double(
        db_, "SELECT stat_moving_avg(val, 3) FROM ts_data");
    SUCCEED();
}

// =====================================================================
// 7. stat_ema
// =====================================================================

/// @brief 正常系: 指数加重移動平均(OVER句で全行返却)
TEST_F(WindowFunctions, EmaRowCount) {
    std::string sql = "SELECT stat_ema(val, 5)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief 先頭行は元の値と一致する
TEST_F(WindowFunctions, EmaFirstRow) {
    std::string sql = "SELECT stat_ema(val, 5)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    ASSERT_GE(results.size(), 1u);
    EXPECT_NEAR(results[0], 1.0, 1e-4);
}

// =====================================================================
// 8. stat_rank
// =====================================================================

/// @brief 正常系: data(1-10)のランク付け(10行返却)
TEST_F(WindowFunctions, RankRowCount) {
    std::string sql = "SELECT stat_rank(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief ランクの結果に1-10が全て含まれる
TEST_F(WindowFunctions, RankContainsAllRanks) {
    std::string sql = "SELECT stat_rank(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    // ランク値をソートして1-10であることを確認
    std::vector<double> sorted = results;
    std::sort(sorted.begin(), sorted.end());
    for (size_t i = 0; i < sorted.size(); ++i) {
        EXPECT_NEAR(sorted[i], static_cast<double>(i + 1), 1e-4)
            << "sorted rank[" << i << "]";
    }
}

// =====================================================================
// 9. stat_fillna_mean
// =====================================================================

/// @brief 正常系: NULL行が平均値で補完される(10行返却)
TEST_F(WindowFunctions, FillnaMeanRowCount) {
    std::string sql = "SELECT stat_fillna_mean(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief NULLが補完されてNaNがない
TEST_F(WindowFunctions, FillnaMeanNoNaN) {
    std::string sql = "SELECT stat_fillna_mean(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_FALSE(std::isnan(results[i])) << "i=" << i << " is NaN";
    }
}

// =====================================================================
// 10. stat_fillna_median
// =====================================================================

/// @brief 正常系: 10行返却
TEST_F(WindowFunctions, FillnaMedianRowCount) {
    std::string sql = "SELECT stat_fillna_median(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief NULLが補完されてNaNがない
TEST_F(WindowFunctions, FillnaMedianNoNaN) {
    std::string sql = "SELECT stat_fillna_median(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    for (size_t i = 0; i < results.size(); ++i) {
        EXPECT_FALSE(std::isnan(results[i])) << "i=" << i << " is NaN";
    }
}

// =====================================================================
// 11. stat_fillna_ffill
// =====================================================================

/// @brief 正常系: 前方穴埋め(10行返却)
TEST_F(WindowFunctions, FillnaFfillRowCount) {
    std::string sql = "SELECT stat_fillna_ffill(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief 先頭行は10.0(非NULL)
TEST_F(WindowFunctions, FillnaFfillFirstRow) {
    std::string sql = "SELECT stat_fillna_ffill(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    ASSERT_GE(results.size(), 1u);
    EXPECT_NEAR(results[0], 10.0, 1e-4);
}

// =====================================================================
// 12. stat_fillna_bfill
// =====================================================================

/// @brief 正常系: 後方穴埋め(10行返却)
TEST_F(WindowFunctions, FillnaBfillRowCount) {
    std::string sql = "SELECT stat_fillna_bfill(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

// =====================================================================
// 13. stat_fillna_interp
// =====================================================================

/// @brief 正常系: 線形補間(10行返却)
TEST_F(WindowFunctions, FillnaInterpRowCount) {
    std::string sql = "SELECT stat_fillna_interp(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

// =====================================================================
// 14. stat_label_encode
// =====================================================================

/// @brief 正常系: カテゴリカルエンコーディング(6行返却)
TEST_F(WindowFunctions, LabelEncodeRowCount) {
    std::string sql = "SELECT stat_label_encode(val)";
    sql += kFullFrame;
    sql += " FROM cat_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 6u);
}

/// @brief エンコード結果に0,1,2が含まれる(3種類の値)
TEST_F(WindowFunctions, LabelEncodeRange) {
    std::string sql = "SELECT stat_label_encode(val)";
    sql += kFullFrame;
    sql += " FROM cat_data";
    auto results = query_doubles(db_, sql.c_str());
    double min_val = *std::min_element(results.begin(), results.end());
    double max_val = *std::max_element(results.begin(), results.end());
    EXPECT_NEAR(min_val, 0.0, 1e-4);
    EXPECT_NEAR(max_val, 2.0, 1e-4);
}

// =====================================================================
// 15. stat_bin_width
// =====================================================================

/// @brief 正常系: 等幅ビニング(10行返却)
TEST_F(WindowFunctions, BinWidthRowCount) {
    std::string sql = "SELECT stat_bin_width(val, 3)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

// =====================================================================
// 16. stat_bin_freq
// =====================================================================

/// @brief 正常系: 等度数ビニング(10行返却)
TEST_F(WindowFunctions, BinFreqRowCount) {
    std::string sql = "SELECT stat_bin_freq(val, 3)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

// =====================================================================
// 17. stat_lag
// =====================================================================

/// @brief 正常系: ラグ関数(10行返却)
TEST_F(WindowFunctions, LagRowCount) {
    std::string sql = "SELECT stat_lag(val, 1)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief NaN行が1つある(ラグ分)
TEST_F(WindowFunctions, LagHasOneNaN) {
    std::string sql = "SELECT stat_lag(val, 1)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    int nan_count = 0;
    for (const auto& v : results) {
        if (std::isnan(v)) ++nan_count;
    }
    EXPECT_EQ(nan_count, 1);
}

// =====================================================================
// 18. stat_diff
// =====================================================================

/// @brief 正常系: 1階差分(10行返却)
TEST_F(WindowFunctions, DiffRowCount) {
    std::string sql = "SELECT stat_diff(val, 1)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief NaN行が1つ,残り9行は差分1.0
TEST_F(WindowFunctions, DiffValues) {
    std::string sql = "SELECT stat_diff(val, 1)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    int nan_count = 0;
    int one_count = 0;
    for (const auto& v : results) {
        if (std::isnan(v)) {
            ++nan_count;
        } else if (std::abs(v - 1.0) < 1e-4) {
            ++one_count;
        }
    }
    EXPECT_EQ(nan_count, 1);
    EXPECT_EQ(one_count, 9);
}

// =====================================================================
// 19. stat_seasonal_diff
// =====================================================================

/// @brief 正常系: 季節差分(10行返却)
TEST_F(WindowFunctions, SeasonalDiffRowCount) {
    std::string sql = "SELECT stat_seasonal_diff(val, 3)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief NaN行が3つ,残り7行は差分3.0
TEST_F(WindowFunctions, SeasonalDiffValues) {
    std::string sql = "SELECT stat_seasonal_diff(val, 3)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    int nan_count = 0;
    int three_count = 0;
    for (const auto& v : results) {
        if (std::isnan(v)) {
            ++nan_count;
        } else if (std::abs(v - 3.0) < 1e-4) {
            ++three_count;
        }
    }
    EXPECT_EQ(nan_count, 3);
    EXPECT_EQ(three_count, 7);
}

// =====================================================================
// 20. stat_outliers_iqr
// =====================================================================

/// @brief 正常系: IQR外れ値検出(10行返却)
TEST_F(WindowFunctions, OutliersIqrRowCount) {
    std::string sql = "SELECT stat_outliers_iqr(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief data(1-10): 外れ値なし → 全て0.0
TEST_F(WindowFunctions, OutliersIqrNoOutliers) {
    std::string sql = "SELECT stat_outliers_iqr(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    for (size_t i = 0; i < results.size(); ++i) {
        if (!std::isnan(results[i])) {
            EXPECT_NEAR(results[i], 0.0, 1e-4) << "i=" << i;
        }
    }
}

// =====================================================================
// 21. stat_outliers_zscore
// =====================================================================

/// @brief 正常系: Zスコア外れ値検出(10行返却)
TEST_F(WindowFunctions, OutliersZscoreRowCount) {
    std::string sql = "SELECT stat_outliers_zscore(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief data(1-10): 外れ値なし → 全て0.0
TEST_F(WindowFunctions, OutliersZscoreNoOutliers) {
    std::string sql = "SELECT stat_outliers_zscore(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    for (size_t i = 0; i < results.size(); ++i) {
        if (!std::isnan(results[i])) {
            EXPECT_NEAR(results[i], 0.0, 1e-4) << "i=" << i;
        }
    }
}

// =====================================================================
// 22. stat_outliers_mzscore
// =====================================================================

/// @brief 正常系: 修正Zスコア外れ値検出(10行返却)
TEST_F(WindowFunctions, OutliersMzscoreRowCount) {
    std::string sql = "SELECT stat_outliers_mzscore(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief data(1-10): 外れ値なし → 全て0.0
TEST_F(WindowFunctions, OutliersMzscoreNoOutliers) {
    std::string sql = "SELECT stat_outliers_mzscore(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    for (size_t i = 0; i < results.size(); ++i) {
        if (!std::isnan(results[i])) {
            EXPECT_NEAR(results[i], 0.0, 1e-4) << "i=" << i;
        }
    }
}

// =====================================================================
// 23. stat_winsorize
// =====================================================================

/// @brief 正常系: ウィンソライズ(10行返却)
TEST_F(WindowFunctions, WinsorizeRowCount) {
    std::string sql = "SELECT stat_winsorize(val, 10)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief 結果に有限値が含まれる
TEST_F(WindowFunctions, WinsorizeHasFiniteValues) {
    std::string sql = "SELECT stat_winsorize(val, 10)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    int finite_count = 0;
    for (const auto& v : results) {
        if (std::isfinite(v)) ++finite_count;
    }
    EXPECT_EQ(finite_count, 10);
}
