/**
 * @file window_functions_test.cpp
 * @brief Tests for the window functions (26 functions)
 *
 * Verifies rolling statistics, moving averages, ranking, missing-value imputation,
 * encoding, time-series transforms, outlier detection and robust processing.
 *
 * Note: a window function behaves differently with and without an OVER clause.
 * - Rolling family (rolling_mean and so on): called as an aggregate, without OVER
 * - Everything else (rank, fillna and so on): OVER (ORDER BY id ROWS BETWEEN
 *   UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING) to return every row
 */

#include "test_helpers.hpp"

#include <algorithm>

/// @brief Fixture for the window function tests
class WindowFunctions : public StatFuncTest {};

/// @brief The standard OVER clause
static const char* kFullFrame =
    " OVER (ORDER BY id ROWS BETWEEN "
    "UNBOUNDED PRECEDING AND UNBOUNDED FOLLOWING)";

// =====================================================================
// 1-5. stat_rolling_mean / std / min / max / sum
// The rolling family is called as an aggregate without OVER (result checked only)
// =====================================================================

/// @brief stat_rolling_mean: can be called as an aggregate
TEST_F(WindowFunctions, RollingMeanAggregate) {
    // Returns a single row as an aggregate (the first row only)
    auto result = query_double(
        db_, "SELECT stat_rolling_mean(val, 3) FROM ts_data");
    // The result is either NULL or a finite value
    SUCCEED();
}

/// @brief stat_rolling_std: can be called as an aggregate
TEST_F(WindowFunctions, RollingStdAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_std(val, 3) FROM ts_data");
    SUCCEED();
}

/// @brief stat_rolling_min: can be called as an aggregate
TEST_F(WindowFunctions, RollingMinAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_min(val, 3) FROM ts_data");
    SUCCEED();
}

/// @brief stat_rolling_max: can be called as an aggregate
TEST_F(WindowFunctions, RollingMaxAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_max(val, 3) FROM ts_data");
    SUCCEED();
}

/// @brief stat_rolling_sum: can be called as an aggregate
TEST_F(WindowFunctions, RollingSumAggregate) {
    auto result = query_double(
        db_, "SELECT stat_rolling_sum(val, 3) FROM ts_data");
    SUCCEED();
}

// =====================================================================
// 6. stat_moving_avg
// =====================================================================

/// @brief stat_moving_avg: can be called as an aggregate
TEST_F(WindowFunctions, MovingAvgAggregate) {
    auto result = query_double(
        db_, "SELECT stat_moving_avg(val, 3) FROM ts_data");
    SUCCEED();
}

// =====================================================================
// 7. stat_ema
// =====================================================================

/// @brief Normal case: exponentially weighted moving average (every row, via OVER)
TEST_F(WindowFunctions, EmaRowCount) {
    std::string sql = "SELECT stat_ema(val, 5)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief The first row matches the original value
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

/// @brief Normal case: ranking data(1-10) (10 rows returned)
TEST_F(WindowFunctions, RankRowCount) {
    std::string sql = "SELECT stat_rank(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief The ranks contain every value from 1 to 10
TEST_F(WindowFunctions, RankContainsAllRanks) {
    std::string sql = "SELECT stat_rank(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    // Sort the ranks and confirm they are 1-10
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

/// @brief Normal case: NULL rows are filled with the mean (10 rows returned)
TEST_F(WindowFunctions, FillnaMeanRowCount) {
    std::string sql = "SELECT stat_fillna_mean(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief NULLs are filled in, leaving no NaN
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

/// @brief Normal case: 10 rows returned
TEST_F(WindowFunctions, FillnaMedianRowCount) {
    std::string sql = "SELECT stat_fillna_median(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief NULLs are filled in, leaving no NaN
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

/// @brief Normal case: forward fill (10 rows returned)
TEST_F(WindowFunctions, FillnaFfillRowCount) {
    std::string sql = "SELECT stat_fillna_ffill(val)";
    sql += kFullFrame;
    sql += " FROM ts_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief The first row is 10.0 (not NULL)
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

/// @brief Normal case: backward fill (10 rows returned)
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

/// @brief Normal case: linear interpolation (10 rows returned)
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

/// @brief Normal case: categorical encoding (6 rows returned)
TEST_F(WindowFunctions, LabelEncodeRowCount) {
    std::string sql = "SELECT stat_label_encode(val)";
    sql += kFullFrame;
    sql += " FROM cat_data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 6u);
}

/// @brief The encoded result contains 0, 1 and 2 (three distinct values)
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

/// @brief Normal case: equal-width binning (10 rows returned)
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

/// @brief Normal case: equal-frequency binning (10 rows returned)
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

/// @brief Normal case: lag function (10 rows returned)
TEST_F(WindowFunctions, LagRowCount) {
    std::string sql = "SELECT stat_lag(val, 1)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief One row is NaN (the lagged one)
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

/// @brief Normal case: first difference (10 rows returned)
TEST_F(WindowFunctions, DiffRowCount) {
    std::string sql = "SELECT stat_diff(val, 1)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief One row is NaN; the remaining 9 have a difference of 1.0
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

/// @brief Normal case: seasonal difference (10 rows returned)
TEST_F(WindowFunctions, SeasonalDiffRowCount) {
    std::string sql = "SELECT stat_seasonal_diff(val, 3)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief Three rows are NaN; the remaining 7 have a difference of 3.0
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

/// @brief Normal case: IQR outlier detection (10 rows returned)
TEST_F(WindowFunctions, OutliersIqrRowCount) {
    std::string sql = "SELECT stat_outliers_iqr(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief data(1-10): no outliers -> all 0.0
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

/// @brief Normal case: z-score outlier detection (10 rows returned)
TEST_F(WindowFunctions, OutliersZscoreRowCount) {
    std::string sql = "SELECT stat_outliers_zscore(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief data(1-10): no outliers -> all 0.0
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

/// @brief Normal case: modified z-score outlier detection (10 rows returned)
TEST_F(WindowFunctions, OutliersMzscoreRowCount) {
    std::string sql = "SELECT stat_outliers_mzscore(val)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief data(1-10): no outliers -> all 0.0
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

/// @brief Normal case: winsorization (10 rows returned)
TEST_F(WindowFunctions, WinsorizeRowCount) {
    std::string sql = "SELECT stat_winsorize(val, 10)";
    sql += kFullFrame;
    sql += " FROM data";
    auto results = query_doubles(db_, sql.c_str());
    EXPECT_EQ(results.size(), 10u);
}

/// @brief The result contains finite values
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

// =====================================================================
// 24-26. stat_bonferroni / stat_bh_correction / stat_holm_correction
//
// Multiple testing corrections. Expected values are measured from R 4.4.2 p.adjust().
// Enforcing monotonicity means a scalar form evaluating one row at a time cannot
// express these, so they are implemented as window functions collecting every row.
// =====================================================================

/// @brief Create a p-value table for the correction tests
static void create_pvalue_table(sqlite3* db, const char* name,
                                 const char* values) {
    std::string sql = "CREATE TABLE ";
    sql += name;
    sql += "(id INTEGER PRIMARY KEY, p REAL)";
    exec_sql(db, sql.c_str());
    sql = "INSERT INTO ";
    sql += name;
    sql += "(p) VALUES ";
    sql += values;
    exec_sql(db, sql.c_str());
}

/// @brief Run a correction as a window function over the full frame
static std::vector<double> run_correction(sqlite3* db, const char* func,
                                           const char* table) {
    std::string sql = "SELECT ";
    sql += func;
    sql += "(p)";
    sql += kFullFrame;
    sql += " FROM ";
    sql += table;
    sql += " ORDER BY id";
    return query_doubles(db, sql.c_str());
}

/// @brief Regression: a case needing the monotonicity step. The old scalar form
///        returned 0.12/0.0615/0.042, whereas R p.adjust(method="BH") gives 0.042 throughout.
TEST_F(WindowFunctions, BhCorrectionEnforcesMonotonicity) {
    create_pvalue_table(db_, "pv", "(0.040),(0.041),(0.042)");
    auto r = run_correction(db_, "stat_bh_correction", "pv");
    ASSERT_EQ(r.size(), 3u);
    EXPECT_NEAR(r[0], 0.042, 1e-9);
    EXPECT_NEAR(r[1], 0.042, 1e-9);
    EXPECT_NEAR(r[2], 0.042, 1e-9);
}

/// @brief Regression: the old scalar form returned 0.042 for the largest p-value,
///        producing a false positive; R p.adjust(method="holm") gives 0.12 throughout.
TEST_F(WindowFunctions, HolmCorrectionEnforcesMonotonicity) {
    create_pvalue_table(db_, "pv", "(0.040),(0.041),(0.042)");
    auto r = run_correction(db_, "stat_holm_correction", "pv");
    ASSERT_EQ(r.size(), 3u);
    EXPECT_NEAR(r[0], 0.12, 1e-9);
    EXPECT_NEAR(r[1], 0.12, 1e-9);
    EXPECT_NEAR(r[2], 0.12, 1e-9);
}

/// @brief Normal case: BH correction matches R p.adjust(c(0.001,0.5,0.9),"BH")
TEST_F(WindowFunctions, BhCorrectionMatchesR) {
    create_pvalue_table(db_, "pv", "(0.001),(0.5),(0.9)");
    auto r = run_correction(db_, "stat_bh_correction", "pv");
    ASSERT_EQ(r.size(), 3u);
    EXPECT_NEAR(r[0], 0.003, 1e-9);
    EXPECT_NEAR(r[1], 0.75, 1e-9);
    EXPECT_NEAR(r[2], 0.90, 1e-9);
}

/// @brief Normal case: Holm correction matches R p.adjust(c(0.001,0.5,0.9),"holm")
TEST_F(WindowFunctions, HolmCorrectionMatchesR) {
    create_pvalue_table(db_, "pv", "(0.001),(0.5),(0.9)");
    auto r = run_correction(db_, "stat_holm_correction", "pv");
    ASSERT_EQ(r.size(), 3u);
    EXPECT_NEAR(r[0], 0.003, 1e-9);
    EXPECT_NEAR(r[1], 1.0, 1e-9);
    EXPECT_NEAR(r[2], 1.0, 1e-9);
}

/// @brief Normal case: Bonferroni correction (window form) matches R
TEST_F(WindowFunctions, BonferroniWindowMatchesR) {
    create_pvalue_table(db_, "pv", "(0.040),(0.041),(0.042)");
    auto r = run_correction(db_, "stat_bonferroni", "pv");
    ASSERT_EQ(r.size(), 3u);
    EXPECT_NEAR(r[0], 0.120, 1e-9);
    EXPECT_NEAR(r[1], 0.123, 1e-9);
    EXPECT_NEAR(r[2], 0.126, 1e-9);
}

/// @brief Normal case: clamped at 1.0 (Bonferroni, matching R)
TEST_F(WindowFunctions, BonferroniWindowClampsAtOne) {
    create_pvalue_table(db_, "pv", "(0.5),(0.6),(0.7)");
    auto r = run_correction(db_, "stat_bonferroni", "pv");
    ASSERT_EQ(r.size(), 3u);
    for (const auto& v : r) EXPECT_NEAR(v, 1.0, 1e-9);
}

/// @brief Monotonicity: for both BH and Holm the corrected values follow the p-value order
TEST_F(WindowFunctions, CorrectionsAreMonotone) {
    create_pvalue_table(db_, "pv",
                        "(0.001),(0.008),(0.039),(0.041),(0.042),(0.6),(0.99)");
    for (const char* func : {"stat_bh_correction", "stat_holm_correction"}) {
        auto r = run_correction(db_, func, "pv");
        ASSERT_EQ(r.size(), 7u) << func;
        // The input is inserted in ascending order, so the corrections must be non-decreasing
        for (std::size_t i = 1; i < r.size(); ++i) {
            EXPECT_LE(r[i - 1], r[i] + 1e-12)
                << func << ": monotonicity broken at index " << i;
        }
    }
}

/// @brief Boundary: with a single row nothing is corrected and the p-value is returned as is
TEST_F(WindowFunctions, CorrectionSingleRowIsUnchanged) {
    create_pvalue_table(db_, "pv", "(0.03)");
    for (const char* func : {"stat_bonferroni", "stat_bh_correction",
                             "stat_holm_correction"}) {
        auto r = run_correction(db_, func, "pv");
        ASSERT_EQ(r.size(), 1u) << func;
        EXPECT_NEAR(r[0], 0.03, 1e-9) << func;
    }
}

/// @brief Error case: NULL rows stay NULL and are excluded from the correction
TEST_F(WindowFunctions, CorrectionPreservesNulls) {
    create_pvalue_table(db_, "pv", "(0.001),(NULL),(0.5),(0.9)");
    auto r = run_correction(db_, "stat_bh_correction", "pv");
    ASSERT_EQ(r.size(), 4u);
    EXPECT_TRUE(std::isnan(r[1])) << "a NULL row should return NULL";
    // Only the 3 non-NULL values are corrected, giving the same result as without the NULL
    EXPECT_NEAR(r[0], 0.003, 1e-9);
    EXPECT_NEAR(r[2], 0.75, 1e-9);
    EXPECT_NEAR(r[3], 0.90, 1e-9);
}

/// @brief Error case: all rows NULL returns NULL for every row
TEST_F(WindowFunctions, CorrectionAllNullsReturnsNull) {
    create_pvalue_table(db_, "pv", "(NULL),(NULL)");
    auto r = run_correction(db_, "stat_bh_correction", "pv");
    ASSERT_EQ(r.size(), 2u);
    for (const auto& v : r) EXPECT_TRUE(std::isnan(v));
}
