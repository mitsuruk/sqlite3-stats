/**
 * @file two_column_aggregates_test.cpp
 * @brief Tests for the two-column aggregate functions (27 functions)
 *
 * Verifies correlation and covariance, weighted statistics, regression, paired
 * tests, prediction accuracy metrics and distance measures.
 */

#include "test_helpers.hpp"

/// @brief Fixture for the two-column aggregate function tests
class TwoColumnAggregates : public StatFuncTest {};

// =====================================================================
// 1. stat_population_covariance (population covariance)
// =====================================================================

/// @brief Normal case: population covariance of xy_data is finite
TEST_F(TwoColumnAggregates, PopulationCovarianceNormal) {
    double result = query_double(
        db_, "SELECT stat_population_covariance(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, PopulationCovarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_population_covariance(val, val) FROM empty_data"));
}

// =====================================================================
// 2. stat_covariance (unbiased covariance)
// =====================================================================

/// @brief Normal case: unbiased covariance of xy_data is finite
TEST_F(TwoColumnAggregates, CovarianceNormal) {
    double result =
        query_double(db_, "SELECT stat_covariance(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, CovarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_covariance(val, val) FROM empty_data"));
}

// =====================================================================
// 3. stat_pearson_r (Pearson correlation coefficient)
// =====================================================================

/// @brief Normal case: a positive correlation is expected for xy_data (range -1 to 1)
TEST_F(TwoColumnAggregates, PearsonRNormal) {
    double result =
        query_double(db_, "SELECT stat_pearson_r(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
    EXPECT_GT(result, 0.0) << "a positive correlation is expected";
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, PearsonREmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_pearson_r(val, val) FROM empty_data"));
}

// =====================================================================
// 4. stat_spearman_r (Spearman rank correlation coefficient)
// =====================================================================

/// @brief Normal case: within the range -1 to 1 for xy_data
TEST_F(TwoColumnAggregates, SpearmanRNormal) {
    double result =
        query_double(db_, "SELECT stat_spearman_r(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, SpearmanREmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_spearman_r(val, val) FROM empty_data"));
}

// =====================================================================
// 5. stat_kendall_tau (Kendall rank correlation coefficient)
// =====================================================================

/// @brief Normal case: within the range -1 to 1 for xy_data
TEST_F(TwoColumnAggregates, KendallTauNormal) {
    double result =
        query_double(db_, "SELECT stat_kendall_tau(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, KendallTauEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_kendall_tau(val, val) FROM empty_data"));
}

// =====================================================================
// 6. stat_weighted_covariance (weighted covariance)
// =====================================================================

/// @brief Normal case: weighted covariance of wt_data is finite
TEST_F(TwoColumnAggregates, WeightedCovarianceNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_covariance(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, WeightedCovarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_covariance(val, val) FROM empty_data"));
}

// =====================================================================
// 7. stat_weighted_mean (weighted mean)
// =====================================================================

/// @brief Normal case: wt_data -> sum(val*wt)/sum(wt) = 270/9 = 30.0
TEST_F(TwoColumnAggregates, WeightedMeanNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_mean(val, wt) FROM wt_data");
    EXPECT_NEAR(result, 30.0, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, WeightedMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_weighted_mean(val, val) FROM empty_data"));
}

// =====================================================================
// 8. stat_weighted_harmonic_mean (weighted harmonic mean)
// =====================================================================

/// @brief Normal case: a finite positive value for wt_data
TEST_F(TwoColumnAggregates, WeightedHarmonicMeanNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_harmonic_mean(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, WeightedHarmonicMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_harmonic_mean(val, val) FROM empty_data"));
}

// =====================================================================
// 9. stat_weighted_variance (weighted variance)
// =====================================================================

/// @brief Normal case: a finite positive value for wt_data
TEST_F(TwoColumnAggregates, WeightedVarianceNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_variance(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, WeightedVarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_variance(val, val) FROM empty_data"));
}

// =====================================================================
// 10. stat_weighted_stddev (weighted standard deviation)
// =====================================================================

/// @brief Normal case: a finite positive value for wt_data
TEST_F(TwoColumnAggregates, WeightedStddevNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_stddev(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, WeightedStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_stddev(val, val) FROM empty_data"));
}

// =====================================================================
// 11. stat_weighted_median (weighted median)
// =====================================================================

/// @brief Normal case: wt_data -> 30.0 (the most heavily weighted value is central)
TEST_F(TwoColumnAggregates, WeightedMedianNormal) {
    double result = query_double(
        db_, "SELECT stat_weighted_median(val, wt) FROM wt_data");
    EXPECT_NEAR(result, 30.0, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, WeightedMedianEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_median(val, val) FROM empty_data"));
}

// =====================================================================
// 12. stat_weighted_percentile (weighted percentile)
// =====================================================================

/// @brief Normal case: 50th percentile of wt_data -> equal or close to weighted_median
TEST_F(TwoColumnAggregates, WeightedPercentileNormal) {
    double result = query_double(
        db_,
        "SELECT stat_weighted_percentile(val, wt, 0.5) FROM wt_data");
    double median = query_double(
        db_, "SELECT stat_weighted_median(val, wt) FROM wt_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_NEAR(result, median, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, WeightedPercentileEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_weighted_percentile(val, val, 0.5) FROM empty_data"));
}

// =====================================================================
// 13. stat_simple_regression (simple linear regression)
// =====================================================================

/// @brief Normal case: xy_data -> JSON, positive slope, r_squared within 0-1
TEST_F(TwoColumnAggregates, SimpleRegressionNormal) {
    std::string result = query_text(
        db_, "SELECT stat_simple_regression(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty()) << "JSON should be returned";

    double slope = json_double(db_, result, "$.slope");
    EXPECT_GT(slope, 0.0) << "a positive slope is expected";

    double r_squared = json_double(db_, result, "$.r_squared");
    EXPECT_GE(r_squared, 0.0);
    EXPECT_LE(r_squared, 1.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, SimpleRegressionEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_simple_regression(val, val) FROM empty_data"));
}

// =====================================================================
// 14. stat_r_squared (coefficient of determination)
// =====================================================================

/// @brief Normal case: pred_data -> within the range 0-1
TEST_F(TwoColumnAggregates, RSquaredNormal) {
    double result = query_double(
        db_,
        "SELECT stat_r_squared(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, 0.0);
    EXPECT_LE(result, 1.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, RSquaredEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_r_squared(val, val) FROM empty_data"));
}

// =====================================================================
// 15. stat_adjusted_r_squared (adjusted coefficient of determination)
// =====================================================================

/// @brief Normal case: pred_data -> no greater than r_squared
TEST_F(TwoColumnAggregates, AdjustedRSquaredNormal) {
    double r_sq = query_double(
        db_,
        "SELECT stat_r_squared(actual, predicted) FROM pred_data");
    double adj_r_sq = query_double(
        db_,
        "SELECT stat_adjusted_r_squared(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(adj_r_sq)) << "result=" << adj_r_sq;
    EXPECT_LE(adj_r_sq, r_sq) << "adjusted R^2 should not exceed R^2";
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, AdjustedRSquaredEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_adjusted_r_squared(val, val) FROM empty_data"));
}

// =====================================================================
// 16. stat_t_test_paired (paired t-test)
// =====================================================================

/// @brief Normal case: xy_data -> JSON, df=9
TEST_F(TwoColumnAggregates, TTestPairedNormal) {
    std::string result = query_text(
        db_, "SELECT stat_t_test_paired(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty()) << "JSON should be returned";

    double statistic = json_double(db_, result, "$.statistic");
    EXPECT_TRUE(std::isfinite(statistic)) << "statistic=" << statistic;

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(p_value)) << "p_value=" << p_value;

    double df = json_double(db_, result, "$.df");
    EXPECT_NEAR(df, 9.0, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, TTestPairedEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_t_test_paired(val, val) FROM empty_data"));
}

// =====================================================================
// 17. stat_chisq_gof (chi-square goodness-of-fit test)
// =====================================================================

/// @brief Normal case: pred_data -> JSON, finite statistic and p_value
TEST_F(TwoColumnAggregates, ChisqGofNormal) {
    std::string result = query_text(
        db_,
        "SELECT stat_chisq_gof(actual, predicted) FROM pred_data");
    EXPECT_FALSE(result.empty()) << "JSON should be returned";

    double statistic = json_double(db_, result, "$.statistic");
    EXPECT_TRUE(std::isfinite(statistic)) << "statistic=" << statistic;

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(p_value)) << "p_value=" << p_value;
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, ChisqGofEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_chisq_gof(val, val) FROM empty_data"));
}

// =====================================================================
// 18. stat_mae (mean absolute error)
// =====================================================================

/// @brief Normal case: a finite positive value for pred_data
TEST_F(TwoColumnAggregates, MaeNormal) {
    double result = query_double(
        db_, "SELECT stat_mae(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, MaeEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mae(val, val) FROM empty_data"));
}

// =====================================================================
// 19. stat_mse (mean squared error)
// =====================================================================

/// @brief Normal case: a finite positive value for pred_data
TEST_F(TwoColumnAggregates, MseNormal) {
    double result = query_double(
        db_, "SELECT stat_mse(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, MseEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mse(val, val) FROM empty_data"));
}

// =====================================================================
// 20. stat_rmse (root mean squared error)
// =====================================================================

/// @brief Normal case: pred_data -> sqrt(mse)
TEST_F(TwoColumnAggregates, RmseNormal) {
    double mse = query_double(
        db_, "SELECT stat_mse(actual, predicted) FROM pred_data");
    double rmse = query_double(
        db_, "SELECT stat_rmse(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(rmse)) << "result=" << rmse;
    EXPECT_GT(rmse, 0.0);
    EXPECT_NEAR(rmse, std::sqrt(mse), 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, RmseEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_rmse(val, val) FROM empty_data"));
}

// =====================================================================
// 21. stat_mape (mean absolute percentage error)
// =====================================================================

/// @brief Normal case: a finite positive value (%) for pred_data
TEST_F(TwoColumnAggregates, MapeNormal) {
    double result = query_double(
        db_, "SELECT stat_mape(actual, predicted) FROM pred_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, MapeEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mape(val, val) FROM empty_data"));
}

// =====================================================================
// 22. stat_euclidean_dist (Euclidean distance)
// =====================================================================

/// @brief Normal case: a finite positive value for xy_data
TEST_F(TwoColumnAggregates, EuclideanDistNormal) {
    double result = query_double(
        db_, "SELECT stat_euclidean_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, EuclideanDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_euclidean_dist(val, val) FROM empty_data"));
}

/// @brief Identical vectors -> 0.0
TEST_F(TwoColumnAggregates, EuclideanDistSameVector) {
    double result = query_double(
        db_, "SELECT stat_euclidean_dist(x, x) FROM xy_data");
    EXPECT_NEAR(result, 0.0, 1e-4);
}

// =====================================================================
// 23. stat_manhattan_dist (Manhattan distance)
// =====================================================================

/// @brief Normal case: a finite positive value for xy_data
TEST_F(TwoColumnAggregates, ManhattanDistNormal) {
    double result = query_double(
        db_, "SELECT stat_manhattan_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, ManhattanDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_manhattan_dist(val, val) FROM empty_data"));
}

// =====================================================================
// 24. stat_cosine_sim (cosine similarity)
// =====================================================================

/// @brief Normal case: within -1 to 1 for xy_data (positive expected)
TEST_F(TwoColumnAggregates, CosineSimNormal) {
    double result = query_double(
        db_, "SELECT stat_cosine_sim(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
    EXPECT_GT(result, 0.0) << "a positive cosine similarity is expected";
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, CosineSimEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cosine_sim(val, val) FROM empty_data"));
}

/// @brief Identical vectors -> 1.0
TEST_F(TwoColumnAggregates, CosineSimSameVector) {
    double result = query_double(
        db_, "SELECT stat_cosine_sim(x, x) FROM xy_data");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

// =====================================================================
// 25. stat_cosine_dist (cosine distance)
// =====================================================================

/// @brief Normal case: xy_data -> 1 - cosine_sim
TEST_F(TwoColumnAggregates, CosineDistNormal) {
    double sim = query_double(
        db_, "SELECT stat_cosine_sim(x, y) FROM xy_data");
    double dist = query_double(
        db_, "SELECT stat_cosine_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(dist)) << "result=" << dist;
    EXPECT_NEAR(dist, 1.0 - sim, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, CosineDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cosine_dist(val, val) FROM empty_data"));
}

// =====================================================================
// 26. stat_minkowski_dist (Minkowski distance)
// =====================================================================

/// @brief Normal case: a finite positive value for xy_data with p=3
TEST_F(TwoColumnAggregates, MinkowskiDistNormal) {
    double result = query_double(
        db_, "SELECT stat_minkowski_dist(x, y, 3) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, MinkowskiDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_minkowski_dist(val, val, 3) FROM empty_data"));
}

// =====================================================================
// 27. stat_chebyshev_dist (Chebyshev distance)
// =====================================================================

/// @brief Normal case: a finite positive value for xy_data
TEST_F(TwoColumnAggregates, ChebyshevDistNormal) {
    double result = query_double(
        db_, "SELECT stat_chebyshev_dist(x, y) FROM xy_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(TwoColumnAggregates, ChebyshevDistEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_chebyshev_dist(val, val) FROM empty_data"));
}
