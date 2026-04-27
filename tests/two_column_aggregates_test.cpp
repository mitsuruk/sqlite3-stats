/**
 * @file two_column_aggregates_test.cpp
 * @brief 2カラム集約統計関数(27関数)のテスト
 *
 * 相関・共分散, 重み付き統計, 回帰, ペア検定,
 * 予測精度指標, 距離尺度の各関数を検証する.
 */

#include "test_helpers.hpp"

/// @brief 2カラム集約統計関数テスト用フィクスチャ
class TwoColumnAggregates : public StatFuncTest {};

// =====================================================================
// 1. stat_population_covariance (母共分散)
// =====================================================================

/// @brief 正常系: xy_dataの母共分散が有限値であること
TEST_F(TwoColumnAggregates, PopulationCovarianceNormal) {
    double result = query_double(
        db_, "SELECT stat_population_covariance(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, PopulationCovarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_population_covariance(val, val) FROM empty_data"));
}

// =====================================================================
// 2. stat_covariance (不偏共分散)
// =====================================================================

/// @brief 正常系: xy_dataの不偏共分散が有限値であること
TEST_F(TwoColumnAggregates, CovarianceNormal) {
    double result =
        query_double(db_, "SELECT stat_covariance(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, CovarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_covariance(val, val) FROM empty_data"));
}

// =====================================================================
// 3. stat_pearson_r (ピアソン相関係数)
// =====================================================================

/// @brief 正常系: xy_dataで正の相関が期待される(-1~1の範囲)
TEST_F(TwoColumnAggregates, PearsonRNormal) {
    double result =
        query_double(db_, "SELECT stat_pearson_r(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
    EXPECT_GT(result, 0.0) << "正の相関が期待される";
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, PearsonREmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_pearson_r(val, val) FROM empty_data"));
}

// =====================================================================
// 4. stat_spearman_r (スピアマン順位相関係数)
// =====================================================================

/// @brief 正常系: xy_dataで-1~1の範囲であること
TEST_F(TwoColumnAggregates, SpearmanRNormal) {
    double result =
        query_double(db_, "SELECT stat_spearman_r(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, SpearmanREmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_spearman_r(val, val) FROM empty_data"));
}

// =====================================================================
// 5. stat_kendall_tau (ケンドールの順位相関係数)
// =====================================================================

/// @brief 正常系: xy_dataで-1~1の範囲であること
TEST_F(TwoColumnAggregates, KendallTauNormal) {
    double result =
        query_double(db_, "SELECT stat_kendall_tau(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, KendallTauEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_kendall_tau(val, val) FROM empty_data"));
}

// =====================================================================
// 6. stat_weighted_covariance (重み付き共分散)
// =====================================================================

/// @brief 正常系: wt_dataの重み付き共分散が有限値であること
TEST_F(TwoColumnAggregates, WeightedCovarianceNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_covariance(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, WeightedCovarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_covariance(val, val) FROM empty_data"));
}

// =====================================================================
// 7. stat_weighted_mean (重み付き平均)
// =====================================================================

/// @brief 正常系: wt_data → sum(val*wt)/sum(wt) = 270/9 = 30.0
TEST_F(TwoColumnAggregates, WeightedMeanNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_mean(val, wt) FROM wt_data");
    EXPECT_NEAR(result, 30.0, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, WeightedMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_weighted_mean(val, val) FROM empty_data"));
}

// =====================================================================
// 8. stat_weighted_harmonic_mean (重み付き調和平均)
// =====================================================================

/// @brief 正常系: wt_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, WeightedHarmonicMeanNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_harmonic_mean(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, WeightedHarmonicMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_harmonic_mean(val, val) FROM empty_data"));
}

// =====================================================================
// 9. stat_weighted_variance (重み付き分散)
// =====================================================================

/// @brief 正常系: wt_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, WeightedVarianceNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_variance(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, WeightedVarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_variance(val, val) FROM empty_data"));
}

// =====================================================================
// 10. stat_weighted_stddev (重み付き標準偏差)
// =====================================================================

/// @brief 正常系: wt_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, WeightedStddevNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_stddev(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, WeightedStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_stddev(val, val) FROM empty_data"));
}

// =====================================================================
// 11. stat_weighted_median (重み付き中央値)
// =====================================================================

/// @brief 正常系: wt_data → 30.0 (重み最大の値が中央)
TEST_F(TwoColumnAggregates, WeightedMedianNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_median(val, wt) FROM wt_data");
    EXPECT_NEAR(result, 30.0, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, WeightedMedianEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_median(val, val) FROM empty_data"));
}

// =====================================================================
// 12. stat_weighted_percentile (重み付きパーセンタイル)
// =====================================================================

/// @brief 正常系: wt_dataの50パーセンタイル → weighted_medianと同じか近い値
TEST_F(TwoColumnAggregates, WeightedPercentileNormal) {
    double result = query_double(
        db_,
        "SELECT stat_weighted_percentile(val, wt, 0.5) FROM wt_data");
    double median = query_double(
        db_, "SELECT stat_weighted_median(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_NEAR(result, median, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, WeightedPercentileEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_percentile(val, val, 0.5) FROM empty_data"));
}

// =====================================================================
// 13. stat_simple_regression (単回帰)
// =====================================================================

/// @brief 正常系: xy_data → JSON, slopeが正, r_squaredが0-1の範囲
TEST_F(TwoColumnAggregates, SimpleRegressionNormal) {
    std::string result = query_text(
        db_, "SELECT stat_simple_regression(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty()) << "JSONが返却されるべき";

    double slope = json_double(db_, result, "$.slope");
    EXPECT_GT(slope, 0.0) << "正の傾きが期待される";

    double r_squared = json_double(db_, result, "$.r_squared");
    EXPECT_GE(r_squared, 0.0);
    EXPECT_LE(r_squared, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, SimpleRegressionEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_simple_regression(val, val) FROM empty_data"));
}

// =====================================================================
// 14. stat_r_squared (決定係数)
// =====================================================================

/// @brief 正常系: pred_data → 0-1の範囲
TEST_F(TwoColumnAggregates, RSquaredNormal) {
    double result = query_double(
        db_,
        "SELECT stat_r_squared(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, 0.0);
    EXPECT_LE(result, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, RSquaredEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_r_squared(val, val) FROM empty_data"));
}

// =====================================================================
// 15. stat_adjusted_r_squared (自由度調整済み決定係数)
// =====================================================================

/// @brief 正常系: pred_data → r_squared以下の値
TEST_F(TwoColumnAggregates, AdjustedRSquaredNormal) {
    double r_sq = query_double(
        db_,
        "SELECT stat_r_squared(actual, predicted) FROM pred_data");
    double adj_r_sq = query_double(
        db_,
        "SELECT stat_adjusted_r_squared(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(adj_r_sq)) << "result=" << adj_r_sq;
    EXPECT_LE(adj_r_sq, r_sq) << "自由度調整済みR^2はR^2以下であるべき";
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, AdjustedRSquaredEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_adjusted_r_squared(val, val) FROM empty_data"));
}

// =====================================================================
// 16. stat_t_test_paired (対応のあるt検定)
// =====================================================================

/// @brief 正常系: xy_data → JSON, df=9
TEST_F(TwoColumnAggregates, TTestPairedNormal) {
    std::string result = query_text(
        db_, "SELECT stat_t_test_paired(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty()) << "JSONが返却されるべき";

    double statistic = json_double(db_, result, "$.statistic");
    EXPECT_TRUE(std::isfinite(statistic)) << "statistic=" << statistic;

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(p_value)) << "p_value=" << p_value;

    double df = json_double(db_, result, "$.df");
    EXPECT_NEAR(df, 9.0, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, TTestPairedEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_t_test_paired(val, val) FROM empty_data"));
}

// =====================================================================
// 17. stat_chisq_gof (カイ二乗適合度検定)
// =====================================================================

/// @brief 正常系: pred_data → JSON, statistic/p_valueが有限値
TEST_F(TwoColumnAggregates, ChisqGofNormal) {
    std::string result = query_text(
        db_,
        "SELECT stat_chisq_gof(actual, predicted) FROM pred_data");
    EXPECT_FALSE(result.empty()) << "JSONが返却されるべき";

    double statistic = json_double(db_, result, "$.statistic");
    EXPECT_TRUE(std::isfinite(statistic)) << "statistic=" << statistic;

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(p_value)) << "p_value=" << p_value;
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, ChisqGofEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_chisq_gof(val, val) FROM empty_data"));
}

// =====================================================================
// 18. stat_mae (平均絶対誤差)
// =====================================================================

/// @brief 正常系: pred_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, MaeNormal) {
    double result = query_double(
        db_, "SELECT stat_mae(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, MaeEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mae(val, val) FROM empty_data"));
}

// =====================================================================
// 19. stat_mse (平均二乗誤差)
// =====================================================================

/// @brief 正常系: pred_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, MseNormal) {
    double result = query_double(
        db_, "SELECT stat_mse(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, MseEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mse(val, val) FROM empty_data"));
}

// =====================================================================
// 20. stat_rmse (二乗平均平方根誤差)
// =====================================================================

/// @brief 正常系: pred_data → sqrt(mse)
TEST_F(TwoColumnAggregates, RmseNormal) {
    double mse = query_double(
        db_, "SELECT stat_mse(actual, predicted) FROM pred_data");
    double rmse = query_double(
        db_, "SELECT stat_rmse(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(rmse)) << "result=" << rmse;
    EXPECT_GT(rmse, 0.0);
    EXPECT_NEAR(rmse, std::sqrt(mse), 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, RmseEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_rmse(val, val) FROM empty_data"));
}

// =====================================================================
// 21. stat_mape (平均絶対パーセント誤差)
// =====================================================================

/// @brief 正常系: pred_dataで有限の正の値(%)であること
TEST_F(TwoColumnAggregates, MapeNormal) {
    double result = query_double(
        db_, "SELECT stat_mape(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, MapeEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mape(val, val) FROM empty_data"));
}

// =====================================================================
// 22. stat_euclidean_dist (ユークリッド距離)
// =====================================================================

/// @brief 正常系: xy_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, EuclideanDistNormal) {
    double result = query_double(
        db_, "SELECT stat_euclidean_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, EuclideanDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_euclidean_dist(val, val) FROM empty_data"));
}

/// @brief 同一ベクトル → 0.0
TEST_F(TwoColumnAggregates, EuclideanDistSameVector) {
    double result = query_double(
        db_, "SELECT stat_euclidean_dist(x, x) FROM xy_data");
    EXPECT_NEAR(result, 0.0, 1e-4);
}

// =====================================================================
// 23. stat_manhattan_dist (マンハッタン距離)
// =====================================================================

/// @brief 正常系: xy_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, ManhattanDistNormal) {
    double result = query_double(
        db_, "SELECT stat_manhattan_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, ManhattanDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_manhattan_dist(val, val) FROM empty_data"));
}

// =====================================================================
// 24. stat_cosine_sim (コサイン類似度)
// =====================================================================

/// @brief 正常系: xy_dataで-1~1の範囲(正が期待される)
TEST_F(TwoColumnAggregates, CosineSimNormal) {
    double result = query_double(
        db_, "SELECT stat_cosine_sim(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
    EXPECT_GT(result, 0.0) << "正のコサイン類似度が期待される";
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, CosineSimEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cosine_sim(val, val) FROM empty_data"));
}

/// @brief 同一ベクトル → 1.0
TEST_F(TwoColumnAggregates, CosineSimSameVector) {
    double result = query_double(
        db_, "SELECT stat_cosine_sim(x, x) FROM xy_data");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

// =====================================================================
// 25. stat_cosine_dist (コサイン距離)
// =====================================================================

/// @brief 正常系: xy_data → 1 - cosine_sim
TEST_F(TwoColumnAggregates, CosineDistNormal) {
    double sim = query_double(
        db_, "SELECT stat_cosine_sim(x, y) FROM xy_data");
    double dist = query_double(
        db_, "SELECT stat_cosine_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(dist)) << "result=" << dist;
    EXPECT_NEAR(dist, 1.0 - sim, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, CosineDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cosine_dist(val, val) FROM empty_data"));
}

// =====================================================================
// 26. stat_minkowski_dist (ミンコフスキー距離)
// =====================================================================

/// @brief 正常系: xy_data, p=3 で有限の正の値であること
TEST_F(TwoColumnAggregates, MinkowskiDistNormal) {
    double result = query_double(
        db_, "SELECT stat_minkowski_dist(x, y, 3) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, MinkowskiDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_minkowski_dist(val, val, 3) FROM empty_data"));
}

// =====================================================================
// 27. stat_chebyshev_dist (チェビシェフ距離)
// =====================================================================

/// @brief 正常系: xy_dataで有限の正の値であること
TEST_F(TwoColumnAggregates, ChebyshevDistNormal) {
    double result = query_double(
        db_, "SELECT stat_chebyshev_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(TwoColumnAggregates, ChebyshevDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_chebyshev_dist(val, val) FROM empty_data"));
}
