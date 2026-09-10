/**
 * @file basic_aggregates_test.cpp
 * @brief Tests for the basic aggregate functions (24 functions)
 *
 * Verifies normal values, empty tables and edge cases for the basic aggregates
 * such as stat_mean, stat_median and stat_mode.
 */

#include "test_helpers.hpp"

/// @brief Fixture for the basic aggregate function tests
class BasicAggregates : public StatFuncTest {};

// =====================================================================
// 1. stat_mean
// =====================================================================

/// @brief Normal case: mean of data(1-10) -> 5.5
TEST_F(BasicAggregates, MeanNormal) {
    double result = query_double(db_, "SELECT stat_mean(val) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, MeanEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_mean(val) FROM empty_data"));
}

/// @brief Mixed NULLs: nulldata(1,NULL,3,NULL,5) -> 3.0
TEST_F(BasicAggregates, MeanWithNulls) {
    double result = query_double(db_, "SELECT stat_mean(val) FROM nulldata");
    EXPECT_NEAR(result, 3.0, 1e-6);
}

/// @brief All NULL -> NULL
TEST_F(BasicAggregates, MeanAllNull) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mean(val) FROM nulldata WHERE val IS NULL"));
}

/// @brief Single row -> the value itself
TEST_F(BasicAggregates, MeanSingleRow) {
    double result = query_double(
        db_, "SELECT stat_mean(val) FROM (SELECT 42.0 AS val)");
    EXPECT_NEAR(result, 42.0, 1e-6);
}

// =====================================================================
// 2. stat_median
// =====================================================================

/// @brief Normal case: median of data(1-10) -> 5.5
TEST_F(BasicAggregates, MedianNormal) {
    double result = query_double(db_, "SELECT stat_median(val) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, MedianEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_median(val) FROM empty_data"));
}

/// @brief Odd row count: median of data2 (9 rows) -> 3.0
TEST_F(BasicAggregates, MedianOddRows) {
    double result = query_double(db_, "SELECT stat_median(val) FROM data2");
    EXPECT_NEAR(result, 3.0, 1e-6);
}

// =====================================================================
// 3. stat_mode
// =====================================================================

/// @brief Normal case: data2(1,2,2,3,3,3,4,4,5) -> 3.0
TEST_F(BasicAggregates, ModeNormal) {
    double result = query_double(db_, "SELECT stat_mode(val) FROM data2");
    EXPECT_NEAR(result, 3.0, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, ModeEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_mode(val) FROM empty_data"));
}

/// @brief All values identical -> that value
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

/// @brief Normal case: pos_data(2,4,8) -> 4.0
TEST_F(BasicAggregates, GeometricMeanNormal) {
    double result =
        query_double(db_, "SELECT stat_geometric_mean(val) FROM pos_data");
    EXPECT_NEAR(result, 4.0, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, GeometricMeanEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_geometric_mean(val) FROM empty_data"));
}

/// @brief Negative value -> NULL
/// @brief Negative values: statcpp may throw instead
TEST_F(BasicAggregates, GeometricMeanNegative) {
    try {
        auto is_null = query_is_null(
            db_,
            "SELECT stat_geometric_mean(val) FROM (SELECT -1.0 AS val)");
        EXPECT_TRUE(is_null);
    } catch (const std::exception&) {
        // Accept the case where statcpp throws
        SUCCEED();
    }
}

// =====================================================================
// 5. stat_harmonic_mean
// =====================================================================

/// @brief Normal case: pos_data(2,4,8) -> 3/(1/2+1/4+1/8) = 3/0.875 ~ 3.428571
TEST_F(BasicAggregates, HarmonicMeanNormal) {
    double result =
        query_double(db_, "SELECT stat_harmonic_mean(val) FROM pos_data");
    EXPECT_NEAR(result, 3.428571, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, HarmonicMeanEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_harmonic_mean(val) FROM empty_data"));
}

/// @brief Contains zero -> NULL
/// @brief Contains zero: statcpp may throw instead
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

/// @brief Normal case: data(1-10) -> 9.0
TEST_F(BasicAggregates, RangeNormal) {
    double result = query_double(db_, "SELECT stat_range(val) FROM data");
    EXPECT_NEAR(result, 9.0, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, RangeEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_range(val) FROM empty_data"));
}

/// @brief Single row -> 0.0
TEST_F(BasicAggregates, RangeSingleRow) {
    double result = query_double(
        db_, "SELECT stat_range(val) FROM (SELECT 42.0 AS val)");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

// =====================================================================
// 7. stat_var (population variance)
// =====================================================================

/// @brief Normal case: data(1-10) -> 8.25
TEST_F(BasicAggregates, VarNormal) {
    double result = query_double(db_, "SELECT stat_var(val) FROM data");
    EXPECT_NEAR(result, 8.25, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, VarEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_var(val) FROM empty_data"));
}

// =====================================================================
// 8. stat_population_variance
// =====================================================================

/// @brief Normal case: data(1-10) -> 8.25
TEST_F(BasicAggregates, PopulationVarianceNormal) {
    double result =
        query_double(db_, "SELECT stat_population_variance(val) FROM data");
    EXPECT_NEAR(result, 8.25, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, PopulationVarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_variance(val) FROM empty_data"));
}

// =====================================================================
// 9. stat_sample_variance
// =====================================================================

/// @brief Normal case: data(1-10) -> 82.5/9 ~ 9.166667
TEST_F(BasicAggregates, SampleVarianceNormal) {
    double result =
        query_double(db_, "SELECT stat_sample_variance(val) FROM data");
    EXPECT_NEAR(result, 9.166667, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, SampleVarianceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample_variance(val) FROM empty_data"));
}

/// @brief Single row -> NULL (n-1=0, so not computable)
TEST_F(BasicAggregates, SampleVarianceSingleRow) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_sample_variance(val) FROM (SELECT 42.0 AS val)"));
}

// =====================================================================
// 10. stat_stdev (population standard deviation)
// =====================================================================

/// @brief Normal case: data(1-10) -> sqrt(8.25) ~ 2.872281
TEST_F(BasicAggregates, StdevNormal) {
    double result = query_double(db_, "SELECT stat_stdev(val) FROM data");
    EXPECT_NEAR(result, 2.872281, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, StdevEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_stdev(val) FROM empty_data"));
}

// =====================================================================
// 11. stat_population_stddev
// =====================================================================

/// @brief Normal case: data(1-10) -> sqrt(8.25) ~ 2.872281
TEST_F(BasicAggregates, PopulationStddevNormal) {
    double result =
        query_double(db_, "SELECT stat_population_stddev(val) FROM data");
    EXPECT_NEAR(result, 2.872281, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, PopulationStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_stddev(val) FROM empty_data"));
}

// =====================================================================
// 12. stat_sample_stddev
// =====================================================================

/// @brief Normal case: data(1-10) -> sqrt(9.166667) ~ 3.027650
TEST_F(BasicAggregates, SampleStddevNormal) {
    double result =
        query_double(db_, "SELECT stat_sample_stddev(val) FROM data");
    EXPECT_NEAR(result, 3.027650, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, SampleStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample_stddev(val) FROM empty_data"));
}

/// @brief Single row -> NULL (n-1=0, so not computable)
TEST_F(BasicAggregates, SampleStddevSingleRow) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_sample_stddev(val) FROM (SELECT 42.0 AS val)"));
}

// =====================================================================
// 13. stat_cv (coefficient of variation)
// =====================================================================

/// @brief Normal case: data(1-10) -> sample_stddev/mean ~ 3.027650/5.5 ~ 0.550482
/// @note If the implementation uses population_stddev/mean it is 2.872281/5.5 ~ 0.522233
TEST_F(BasicAggregates, CvNormal) {
    double result = query_double(db_, "SELECT stat_cv(val) FROM data");
    EXPECT_NEAR(result, 0.550482, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, CvEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_cv(val) FROM empty_data"));
}

/// @brief All values identical -> 0.0 (variance is zero)
/// @note Some implementations may return NULL instead
TEST_F(BasicAggregates, CvAllSame) {
    double result = query_double(
        db_,
        "SELECT stat_cv(val) FROM ("
        "SELECT 5.0 AS val UNION ALL SELECT 5.0 UNION ALL SELECT 5.0)");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

// =====================================================================
// 14. stat_iqr (interquartile range)
// =====================================================================

/// @brief Normal case: data(1-10) -> 4.5
TEST_F(BasicAggregates, IqrNormal) {
    double result = query_double(db_, "SELECT stat_iqr(val) FROM data");
    EXPECT_NEAR(result, 4.5, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, IqrEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_iqr(val) FROM empty_data"));
}

// =====================================================================
// 15. stat_mad_mean (mean absolute deviation)
// =====================================================================

/// @brief Normal case: data(1-10) -> 2.5
TEST_F(BasicAggregates, MadMeanNormal) {
    double result = query_double(db_, "SELECT stat_mad_mean(val) FROM data");
    EXPECT_NEAR(result, 2.5, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, MadMeanEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_mad_mean(val) FROM empty_data"));
}

// =====================================================================
// 16. stat_geometric_stddev (geometric standard deviation)
// =====================================================================

/// @brief Normal case: pos_data(2,4,8) gives a finite positive value
TEST_F(BasicAggregates, GeometricStddevNormal) {
    double result =
        query_double(db_, "SELECT stat_geometric_stddev(val) FROM pos_data");
    EXPECT_TRUE(std::isfinite(result)) << "result=" << result;
    EXPECT_GT(result, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, GeometricStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_geometric_stddev(val) FROM empty_data"));
}

// =====================================================================
// 17. stat_population_skewness (population skewness)
// =====================================================================

/// @brief Normal case: data(1-10), a symmetric distribution -> 0.0
TEST_F(BasicAggregates, PopulationSkewnessNormal) {
    double result =
        query_double(db_, "SELECT stat_population_skewness(val) FROM data");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, PopulationSkewnessEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_skewness(val) FROM empty_data"));
}

// =====================================================================
// 18. stat_skewness (sample skewness)
// =====================================================================

/// @brief Normal case: data(1-10), a symmetric distribution -> 0.0
TEST_F(BasicAggregates, SkewnessNormal) {
    double result =
        query_double(db_, "SELECT stat_skewness(val) FROM data");
    EXPECT_NEAR(result, 0.0, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, SkewnessEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_skewness(val) FROM empty_data"));
}

/// @brief n<3 -> NULL (sample skewness needs n>=3)
TEST_F(BasicAggregates, SkewnessTooFewRows) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_skewness(val) FROM ("
        "SELECT 1.0 AS val UNION ALL SELECT 2.0)"));
}

// =====================================================================
// 19. stat_population_kurtosis (population kurtosis)
// =====================================================================

/// @brief Normal case: data(1-10) -> about -1.224242
TEST_F(BasicAggregates, PopulationKurtosisNormal) {
    double result =
        query_double(db_, "SELECT stat_population_kurtosis(val) FROM data");
    EXPECT_NEAR(result, -1.224242, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, PopulationKurtosisEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_population_kurtosis(val) FROM empty_data"));
}

// =====================================================================
// 20. stat_kurtosis (sample kurtosis)
// =====================================================================

/// @brief Normal case: data(1-10) -> about -1.200000
TEST_F(BasicAggregates, KurtosisNormal) {
    double result =
        query_double(db_, "SELECT stat_kurtosis(val) FROM data");
    EXPECT_NEAR(result, -1.200000, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, KurtosisEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_kurtosis(val) FROM empty_data"));
}

/// @brief n<4 -> NULL (sample kurtosis needs n>=4)
TEST_F(BasicAggregates, KurtosisTooFewRows) {
    EXPECT_TRUE(query_is_null(
        db_,
        "SELECT stat_kurtosis(val) FROM ("
        "SELECT 1.0 AS val UNION ALL SELECT 2.0 UNION ALL SELECT 3.0)"));
}

// =====================================================================
// 21. stat_se (standard error)
// =====================================================================

/// @brief Normal case: data(1-10) -> sample_stddev/sqrt(10) ~ 0.957427
TEST_F(BasicAggregates, SeNormal) {
    double result = query_double(db_, "SELECT stat_se(val) FROM data");
    EXPECT_NEAR(result, 0.957427, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, SeEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_se(val) FROM empty_data"));
}

// =====================================================================
// 22. stat_mad (median absolute deviation)
// =====================================================================

/// @brief Normal case: data(1-10) -> 2.5
TEST_F(BasicAggregates, MadNormal) {
    double result = query_double(db_, "SELECT stat_mad(val) FROM data");
    EXPECT_NEAR(result, 2.5, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, MadEmpty) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_mad(val) FROM empty_data"));
}

// =====================================================================
// 23. stat_mad_scaled (scaled median absolute deviation)
// =====================================================================

/// @brief Normal case: data(1-10) -> 2.5 * 1.4826 ~ 3.7065
TEST_F(BasicAggregates, MadScaledNormal) {
    double result =
        query_double(db_, "SELECT stat_mad_scaled(val) FROM data");
    EXPECT_NEAR(result, 3.7065, 1e-3);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, MadScaledEmpty) {
    EXPECT_TRUE(
        query_is_null(db_, "SELECT stat_mad_scaled(val) FROM empty_data"));
}

// =====================================================================
// 24. stat_hodges_lehmann (Hodges-Lehmann estimator)
// =====================================================================

/// @brief Normal case: data(1-10) -> 5.5
TEST_F(BasicAggregates, HodgesLehmannNormal) {
    double result =
        query_double(db_, "SELECT stat_hodges_lehmann(val) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-6);
}

/// @brief Empty table -> NULL
TEST_F(BasicAggregates, HodgesLehmannEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_hodges_lehmann(val) FROM empty_data"));
}
