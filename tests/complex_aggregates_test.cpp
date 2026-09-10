/**
 * @file complex_aggregates_test.cpp
 * @brief 複合集約統計関数(32関数)のテスト
 *
 * 基本統計量(複数結果), 度数分布, 2標本検定, 分散分析, 分割表,
 * 効果量, CI差, 生存分析, リサンプリング, 時系列, サンプリング
 * の各関数について,正常値・空テーブルを検証する.
 */

#include "test_helpers.hpp"

/// @brief 複合集約統計関数テスト用フィクスチャ
class ComplexAggregates : public StatFuncTest {};

// =====================================================================
// 1. stat_modes (JSON返却)
// =====================================================================

/// @brief 正常系: data2の最頻値 → [3.0]
TEST_F(ComplexAggregates, ModesNormal) {
    std::string result = query_text(
        db_, "SELECT stat_modes(val) FROM data2");
    EXPECT_FALSE(result.empty());
    // 整数値の場合 "[3]" が返る
    EXPECT_EQ(result, "[3]");
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, ModesEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_modes(val) FROM empty_data"));
}

// =====================================================================
// 2. stat_five_number_summary (JSON返却)
// =====================================================================

/// @brief 正常系: data(1-10)の五数要約
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

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, FiveNumberSummaryEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_five_number_summary(val) FROM empty_data"));
}

// =====================================================================
// 3. stat_frequency_table (JSON返却)
// =====================================================================

/// @brief 正常系: 度数分布表が空でないJSON文字列
TEST_F(ComplexAggregates, FrequencyTableNormal) {
    std::string result = query_text(
        db_, "SELECT stat_frequency_table(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, FrequencyTableEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_frequency_table(val) FROM empty_data"));
}

// =====================================================================
// 4. stat_frequency_count (JSON返却)
// =====================================================================

/// @brief 正常系: 度数カウントが空でないJSON文字列
TEST_F(ComplexAggregates, FrequencyCountNormal) {
    std::string result = query_text(
        db_, "SELECT stat_frequency_count(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, FrequencyCountEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_frequency_count(val) FROM empty_data"));
}

// =====================================================================
// 5. stat_relative_frequency (JSON返却)
// =====================================================================

/// @brief 正常系: 相対度数が空でないJSON文字列
TEST_F(ComplexAggregates, RelativeFrequencyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_relative_frequency(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, RelativeFrequencyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_relative_frequency(val) FROM empty_data"));
}

// =====================================================================
// 6. stat_cumulative_frequency (JSON返却)
// =====================================================================

/// @brief 正常系: 累積度数が空でないJSON文字列
TEST_F(ComplexAggregates, CumulativeFrequencyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_cumulative_frequency(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, CumulativeFrequencyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cumulative_frequency(val) FROM empty_data"));
}

// =====================================================================
// 7. stat_cumulative_relative_frequency (JSON返却)
// =====================================================================

/// @brief 正常系: 累積相対度数が空でないJSON文字列
TEST_F(ComplexAggregates, CumulativeRelativeFrequencyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_cumulative_relative_frequency(val) FROM data2");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, CumulativeRelativeFrequencyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cumulative_relative_frequency(val) FROM empty_data"));
}

// =====================================================================
// 8. stat_t_test2 (JSON返却)
// =====================================================================

/// @brief 正常系: 2群は明確に異なるので p_value < 0.05
TEST_F(ComplexAggregates, TTest2Normal) {
    std::string result = query_text(
        db_, "SELECT stat_t_test2(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_LT(p_value, 0.05);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, TTest2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_t_test2(val, val) FROM empty_data"));
}

// =====================================================================
// 9. stat_t_test_welch (JSON返却)
// =====================================================================

/// @brief 正常系: Welch t検定でも p_value < 0.05
TEST_F(ComplexAggregates, TTestWelchNormal) {
    std::string result = query_text(
        db_, "SELECT stat_t_test_welch(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_LT(p_value, 0.05);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, TTestWelchEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_t_test_welch(val, val) FROM empty_data"));
}

// =====================================================================
// 10. stat_f_test (JSON返却)
// =====================================================================

/// @brief 正常系: F検定の結果がJSON文字列
TEST_F(ComplexAggregates, FTestNormal) {
    std::string result = query_text(
        db_, "SELECT stat_f_test(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(p_value));
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, FTestEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_f_test(val, val) FROM empty_data"));
}

// =====================================================================
// 11. stat_mann_whitney (JSON返却)
// =====================================================================

/// @brief 正常系: 2群は明確に異なるので p_value < 0.05
TEST_F(ComplexAggregates, MannWhitneyNormal) {
    std::string result = query_text(
        db_, "SELECT stat_mann_whitney(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_LT(p_value, 0.05);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, MannWhitneyEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_mann_whitney(val, val) FROM empty_data"));
}

// =====================================================================
// 12. stat_chisq_independence (JSON返却)
// =====================================================================

/// @brief 正常系: カイ二乗独立性検定の結果がJSON文字列
TEST_F(ComplexAggregates, ChisqIndependenceNormal) {
    std::string result = query_text(
        db_, "SELECT stat_chisq_independence(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, ChisqIndependenceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_chisq_independence(val, val) FROM empty_data"));
}

// =====================================================================
// 13. stat_anova1 (JSON返却)
// =====================================================================

/// @brief 正常系: 一元配置分散分析, 2群は明確に異なるので p_value < 0.05
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

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, Anova1Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_anova1(val, val) FROM empty_data"));
}

// =====================================================================
// 14. stat_contingency_table (JSON返却)
// =====================================================================

/// @brief 正常系: 分割表が空でないJSON文字列
TEST_F(ComplexAggregates, ContingencyTableNormal) {
    std::string result = query_text(
        db_, "SELECT stat_contingency_table(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, ContingencyTableEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_contingency_table(val, val) FROM empty_data"));
}

// =====================================================================
// 15. stat_cohens_d2 (REAL返却)
// =====================================================================

/// @brief 正常系: 2群の Cohen's d (実測値 ≈ 4.304)
TEST_F(ComplexAggregates, CohensD2Normal) {
    double result = query_double(
        db_, "SELECT stat_cohens_d2(val, grp) FROM grp_data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result, 4.304, 0.1);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, CohensD2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cohens_d2(val, val) FROM empty_data"));
}

// =====================================================================
// 16. stat_hedges_g2 (REAL返却)
// =====================================================================

/// @brief 正常系: Cohen's d に近い値
TEST_F(ComplexAggregates, HedgesG2Normal) {
    double result = query_double(
        db_, "SELECT stat_hedges_g2(val, grp) FROM grp_data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);

    // Cohen's d との比較(補正係数分だけ小さくなる)
    double cohens_d = query_double(
        db_, "SELECT stat_cohens_d2(val, grp) FROM grp_data");
    EXPECT_NEAR(result, cohens_d, 0.5);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, HedgesG2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_hedges_g2(val, val) FROM empty_data"));
}

// =====================================================================
// 17. stat_glass_delta (REAL返却)
// =====================================================================

/// @brief 正常系: Glass's delta (基準群によって符号が異なる, 実測値 ≈ -3.055)
TEST_F(ComplexAggregates, GlassDeltaNormal) {
    double result = query_double(
        db_, "SELECT stat_glass_delta(val, grp) FROM grp_data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_NEAR(result, -3.055, 0.1);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, GlassDeltaEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_glass_delta(val, val) FROM empty_data"));
}

// =====================================================================
// 18. stat_ci_mean_diff (JSON返却)
// =====================================================================

/// @brief 正常系: 2群に明確な差があるので lower > 0, upper > 0
TEST_F(ComplexAggregates, CiMeanDiffNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ci_mean_diff(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double lower = json_double(db_, result, "$.lower");
    double upper = json_double(db_, result, "$.upper");
    EXPECT_GT(lower, 0.0);
    EXPECT_GT(upper, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, CiMeanDiffEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ci_mean_diff(val, val) FROM empty_data"));
}

// =====================================================================
// 19. stat_ci_mean_diff_welch (JSON返却)
// =====================================================================

/// @brief 正常系: Welch版でも lower > 0, upper > 0
TEST_F(ComplexAggregates, CiMeanDiffWelchNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ci_mean_diff_welch(val, grp) FROM grp_data");
    EXPECT_FALSE(result.empty());

    double lower = json_double(db_, result, "$.lower");
    double upper = json_double(db_, result, "$.upper");
    EXPECT_GT(lower, 0.0);
    EXPECT_GT(upper, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, CiMeanDiffWelchEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ci_mean_diff_welch(val, val) FROM empty_data"));
}

// =====================================================================
// 20. stat_kaplan_meier (JSON返却)
// =====================================================================

/// @brief 正常系: Kaplan-Meier推定が空でないJSON文字列
TEST_F(ComplexAggregates, KaplanMeierNormal) {
    std::string result = query_text(
        db_, "SELECT stat_kaplan_meier(time, event) FROM surv_data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, KaplanMeierEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_kaplan_meier(val, val) FROM empty_data"));
}

// =====================================================================
// 21. stat_nelson_aalen (JSON返却)
// =====================================================================

/// @brief 正常系: Nelson-Aalen推定が空でないJSON文字列
TEST_F(ComplexAggregates, NelsonAalenNormal) {
    std::string result = query_text(
        db_, "SELECT stat_nelson_aalen(time, event) FROM surv_data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, NelsonAalenEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_nelson_aalen(val, val) FROM empty_data"));
}

// =====================================================================
// 22. stat_logrank (JSON返却)
// =====================================================================

/// @brief 正常系: ログランク検定の結果がJSON文字列
TEST_F(ComplexAggregates, LograrkNormal) {
    std::string result = query_text(
        db_, "SELECT stat_logrank(time, event, grp) FROM surv2");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_TRUE(std::isfinite(p_value));
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, LograrkEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_logrank(val, val, val) FROM empty_data"));
}

// =====================================================================
// 23. stat_bootstrap (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: bootstrap推定, estimate ≈ 5.5 (±2.0)
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

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, BootstrapEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap(val, 500) FROM empty_data"));
}

// =====================================================================
// 24. stat_bootstrap_bca (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: BCa bootstrap推定が空でないJSON文字列
TEST_F(ComplexAggregates, BootstrapBcaNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap_bca(val, 500) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, BootstrapBcaEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap_bca(val, 500) FROM empty_data"));
}

// =====================================================================
// 25. stat_bootstrap_sample (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: ブートストラップ標本が空でないJSON配列
TEST_F(ComplexAggregates, BootstrapSampleNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap_sample(val) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, BootstrapSampleEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap_sample(val) FROM empty_data"));
}

// =====================================================================
// 26. stat_permutation_test2 (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: 置換検定, 2群は明確に異なるので p_value < 0.1
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

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, PermutationTest2Empty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_permutation_test2(val, val) FROM empty_data"));
}

// =====================================================================
// 27. stat_permutation_paired (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: 対応のある置換検定が空でないJSON文字列
TEST_F(ComplexAggregates, PermutationPairedNormal) {
    std::string result = query_text(
        db_, "SELECT stat_permutation_paired(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, PermutationPairedEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_permutation_paired(val, val) FROM empty_data"));
}

// =====================================================================
// 28. stat_permutation_corr (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: 相関の置換検定が空でないJSON文字列
TEST_F(ComplexAggregates, PermutationCorrNormal) {
    std::string result = query_text(
        db_, "SELECT stat_permutation_corr(x, y) FROM xy_data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, PermutationCorrEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_permutation_corr(val, val) FROM empty_data"));
}

// =====================================================================
// 29. stat_acf (JSON返却)
// =====================================================================

/// @brief 正常系: 自己相関関数が空でないJSON配列
TEST_F(ComplexAggregates, AcfNormal) {
    std::string result = query_text(
        db_, "SELECT stat_acf(val, 5) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, AcfEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_acf(val, 5) FROM empty_data"));
}

// =====================================================================
// 30. stat_pacf (JSON返却)
// =====================================================================

/// @brief 正常系: 偏自己相関関数が空でないJSON配列
TEST_F(ComplexAggregates, PacfNormal) {
    std::string result = query_text(
        db_, "SELECT stat_pacf(val, 5) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, PacfEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_pacf(val, 5) FROM empty_data"));
}

// =====================================================================
// 31. stat_sample_replace (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: 復元抽出が空でないJSON配列
TEST_F(ComplexAggregates, SampleReplaceNormal) {
    std::string result = query_text(
        db_, "SELECT stat_sample_replace(val, 3) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, SampleReplaceEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample_replace(val, 3) FROM empty_data"));
}

// =====================================================================
// 32. stat_sample (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: 非復元抽出が空でないJSON配列
TEST_F(ComplexAggregates, SampleNormal) {
    std::string result = query_text(
        db_, "SELECT stat_sample(val, 3) FROM data");
    EXPECT_FALSE(result.empty());
}

/// @brief 空テーブル → NULL
TEST_F(ComplexAggregates, SampleEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_sample(val, 3) FROM empty_data"));
}

// =====================================================================
// 33-41. 群列パターン (value, group)
//
// stat_anova1 と同じ「値列 + 群列」形式を取る検定・事後検定.
// 期待値は R 4.4.2 の実測値を使用する.
//   v <- c(10,12,14,11,13, 20,30,15,25,40, 15,17,16,18,14)
//   g <- factor(rep(1:3, each = 5))
// =====================================================================

/// @brief 群列テスト用のデータを作成する(3群・分散が不均一)
static void create_group_table(sqlite3* db) {
    exec_sql(db, "CREATE TABLE g3(val REAL, grp INT)");
    exec_sql(db,
        "INSERT INTO g3 VALUES "
        "(10,1),(12,1),(14,1),(11,1),(13,1),"
        "(20,2),(30,2),(15,2),(25,2),(40,2),"
        "(15,3),(17,3),(16,3),(18,3),(14,3)");
}

/// @brief 群列テスト用のデータを作成する(3群・等分散, 事後検定用)
static void create_balanced_group_table(sqlite3* db) {
    exec_sql(db, "CREATE TABLE gb(val REAL, grp INT)");
    exec_sql(db,
        "INSERT INTO gb VALUES "
        "(10,1),(12,1),(14,1),(11,1),(13,1),"
        "(20,2),(22,2),(19,2),(21,2),(23,2),"
        "(15,3),(17,3),(16,3),(18,3),(14,3)");
}

/// @brief 正常系: Kruskal-Wallis 検定が R の kruskal.test() と一致する
TEST_F(ComplexAggregates, KruskalWallisMatchesR) {
    create_group_table(db_);
    std::string r = query_text(db_, "SELECT stat_kruskal_wallis(val, grp) FROM g3");
    ASSERT_FALSE(r.empty());
    EXPECT_NEAR(json_double(db_, r, "$.statistic"), 10.75341, 1e-4);
    EXPECT_NEAR(json_double(db_, r, "$.p_value"), 0.004623041, 1e-8);
    EXPECT_NEAR(json_double(db_, r, "$.df"), 2.0, 1e-9);
}

/// @brief 正常系: Levene 検定が R の car::leveneTest() と一致する
///        (statcpp は中央値基準の Brown-Forsythe 版. car の既定と同じ)
TEST_F(ComplexAggregates, LeveneMatchesR) {
    create_group_table(db_);
    std::string r = query_text(db_, "SELECT stat_levene(val, grp) FROM g3");
    ASSERT_FALSE(r.empty());
    EXPECT_NEAR(json_double(db_, r, "$.statistic"), 4.961652, 1e-5);
    EXPECT_NEAR(json_double(db_, r, "$.p_value"), 0.02689376, 1e-7);
}

/// @brief 正常系: Bartlett 検定が R の bartlett.test() と一致する
TEST_F(ComplexAggregates, BartlettMatchesR) {
    create_group_table(db_);
    std::string r = query_text(db_, "SELECT stat_bartlett(val, grp) FROM g3");
    ASSERT_FALSE(r.empty());
    EXPECT_NEAR(json_double(db_, r, "$.statistic"), 14.70215, 1e-4);
    EXPECT_NEAR(json_double(db_, r, "$.p_value"), 0.0006419024, 1e-9);
}

/// @brief 境界値: 全群の分散が等しい場合, Levene/Bartlett は F=0, p=1 を返す
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

/// @brief 正常系: Cohen's f が R の sqrt(eta2/(1-eta2)) と一致する
TEST_F(ComplexAggregates, CohensFMatchesR) {
    create_group_table(db_);
    double f = query_double(db_, "SELECT stat_cohens_f(val, grp) FROM g3");
    EXPECT_NEAR(f, 1.154701, 1e-6);
}

/// @brief 正常系: Tukey HSD が R の TukeyHSD() と一致する
///        statcpp は group1 - group2 (添字の小さい方が基準), R は逆向きに
///        報告するため, 平均差と信頼区間の符号が反転する
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
    // R: 3-2 diff=-5, p adj=0.0008342 → 添字では group1=1, group2=2 の +5
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[2].mean_diff"), 5.0, 1e-9);
    EXPECT_NEAR(json_double(db_, r, "$.comparisons[2].p_value"), 0.00083421, 1e-8);
}

/// @brief 正常系: 3群の全ペア比較なので comparisons は 3 要素
TEST_F(ComplexAggregates, PosthocComparisonCount) {
    create_balanced_group_table(db_);
    for (const char* fn : {"stat_tukey_hsd", "stat_bonferroni_posthoc",
                           "stat_scheffe_posthoc"}) {
        std::string sql = "SELECT json_array_length(";
        sql += fn;
        sql += "(val, grp), '$.comparisons') FROM gb";
        EXPECT_NEAR(query_double(db_, sql.c_str()), 3.0, 1e-9) << fn;
    }
    // Dunnett は対照群との比較のみなので k-1 = 2 要素
    EXPECT_NEAR(query_double(db_,
        "SELECT json_array_length(stat_dunnett_posthoc(val, grp), "
        "'$.comparisons') FROM gb"), 2.0, 1e-9);
}

/// @brief 正常系: alpha を省略すると 0.05, 指定するとその値が使われる
TEST_F(ComplexAggregates, PosthocAlphaIsConfigurable) {
    create_balanced_group_table(db_);
    EXPECT_NEAR(query_double(db_,
        "SELECT json_extract(stat_tukey_hsd(val, grp), '$.alpha') FROM gb"),
        0.05, 1e-9);
    EXPECT_NEAR(query_double(db_,
        "SELECT json_extract(stat_tukey_hsd(val, grp, 0.01), '$.alpha') FROM gb"),
        0.01, 1e-9);
}

/// @brief 正常系: Scheffe は Tukey より保守的なので p 値が大きくなる
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

/// @brief 異常系: 群が 1 つしかない場合は NULL を返す
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

/// @brief 異常系: 空テーブル → NULL
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

/// @brief 異常系: 対照群の添字が群数を超える場合は NULL
TEST_F(ComplexAggregates, DunnettRejectsOutOfRangeControl) {
    create_balanced_group_table(db_);
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_dunnett_posthoc(val, grp, 9, 0.05) FROM gb"));
}

/// @brief 異常系: NULL 行は群分割から除外される
TEST_F(ComplexAggregates, GroupFunctionsSkipNulls) {
    exec_sql(db_, "CREATE TABLE gn(val REAL, grp INT)");
    exec_sql(db_, "INSERT INTO gn VALUES "
                  "(10,1),(NULL,1),(12,1),(14,1),(20,2),(22,2),(NULL,2),(19,2)");
    std::string r = query_text(db_, "SELECT stat_bartlett(val, grp) FROM gn");
    ASSERT_FALSE(r.empty());
    // NULL を除いた 3 件ずつで検定されるため df = k - 1 = 1
    EXPECT_NEAR(json_double(db_, r, "$.df"), 1.0, 1e-9);
}

/// @brief 正常系: 層化抽出は各層から抽出率に応じた件数を取る
TEST_F(ComplexAggregates, StratifiedSampleRespectsRatio) {
    create_balanced_group_table(db_);
    // 各群 5 件 × 3 群. 抽出率 0.4 → 各群 2 件 = 計 6 件
    double n = query_double(db_,
        "SELECT json_array_length(stat_stratified_sample(val, grp, 0.4)) FROM gb");
    EXPECT_NEAR(n, 6.0, 1e-9);
}

/// @brief 境界値: 抽出率 1.0 では全件が返る
TEST_F(ComplexAggregates, StratifiedSampleFullRatio) {
    create_balanced_group_table(db_);
    double n = query_double(db_,
        "SELECT json_array_length(stat_stratified_sample(val, grp, 1.0)) FROM gb");
    EXPECT_NEAR(n, 15.0, 1e-9);
}

/// @brief 正常系: Dunnett は 2/3/4 引数のいずれの形式でも呼べる
///        (末尾のパラメータから順に省略できる)
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
