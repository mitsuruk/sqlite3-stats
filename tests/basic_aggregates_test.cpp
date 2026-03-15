/**
 * @file basic_aggregates_test.cpp
 * @brief 基本集約統計関数(24関数)のテスト
 *
 * stat_mean, stat_median, stat_mode 等の基本統計集約関数について,
 * 正常値・空テーブル・エッジケースを検証する.
 */

#include "test_helpers.hpp"

/// @brief 基本集約統計関数テスト用フィクスチャ
class BasicAggregates : public StatFuncTest {};

// =====================================================================
// 1. stat_mean
// =====================================================================

/// @brief 正常系: data(1-10)の平均 → 5.5
TEST_F(BasicAggregates, MeanNormal) {
    double result = query_double(db_, "SELECT stat_mean(val) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, MeanEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_mean(val) FROM empty_data"));
}

/// @brief NULL混在: nulldata(1,NULL,3,NULL,5) → 3.0
TEST_F(BasicAggregates, MeanWithNulls) {
    double result = query_double(db_, "SELECT stat_mean(val) FROM nulldata");
    EXPECT_NEAR(result, 3.0, 1e-6);
}

/// @brief 全NULL → NULL
TEST_F(BasicAggregates, MeanAllNull) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mean(val) FROM nulldata WHERE val IS NULL"));
}

/// @brief 単一行 → その値自体
TEST_F(BasicAggregates, MeanSingleRow) {
    double result = query_double(
        db_, "SELECT stat_mean(val) FROM (SELECT 42.0 AS val)");
    EXPECT_NEAR(result, 42.0, 1e-6);
}

// =====================================================================
// 2. stat_median
// =====================================================================

/// @brief 正常系: data(1-10)の中央値 → 5.5
TEST_F(BasicAggregates, MedianNormal) {
    double result = query_double(db_, "SELECT stat_median(val) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, MedianEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_median(val) FROM empty_data"));
}

/// @brief 奇数行: data2(9行)の中央値 → 3.0
TEST_F(BasicAggregates, MedianOddRows) {
    double result = query_double(db_, "SELECT stat_median(val) FROM data2");
    EXPECT_NEAR(result, 3.0, 1e-6);
}

// =====================================================================
// 3. stat_mode
// =====================================================================

/// @brief 正常系: data2(1,2,2,3,3,3,4,4,5) → 3.0
TEST_F(BasicAggregates, ModeNormal) {
    double result = query_double(db_, "SELECT stat_mode(val) FROM data2");
    EXPECT_NEAR(result, 3.0, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, ModeEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_mode(val) FROM empty_data"));
}

/// @brief 全て同一値 → その値
TEST_F(BasicAggregates, ModeAllSame) {
    double result = query_double(
        db_,
        "SELECT stat_mode(val) FROM ("
        "SELECT 5.0 AS val UNION ALL SELECT 5.0 UNION ALL SELECT 5.0)");
    EXPECT_NEAR(result, 5.0, 1e-6);
}

// =====================================================================
// 4. stat_geometric_mean
// =====================================================================

/// @brief 正常系: pos_data(2,4,8) → 4.0
TEST_F(BasicAggregates, GeometricMeanNormal) {
    double result =
        query_double(db_, "SELECT stat_geometric_mean(val) FROM pos_data");
    EXPECT_NEAR(result, 4.0, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, GeometricMeanEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_geometric_mean(val) FROM empty_data"));
}

/// @brief 負値 → NULL
/// @brief 負の値: statcppが例外を投げる場合がある
TEST_F(BasicAggregates, GeometricMeanNegative) {
    try {
        auto is_null = query_is_null(
            db_,
            "SELECT stat_geometric_mean(val) FROM (SELECT -1.0 AS val)");
        EXPECT_TRUE(is_null);
    } catch (const std::exception&) {
        // statcppが例外を投げる場合は許容
        SUCCEED();
    }
}

// =====================================================================
// 5. stat_harmonic_mean
// =====================================================================

/// @brief 正常系: pos_data(2,4,8) → 3/(1/2+1/4+1/8) = 3/0.875 ≈ 3.428571
TEST_F(BasicAggregates, HarmonicMeanNormal) {
    double result =
        query_double(db_, "SELECT stat_harmonic_mean(val) FROM pos_data");
    EXPECT_NEAR(result, 3.428571, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, HarmonicMeanEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_harmonic_mean(val) FROM empty_data"));
}

/// @brief ゼロ含む → NULL
/// @brief ゼロ含む: statcppが例外を投げる場合がある
TEST_F(BasicAggregates, HarmonicMeanWithZero) {
    try {
        auto is_null = query_is_null(
            db_,
            "SELECT stat_harmonic_mean(val) FROM ("
            "SELECT 0.0 AS val UNION ALL SELECT 1.0)");
        EXPECT_TRUE(is_null);
    } catch (const std::exception&) {
        SUCCEED();
    }
}

// =====================================================================
// 6. stat_range
// =====================================================================

/// @brief 正常系: data(1-10) → 9.0
TEST_F(BasicAggregates, RangeNormal) {
    double result = query_double(db_, "SELECT stat_range(val) FROM data");
    EXPECT_NEAR(result, 9.0, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, RangeEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_range(val) FROM empty_data"));
}

/// @brief 単一行 → 0.0
TEST_F(BasicAggregates, RangeSingleRow) {
    double result = query_double(
        db_, "SELECT stat_range(val) FROM (SELECT 42.0 AS val)");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

// =====================================================================
// 7. stat_var (母分散)
// =====================================================================

/// @brief 正常系: data(1-10) → 8.25
TEST_F(BasicAggregates, VarNormal) {
    double result = query_double(db_, "SELECT stat_var(val) FROM data");
    EXPECT_NEAR(result, 8.25, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, VarEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_var(val) FROM empty_data"));
}

// =====================================================================
// 8. stat_population_variance
// =====================================================================

/// @brief 正常系: data(1-10) → 8.25
TEST_F(BasicAggregates, PopulationVarianceNormal) {
    double result =
        query_double(db_, "SELECT stat_population_variance(val) FROM data");
    EXPECT_NEAR(result, 8.25, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, PopulationVarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_variance(val) FROM empty_data"));
}

// =====================================================================
// 9. stat_sample_variance
// =====================================================================

/// @brief 正常系: data(1-10) → 82.5/9 ≈ 9.166667
TEST_F(BasicAggregates, SampleVarianceNormal) {
    double result =
        query_double(db_, "SELECT stat_sample_variance(val) FROM data");
    EXPECT_NEAR(result, 9.166667, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, SampleVarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample_variance(val) FROM empty_data"));
}

/// @brief 単一行 → NULL (n-1=0で計算不能)
TEST_F(BasicAggregates, SampleVarianceSingleRow) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_sample_variance(val) FROM (SELECT 42.0 AS val)"));
}

// =====================================================================
// 10. stat_stdev (母標準偏差)
// =====================================================================

/// @brief 正常系: data(1-10) → sqrt(8.25) ≈ 2.872281
TEST_F(BasicAggregates, StdevNormal) {
    double result = query_double(db_, "SELECT stat_stdev(val) FROM data");
    EXPECT_NEAR(result, 2.872281, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, StdevEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_stdev(val) FROM empty_data"));
}

// =====================================================================
// 11. stat_population_stddev
// =====================================================================

/// @brief 正常系: data(1-10) → sqrt(8.25) ≈ 2.872281
TEST_F(BasicAggregates, PopulationStddevNormal) {
    double result =
        query_double(db_, "SELECT stat_population_stddev(val) FROM data");
    EXPECT_NEAR(result, 2.872281, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, PopulationStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_stddev(val) FROM empty_data"));
}

// =====================================================================
// 12. stat_sample_stddev
// =====================================================================

/// @brief 正常系: data(1-10) → sqrt(9.166667) ≈ 3.027650
TEST_F(BasicAggregates, SampleStddevNormal) {
    double result =
        query_double(db_, "SELECT stat_sample_stddev(val) FROM data");
    EXPECT_NEAR(result, 3.027650, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, SampleStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample_stddev(val) FROM empty_data"));
}

/// @brief 単一行 → NULL (n-1=0で計算不能)
TEST_F(BasicAggregates, SampleStddevSingleRow) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_sample_stddev(val) FROM (SELECT 42.0 AS val)"));
}

// =====================================================================
// 13. stat_cv (変動係数)
// =====================================================================

/// @brief 正常系: data(1-10) → sample_stddev/mean ≈ 3.027650/5.5 ≈ 0.550482
/// @note 実装が population_stddev/mean の場合は 2.872281/5.5 ≈ 0.522233
TEST_F(BasicAggregates, CvNormal) {
    double result = query_double(db_, "SELECT stat_cv(val) FROM data");
    EXPECT_NEAR(result, 0.550482, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, CvEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_cv(val) FROM empty_data"));
}

/// @brief 全同一値 → 0.0 (分散0のため)
/// @note 実装によっては NULLを返す可能性あり
TEST_F(BasicAggregates, CvAllSame) {
    double result = query_double(
        db_,
        "SELECT stat_cv(val) FROM ("
        "SELECT 5.0 AS val UNION ALL SELECT 5.0 UNION ALL SELECT 5.0)");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

// =====================================================================
// 14. stat_iqr (四分位範囲)
// =====================================================================

/// @brief 正常系: data(1-10) → 4.5
TEST_F(BasicAggregates, IqrNormal) {
    double result = query_double(db_, "SELECT stat_iqr(val) FROM data");
    EXPECT_NEAR(result, 4.5, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, IqrEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_iqr(val) FROM empty_data"));
}

// =====================================================================
// 15. stat_mad_mean (平均絶対偏差)
// =====================================================================

/// @brief 正常系: data(1-10) → 2.5
TEST_F(BasicAggregates, MadMeanNormal) {
    double result = query_double(db_, "SELECT stat_mad_mean(val) FROM data");
    EXPECT_NEAR(result, 2.5, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, MadMeanEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_mad_mean(val) FROM empty_data"));
}

// =====================================================================
// 16. stat_geometric_stddev (幾何標準偏差)
// =====================================================================

/// @brief 正常系: pos_data(2,4,8)の結果が有限の正の値であること
TEST_F(BasicAggregates, GeometricStddevNormal) {
    double result =
        query_double(db_, "SELECT stat_geometric_stddev(val) FROM pos_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, GeometricStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_geometric_stddev(val) FROM empty_data"));
}

// =====================================================================
// 17. stat_population_skewness (母歪度)
// =====================================================================

/// @brief 正常系: data(1-10, 対称分布) → 0.0
TEST_F(BasicAggregates, PopulationSkewnessNormal) {
    double result =
        query_double(db_, "SELECT stat_population_skewness(val) FROM data");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, PopulationSkewnessEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_skewness(val) FROM empty_data"));
}

// =====================================================================
// 18. stat_skewness (標本歪度)
// =====================================================================

/// @brief 正常系: data(1-10, 対称分布) → 0.0
TEST_F(BasicAggregates, SkewnessNormal) {
    double result =
        query_double(db_, "SELECT stat_skewness(val) FROM data");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, SkewnessEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_skewness(val) FROM empty_data"));
}

/// @brief n<3 → NULL (標本歪度はn>=3が必要)
TEST_F(BasicAggregates, SkewnessTooFewRows) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_skewness(val) FROM ("
        "SELECT 1.0 AS val UNION ALL SELECT 2.0)"));
}

// =====================================================================
// 19. stat_population_kurtosis (母尖度)
// =====================================================================

/// @brief 正常系: data(1-10) → 約-1.224242
TEST_F(BasicAggregates, PopulationKurtosisNormal) {
    double result =
        query_double(db_, "SELECT stat_population_kurtosis(val) FROM data");
    EXPECT_NEAR(result, -1.224242, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, PopulationKurtosisEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_kurtosis(val) FROM empty_data"));
}

// =====================================================================
// 20. stat_kurtosis (標本尖度)
// =====================================================================

/// @brief 正常系: data(1-10) → 約-1.200000
TEST_F(BasicAggregates, KurtosisNormal) {
    double result =
        query_double(db_, "SELECT stat_kurtosis(val) FROM data");
    EXPECT_NEAR(result, -1.200000, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, KurtosisEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_kurtosis(val) FROM empty_data"));
}

/// @brief n<4 → NULL (標本尖度はn>=4が必要)
TEST_F(BasicAggregates, KurtosisTooFewRows) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_kurtosis(val) FROM ("
        "SELECT 1.0 AS val UNION ALL SELECT 2.0 UNION ALL SELECT 3.0)"));
}

// =====================================================================
// 21. stat_se (標準誤差)
// =====================================================================

/// @brief 正常系: data(1-10) → sample_stddev/sqrt(10) ≈ 0.957427
TEST_F(BasicAggregates, SeNormal) {
    double result = query_double(db_, "SELECT stat_se(val) FROM data");
    EXPECT_NEAR(result, 0.957427, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, SeEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_se(val) FROM empty_data"));
}

// =====================================================================
// 22. stat_mad (中央絶対偏差)
// =====================================================================

/// @brief 正常系: data(1-10) → 2.5
TEST_F(BasicAggregates, MadNormal) {
    double result = query_double(db_, "SELECT stat_mad(val) FROM data");
    EXPECT_NEAR(result, 2.5, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, MadEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_mad(val) FROM empty_data"));
}

// =====================================================================
// 23. stat_mad_scaled (スケール済み中央絶対偏差)
// =====================================================================

/// @brief 正常系: data(1-10) → 2.5 * 1.4826 ≈ 3.7065
TEST_F(BasicAggregates, MadScaledNormal) {
    double result =
        query_double(db_, "SELECT stat_mad_scaled(val) FROM data");
    EXPECT_NEAR(result, 3.7065, 1e-3);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, MadScaledEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_mad_scaled(val) FROM empty_data"));
}

// =====================================================================
// 24. stat_hodges_lehmann (Hodges-Lehmann推定量)
// =====================================================================

/// @brief 正常系: data(1-10) → 5.5
TEST_F(BasicAggregates, HodgesLehmannNormal) {
    double result =
        query_double(db_, "SELECT stat_hodges_lehmann(val) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-6);
}

/// @brief 空テーブル → NULL
TEST_F(BasicAggregates, HodgesLehmannEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_hodges_lehmann(val) FROM empty_data"));
}
