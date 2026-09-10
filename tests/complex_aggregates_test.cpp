/**
 * @file complex_aggregates_test.cpp
 * @brief Tests for the complex aggregate functions (41 functions)
 *
 * Verifies normal values and empty tables for basic statistics with multiple
 * results, frequency distributions, two-sample tests, analysis of variance,
 * contingency tables, effect sizes, CI differences, survival analysis, resampling,
 */

#include "test_helpers.hpp"

/// @brief Fixture for the complex aggregate function tests
class ComplexAggregates : public StatFuncTest {};

// =====================================================================
// 1. stat_modes (returns JSON)
// =====================================================================

/// @brief Normal case: the mode of data2 -> [3.0]
TEST_F(ComplexAggregates, ModesNormal) {
    std::string result = query_text(
        db_, "SELECT stat_modes(val) FROM data2");
    EXPECT_FALSE(result.empty());
    // Integer values come back as "[3]"
    EXPECT_EQ(result, "[3]");
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, ModesEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_modes(val) FROM empty_data"));
}

// =====================================================================
// 2. stat_five_number_summary (returns JSON)
// =====================================================================

/// @brief Normal case: five-number summary of data(1-10)
TEST_F(ComplexAggregates, FiveNumberSummaryNormal) {
    std::string result = query_text(
        db_, "SELECT stat_five_number_summary(val) FROM data");
    EXPECT_FALSE(result.empty());

    double min_val = json_double(db_, result, "$.min");
    double q1 = json_double(db_, result, "$.q1");
    double q2 = json_double(db_, result, "$.median");
    double q3 = json_double(db_, result, "$.q3");
    double max_val = json_double(db_, result, "$.max");

    EXPECT_NEAR(min_val, 1.0, 1e-4);
    EXPECT_NEAR(q1, 3.25, 1e-4);
    EXPECT_NEAR(q2, 5.5, 1e-4);
    EXPECT_NEAR(q3, 7.75, 1e-4);
    EXPECT_NEAR(max_val, 10.0, 1e-4);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, FiveNumberSummaryEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_five_number_summary(val) FROM empty_data"));
}

// =====================================================================
// 3. stat_frequency_table (returns JSON)
// =====================================================================

/// @brief Normal case: the frequency table is a non-empty JSON string
TEST_F(ComplexAggregates, FrequencyTableNormal) {
    std::string result = query_text(
        db_, "SELECT stat_frequency_table(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, FrequencyTableEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_frequency_table(val) FROM empty_data"));
}

// =====================================================================
// 4. stat_frequency_count (returns JSON)
// =====================================================================

/// @brief Normal case: the frequency count is a non-empty JSON string
TEST_F(ComplexAggregates, FrequencyCountNormal) {
    std::string result = query_text(
        db_, "SELECT stat_frequency_count(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, FrequencyCountEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_frequency_count(val) FROM empty_data"));
}

// =====================================================================
// 5. stat_relative_frequency (returns JSON)
// =====================================================================

/// @brief Normal case: the relative frequency is a non-empty JSON string
TEST_F(ComplexAggregates, RelativeFrequencyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_relative_frequency(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, RelativeFrequencyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_relative_frequency(val) FROM empty_data"));
}

// =====================================================================
// 6. stat_cumulative_frequency (returns JSON)
// =====================================================================

/// @brief Normal case: the cumulative frequency is a non-empty JSON string
TEST_F(ComplexAggregates, CumulativeFrequencyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_cumulative_frequency(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, CumulativeFrequencyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cumulative_frequency(val) FROM empty_data"));
}

// =====================================================================
// 7. stat_cumulative_relative_frequency (returns JSON)
// =====================================================================

/// @brief Normal case: the cumulative relative frequency is a non-empty JSON string
TEST_F(ComplexAggregates, CumulativeRelativeFrequencyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_cumulative_relative_frequency(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, CumulativeRelativeFrequencyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cumulative_relative_frequency(val) FROM empty_data"));
}

// =====================================================================
// 8. stat_t_test2 (returns JSON)
// =====================================================================

/// @brief Normal case: the two groups differ clearly, so p_value < 0.05
TEST_F(ComplexAggregates, TTest2Normal) {
    std::string result = query_text(
        db_, "SELECT stat_t_test2(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_LT(p_value, 0.05);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, TTest2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_t_test2(val, val) FROM empty_data"));
}

// =====================================================================
// 9. stat_t_test_welch (returns JSON)
// =====================================================================

/// @brief Normal case: p_value < 0.05 for the Welch t-test as well
TEST_F(ComplexAggregates, TTestWelchNormal) {
    std::string result = query_text(
        db_, "SELECT stat_t_test_welch(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_LT(p_value, 0.05);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, TTestWelchEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_t_test_welch(val, val) FROM empty_data"));
}

// =====================================================================
// 10. stat_f_test (returns JSON)
// =====================================================================

/// @brief Normal case: the F-test result is a non-empty JSON string
TEST_F(ComplexAggregates, FTestNormal) {
    std::string result = query_text(
        db_, "SELECT stat_f_test(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(p_value));
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, FTestEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_f_test(val, val) FROM empty_data"));
}

// =====================================================================
// 11. stat_mann_whitney (returns JSON)
// =====================================================================

/// @brief Normal case: the two groups differ clearly, so p_value < 0.05
TEST_F(ComplexAggregates, MannWhitneyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_mann_whitney(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_LT(p_value, 0.05);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, MannWhitneyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mann_whitney(val, val) FROM empty_data"));
}

// =====================================================================
// 12. stat_chisq_independence (returns JSON)
// =====================================================================

/// @brief Normal case: the chi-square test of independence is a non-empty JSON string
TEST_F(ComplexAggregates, ChisqIndependenceNormal) {
    std::string result = query_text(
        db_, "SELECT stat_chisq_independence(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, ChisqIndependenceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_chisq_independence(val, val) FROM empty_data"));
}

// =====================================================================
// 13. stat_anova1 (returns JSON)
// =====================================================================

/// @brief Normal case: one-way ANOVA; the two groups differ clearly, so p_value < 0.05
TEST_F(ComplexAggregates, Anova1Normal) {
    std::string result = query_text(
        db_, "SELECT stat_anova1(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double f_stat = json_double(db_, result, "$.f_statistic");
    double p_value = json_double(db_, result, "$.p_value");
    double df_between = json_double(db_, result, "$.df_between");
    double df_within = json_double(db_, result, "$.df_within");

    EXPECT_TRUE(std::isfinite(f_stat));
    EXPECT_LT(p_value, 0.05);
    EXPECT_TRUE(std::isfinite(df_between));
    EXPECT_TRUE(std::isfinite(df_within));
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, Anova1Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_anova1(val, val) FROM empty_data"));
}

// =====================================================================
// 14. stat_contingency_table (returns JSON)
// =====================================================================

/// @brief Normal case: the contingency table is a non-empty JSON string
TEST_F(ComplexAggregates, ContingencyTableNormal) {
    std::string result = query_text(
        db_, "SELECT stat_contingency_table(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, ContingencyTableEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_contingency_table(val, val) FROM empty_data"));
}

// =====================================================================
// 15. stat_cohens_d2 (returns REAL)
// =====================================================================

/// @brief Normal case: Cohen's d for two groups (measured value ~ 4.304)
TEST_F(ComplexAggregates, CohensD2Normal) {
    double result = query_double(
        db_, "SELECT stat_cohens_d2(val, grp) FROM grp_data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result, 4.304, 0.1);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, CohensD2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cohens_d2(val, val) FROM empty_data"));
}

// =====================================================================
// 16. stat_hedges_g2 (returns REAL)
// =====================================================================

/// @brief Normal case: close to Cohen's d
TEST_F(ComplexAggregates, HedgesG2Normal) {
    double result = query_double(
        db_, "SELECT stat_hedges_g2(val, grp) FROM grp_data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);

    // Compared with Cohen's d, smaller by the correction factor
    double cohens_d = query_double(
        db_, "SELECT stat_cohens_d2(val, grp) FROM grp_data");
    EXPECT_NEAR(result, cohens_d, 0.5);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, HedgesG2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_hedges_g2(val, val) FROM empty_data"));
}

// =====================================================================
// 17. stat_glass_delta (returns REAL)
// =====================================================================

/// @brief Normal case: Glass's delta (sign depends on the reference group, measured ~ -3.055)
TEST_F(ComplexAggregates, GlassDeltaNormal) {
    double result = query_double(
        db_, "SELECT stat_glass_delta(val, grp) FROM grp_data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result, -3.055, 0.1);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, GlassDeltaEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_glass_delta(val, val) FROM empty_data"));
}

// =====================================================================
// 18. stat_ci_mean_diff (returns JSON)
// =====================================================================

/// @brief Normal case: the groups clearly differ, so lower > 0 and upper > 0
TEST_F(ComplexAggregates, CiMeanDiffNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ci_mean_diff(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double lower = json_double(db_, result, "$.lower");
    double upper = json_double(db_, result, "$.upper");
    EXPECT_GT(lower, 0.0);
    EXPECT_GT(upper, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, CiMeanDiffEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ci_mean_diff(val, val) FROM empty_data"));
}

// =====================================================================
// 19. stat_ci_mean_diff_welch (returns JSON)
// =====================================================================

/// @brief Normal case: lower > 0 and upper > 0 for the Welch form too
TEST_F(ComplexAggregates, CiMeanDiffWelchNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ci_mean_diff_welch(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double lower = json_double(db_, result, "$.lower");
    double upper = json_double(db_, result, "$.upper");
    EXPECT_GT(lower, 0.0);
    EXPECT_GT(upper, 0.0);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, CiMeanDiffWelchEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ci_mean_diff_welch(val, val) FROM empty_data"));
}

// =====================================================================
// 20. stat_kaplan_meier (returns JSON)
// =====================================================================

/// @brief Normal case: the Kaplan-Meier estimate is a non-empty JSON string
TEST_F(ComplexAggregates, KaplanMeierNormal) {
    std::string result = query_text(
        db_, "SELECT stat_kaplan_meier(time, event) FROM surv_data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, KaplanMeierEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_kaplan_meier(val, val) FROM empty_data"));
}

// =====================================================================
// 21. stat_nelson_aalen (returns JSON)
// =====================================================================

/// @brief Normal case: the Nelson-Aalen estimate is a non-empty JSON string
TEST_F(ComplexAggregates, NelsonAalenNormal) {
    std::string result = query_text(
        db_, "SELECT stat_nelson_aalen(time, event) FROM surv_data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, NelsonAalenEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_nelson_aalen(val, val) FROM empty_data"));
}

// =====================================================================
// 22. stat_logrank (returns JSON)
// =====================================================================

/// @brief Normal case: the log-rank test result is a non-empty JSON string
TEST_F(ComplexAggregates, LograrkNormal) {
    std::string result = query_text(
        db_, "SELECT stat_logrank(time, event, grp) FROM surv2");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_TRUE(std::isfinite(p_value));
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, LograrkEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_logrank(val, val, val) FROM empty_data"));
}

// =====================================================================
// 23. stat_bootstrap (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: bootstrap estimate ~ 5.5 (+/-2.0)
TEST_F(ComplexAggregates, BootstrapNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap(val, 500) FROM data");
    EXPECT_FALSE(result.empty());

    double estimate = json_double(db_, result, "$.estimate");
    double se = json_double(db_, result, "$.standard_error");
    EXPECT_TRUE(std::isfinite(estimate));
    EXPECT_TRUE(std::isfinite(se));
    EXPECT_NEAR(estimate, 5.5, 2.0);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, BootstrapEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap(val, 500) FROM empty_data"));
}

// =====================================================================
// 24. stat_bootstrap_bca (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: the BCa bootstrap estimate is a non-empty JSON string
TEST_F(ComplexAggregates, BootstrapBcaNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap_bca(val, 500) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, BootstrapBcaEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap_bca(val, 500) FROM empty_data"));
}

// =====================================================================
// 25. stat_bootstrap_sample (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: the bootstrap sample is a non-empty JSON array
TEST_F(ComplexAggregates, BootstrapSampleNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap_sample(val) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, BootstrapSampleEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap_sample(val) FROM empty_data"));
}

// =====================================================================
// 26. stat_permutation_test2 (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: permutation test; the groups differ clearly, so p_value < 0.1
TEST_F(ComplexAggregates, PermutationTest2Normal) {
    std::string result = query_text(
        db_, "SELECT stat_permutation_test2(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.observed_statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_TRUE(std::isfinite(p_value));
    EXPECT_LT(p_value, 0.1);
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, PermutationTest2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_permutation_test2(val, val) FROM empty_data"));
}

// =====================================================================
// 27. stat_permutation_paired (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: the paired permutation test is a non-empty JSON string
TEST_F(ComplexAggregates, PermutationPairedNormal) {
    std::string result = query_text(
        db_, "SELECT stat_permutation_paired(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, PermutationPairedEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_permutation_paired(val, val) FROM empty_data"));
}

// =====================================================================
// 28. stat_permutation_corr (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: the correlation permutation test is a non-empty JSON string
TEST_F(ComplexAggregates, PermutationCorrNormal) {
    std::string result = query_text(
        db_, "SELECT stat_permutation_corr(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, PermutationCorrEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_permutation_corr(val, val) FROM empty_data"));
}

// =====================================================================
// 29. stat_acf (returns JSON)
// =====================================================================

/// @brief Normal case: the autocorrelation function is a non-empty JSON array
TEST_F(ComplexAggregates, AcfNormal) {
    std::string result = query_text(
        db_, "SELECT stat_acf(val, 5) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, AcfEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_acf(val, 5) FROM empty_data"));
}

// =====================================================================
// 30. stat_pacf (returns JSON)
// =====================================================================

/// @brief Normal case: the partial autocorrelation function is a non-empty JSON array
TEST_F(ComplexAggregates, PacfNormal) {
    std::string result = query_text(
        db_, "SELECT stat_pacf(val, 5) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, PacfEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_pacf(val, 5) FROM empty_data"));
}

// =====================================================================
// 31. stat_sample_replace (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: sampling with replacement gives a non-empty JSON array
TEST_F(ComplexAggregates, SampleReplaceNormal) {
    std::string result = query_text(
        db_, "SELECT stat_sample_replace(val, 3) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, SampleReplaceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample_replace(val, 3) FROM empty_data"));
}

// =====================================================================
// 32. stat_sample (returns JSON, non-deterministic)
// =====================================================================

/// @brief Normal case: sampling without replacement gives a non-empty JSON array
TEST_F(ComplexAggregates, SampleNormal) {
    std::string result = query_text(
        db_, "SELECT stat_sample(val, 3) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief Empty table -> NULL
TEST_F(ComplexAggregates, SampleEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample(val, 3) FROM empty_data"));
}

// =====================================================================
// 33-41. Group-column form (value, group)
//
// Tests and post-hoc tests taking the same "values + groups" form as stat_anova1.
// Expected values are measured from R 4.4.2.
//   v <- c(10,12,14,11,13, 20,30,15,25,40, 15,17,16,18,14)
//   g <- factor(rep(1:3, each = 5))
// =====================================================================

/// @brief Create the group test data (three groups, unequal variances)
static void create_group_table(sqlite3* db) {
    exec_sql(db, "CREATE TABLE g3(val REAL, grp INT)");
    exec_sql(db,
        "INSERT INTO g3 VALUES "
        "(10,1),(12,1),(14,1),(11,1),(13,1),"
        "(20,2),(30,2),(15,2),(25,2),(40,2),"
        "(15,3),(17,3),(16,3),(18,3),(14,3)");
}

/// @brief Create the group test data (three groups, equal variances, for post-hoc tests)
static void create_balanced_group_table(sqlite3* db) {
    exec_sql(db, "CREATE TABLE gb(val REAL, grp INT)");
    exec_sql(db,
        "INSERT INTO gb VALUES "
        "(10,1),(12,1),(14,1),(11,1),(13,1),"
        "(20,2),(22,2),(19,2),(21,2),(23,2),"
        "(15,3),(17,3),(16,3),(18,3),(14,3)");
}

/// @brief Normal case: Kruskal-Wallis matches R's kruskal.test()
TEST_F(ComplexAggregates, KruskalWallisMatchesR) {
    create_group_table(db_);
    std::string r = query_text(db_, "SELECT stat_kruskal_wallis(val, grp) FROM g3");
    ASSERT_FALSE(r.empty());
    EXPECT_NEAR(json_double(db_, r, "$.statistic"), 10.75341, 1e-4);
    EXPECT_NEAR(json_double(db_, r, "$.p_value"), 0.004623041, 1e-8);
    EXPECT_NEAR(json_double(db_, r, "$.df"), 2.0, 1e-9);
}

/// @brief Normal case: Levene matches R's car::leveneTest()
///        (statcpp uses the median-based Brown-Forsythe form, as car does by default)
TEST_F(ComplexAggregates, LeveneMatchesR) {
    create_group_table(db_);
    std::string r = query_text(db_, "SELECT stat_levene(val, grp) FROM g3");
    ASSERT_FALSE(r.empty());
    EXPECT_NEAR(json_double(db_, r, "$.statistic"), 4.961652, 1e-5);
    EXPECT_NEAR(json_double(db_, r, "$.p_value"), 0.02689376, 1e-7);
}

/// @brief Normal case: Bartlett matches R's bartlett.test()
TEST_F(ComplexAggregates, BartlettMatchesR) {
    create_group_table(db_);
    std::string r = query_text(db_, "SELECT stat_bartlett(val, grp) FROM g3");
    ASSERT_FALSE(r.empty());
    EXPECT_NEAR(json_double(db_, r, "$.statistic"), 14.70215, 1e-4);
    EXPECT_NEAR(json_double(db_, r, "$.p_value"), 0.0006419024, 1e-9);
}

/// @brief Boundary: with equal variances across groups, Levene and Bartlett give F=0, p=1
TEST_F(ComplexAggregates, EqualVarianceGivesZeroStatistic) {
    create_balanced_group_table(db_);
    for (const char* fn : {"stat_levene", "stat_bartlett"}) {
        std::string sql = "SELECT ";
        sql += fn;
        sql += "(val, grp) FROM gb";
        std::string r = query_text(db_, sql.c_str());
        ASSERT_FALSE(r.empty()) << fn;
        EXPECT_NEAR(json_double(db_, r, "$.statistic"), 0.0, 1e-9) << fn;
        EXPECT_NEAR(json_double(db_, r, "$.p_value"), 1.0, 1e-9) << fn;
    }
}

/// @brief Normal case: Cohen's f matches R's sqrt(eta2/(1-eta2))
TEST_F(ComplexAggregates, CohensFMatchesR) {
    create_group_table(db_);
    double f = query_double(db_, "SELECT stat_cohens_f(val, grp) FROM g3");
    EXPECT_NEAR(f, 1.154701, 1e-6);
}

/// @brief Normal case: Tukey HSD matches R's TukeyHSD()
///        statcpp reports group1 - group2 with group1 the lower index, whereas R
///        reports the opposite direction, so the mean difference and interval are negated
TEST_F(ComplexAggregates, TukeyHsdMatchesR) {
    create_balanced_group_table(db_);
    std::string r = query_text(db_, "SELECT stat_tukey_hsd(val, grp) FROM gb");
    ASSERT_FALSE(r.empty());
    EXPECT_NEAR(json_double(db_, r, "$.alpha"), 0.05, 1e-9);
    EXPECT_NEAR(json_double(db_, r, "$.mse"), 2.5, 1e-9);
    EXPECT_NEAR(json_double(db_, r, "$.df_error"), 12.0, 1e-9);
    // R: 2-1 diff=9, lwr=6.332136, upr=11.667864, p adj=0.0000031
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[0].mean_diff"), -9.0, 1e-9);
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[0].lower"), -11.667864, 1e-5);
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[0].upper"), -6.332136, 1e-5);
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[0].p_value"), 3.07581e-06, 1e-10);
    // R: 3-2 diff=-5, p adj=0.0008342 -> by index this is group1=1, group2=2 with +5
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[2].mean_diff"), 5.0, 1e-9);
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[2].p_value"), 0.00083421, 1e-8);
}

/// @brief Normal case: three groups means all pairs, so comparisons has 3 entries
TEST_F(ComplexAggregates, PosthocComparisonCount) {
    create_balanced_group_table(db_);
    for (const char* fn : {"stat_tukey_hsd", "stat_bonferroni_posthoc",
                           "stat_scheffe_posthoc"}) {
        std::string sql = "SELECT json_array_length(";
        sql += fn;
        sql += "(val, grp), '$.comparisons') FROM gb";
        EXPECT_NEAR(query_double(db_, sql.c_str()), 3.0, 1e-9) << fn;
    }
    // Dunnett compares against the control only, so k-1 = 2 entries
    EXPECT_NEAR(query_double(db_,
        "SELECT json_array_length(stat_dunnett_posthoc(val, grp), "
        "'$.comparisons') FROM gb"), 2.0, 1e-9);
}

/// @brief Normal case: alpha defaults to 0.05, or takes the value given
TEST_F(ComplexAggregates, PosthocAlphaIsConfigurable) {
    create_balanced_group_table(db_);
    EXPECT_NEAR(query_double(db_,
        "SELECT json_extract(stat_tukey_hsd(val, grp), '$.alpha') FROM gb"),
        0.05, 1e-9);
    EXPECT_NEAR(query_double(db_,
        "SELECT json_extract(stat_tukey_hsd(val, grp, 0.01), '$.alpha') FROM gb"),
        0.01, 1e-9);
}

/// @brief Normal case: Scheffe is more conservative than Tukey, so its p-value is larger
TEST_F(ComplexAggregates, ScheffeIsMoreConservativeThanTukey) {
    create_balanced_group_table(db_);
    double tukey = query_double(db_,
        "SELECT json_extract(stat_tukey_hsd(val, grp), "
        "'$.comparisons[1].p_value') FROM gb");
    double scheffe = query_double(db_,
        "SELECT json_extract(stat_scheffe_posthoc(val, grp), "
        "'$.comparisons[1].p_value') FROM gb");
    EXPECT_GT(scheffe, tukey);
}

/// @brief Error case: returns NULL when there is only one group
TEST_F(ComplexAggregates, GroupFunctionsRequireTwoGroups) {
    exec_sql(db_, "CREATE TABLE g1(val REAL, grp INT)");
    exec_sql(db_, "INSERT INTO g1 VALUES (1,1),(2,1),(3,1)");
    for (const char* fn : {"stat_kruskal_wallis", "stat_levene", "stat_bartlett",
                           "stat_cohens_f", "stat_tukey_hsd",
                           "stat_bonferroni_posthoc", "stat_scheffe_posthoc",
                           "stat_dunnett_posthoc"}) {
        std::string sql = "SELECT ";
        sql += fn;
        sql += "(val, grp) FROM g1";
        EXPECT_TRUE(query_is_null(db_, sql.c_str())) << fn;
    }
}

/// @brief Error case: empty table -> NULL
TEST_F(ComplexAggregates, GroupFunctionsEmpty) {
    for (const char* fn : {"stat_kruskal_wallis", "stat_levene", "stat_bartlett",
                           "stat_cohens_f", "stat_tukey_hsd",
                           "stat_stratified_sample"}) {
        std::string sql = "SELECT ";
        sql += fn;
        sql += "(val, val) FROM empty_data";
        EXPECT_TRUE(query_is_null(db_, sql.c_str())) << fn;
    }
}

/// @brief Error case: returns NULL when the control index exceeds the group count
TEST_F(ComplexAggregates, DunnettRejectsOutOfRangeControl) {
    create_balanced_group_table(db_);
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_dunnett_posthoc(val, grp, 9, 0.05) FROM gb"));
}

/// @brief Error case: NULL rows are excluded when splitting into groups
TEST_F(ComplexAggregates, GroupFunctionsSkipNulls) {
    exec_sql(db_, "CREATE TABLE gn(val REAL, grp INT)");
    exec_sql(db_, "INSERT INTO gn VALUES "
                  "(10,1),(NULL,1),(12,1),(14,1),(20,2),(22,2),(NULL,2),(19,2)");
    std::string r = query_text(db_, "SELECT stat_bartlett(val, grp) FROM gn");
    ASSERT_FALSE(r.empty());
    // Only the 3 non-NULL values per group are tested, so df = k - 1 = 1
    EXPECT_NEAR(json_double(db_, r, "$.df"), 1.0, 1e-9);
}

/// @brief Normal case: stratified sampling draws from each stratum in proportion to the ratio
TEST_F(ComplexAggregates, StratifiedSampleRespectsRatio) {
    create_balanced_group_table(db_);
    // 5 rows per group across 3 groups; a ratio of 0.4 gives 2 per group = 6 in total
    double n = query_double(db_,
        "SELECT json_array_length(stat_stratified_sample(val, grp, 0.4)) FROM gb");
    EXPECT_NEAR(n, 6.0, 1e-9);
}

/// @brief Boundary: a ratio of 1.0 returns every row
TEST_F(ComplexAggregates, StratifiedSampleFullRatio) {
    create_balanced_group_table(db_);
    double n = query_double(db_,
        "SELECT json_array_length(stat_stratified_sample(val, grp, 1.0)) FROM gb");
    EXPECT_NEAR(n, 15.0, 1e-9);
}

/// @brief Normal case: Dunnett accepts the 2, 3 and 4 argument forms
///        (trailing parameters can be dropped from the right)
TEST_F(ComplexAggregates, DunnettAcceptsAllArities) {
    create_balanced_group_table(db_);
    const char* sqls[] = {
        "SELECT json_extract(stat_dunnett_posthoc(val, grp), '$.alpha') FROM gb",
        "SELECT json_extract(stat_dunnett_posthoc(val, grp, 1), '$.alpha') FROM gb",
        "SELECT json_extract(stat_dunnett_posthoc(val, grp, 1, 0.01), '$.alpha') "
        "FROM gb",
    };
    EXPECT_NEAR(query_double(db_, sqls[0]), 0.05, 1e-9);
    EXPECT_NEAR(query_double(db_, sqls[1]), 0.05, 1e-9);
    EXPECT_NEAR(query_double(db_, sqls[2]), 0.01, 1e-9);
}
