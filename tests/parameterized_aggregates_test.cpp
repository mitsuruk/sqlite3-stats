/**
 * @file parameterized_aggregates_test.cpp
 * @brief パラメータ付き集約統計関数(20関数)のテスト
 *
 * stat_trimmed_mean, stat_percentile, stat_z_test 等のパラメータ付き
 * 統計集約関数について,正常値・空テーブル・エッジケースを検証する.
 */

#include "test_helpers.hpp"

/// @brief パラメータ付き集約統計関数テスト用フィクスチャ
class ParameterizedAggregates : public StatFuncTest {};

// =====================================================================
// 1. stat_trimmed_mean
// =====================================================================

/// @brief 正常系: 10%トリム平均 → 2-9の平均 = 5.5
TEST_F(ParameterizedAggregates, TrimmedMeanNormal) {
    double result = query_double(
        db_, "SELECT stat_trimmed_mean(val, 0.1) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, TrimmedMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_trimmed_mean(val, 0.1) FROM empty_data"));
}

/// @brief trim=0.0: トリムなし → 通常平均 = 5.5
TEST_F(ParameterizedAggregates, TrimmedMeanNoTrim) {
    double result = query_double(
        db_, "SELECT stat_trimmed_mean(val, 0.0) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-4);
}

// =====================================================================
// 2. stat_percentile
// =====================================================================

/// @brief 正常系: 50パーセンタイル(中央値) → 5.5
TEST_F(ParameterizedAggregates, PercentileMedian) {
    double result = query_double(
        db_, "SELECT stat_percentile(val, 0.5) FROM data");
    EXPECT_NEAR(result, 5.5, 1e-4);
}

/// @brief 0パーセンタイル(最小値) → 1.0
TEST_F(ParameterizedAggregates, PercentileMin) {
    double result = query_double(
        db_, "SELECT stat_percentile(val, 0.0) FROM data");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

/// @brief 100パーセンタイル(最大値) → 10.0
TEST_F(ParameterizedAggregates, PercentileMax) {
    double result = query_double(
        db_, "SELECT stat_percentile(val, 1.0) FROM data");
    EXPECT_NEAR(result, 10.0, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, PercentileEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_percentile(val, 0.5) FROM empty_data"));
}

// =====================================================================
// 3. stat_quartile (JSON返却)
// =====================================================================

/// @brief 正常系: 四分位数 → {"q1": 3.25, "q2": 5.5, "q3": 7.75}
TEST_F(ParameterizedAggregates, QuartileNormal) {
    std::string result = query_text(
        db_, "SELECT stat_quartile(val) FROM data");
    EXPECT_FALSE(result.empty());

    double q1 = json_double(db_, result, "$.q1");
    double q2 = json_double(db_, result, "$.q2");
    double q3 = json_double(db_, result, "$.q3");
    EXPECT_NEAR(q1, 3.25, 1e-4);
    EXPECT_NEAR(q2, 5.5, 1e-4);
    EXPECT_NEAR(q3, 7.75, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, QuartileEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_quartile(val) FROM empty_data"));
}

// =====================================================================
// 4. stat_z_test (JSON返却)
// =====================================================================

/// @brief 正常系: mu0=mean → z=0, p=1
TEST_F(ParameterizedAggregates, ZTestMeanEqualsNull) {
    std::string result = query_text(
        db_, "SELECT stat_z_test(val, 5.5, 3.0) FROM data");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_NEAR(statistic, 0.0, 1e-4);
    EXPECT_NEAR(p_value, 1.0, 1e-4);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, ZTestEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_z_test(val, 5.5, 3.0) FROM empty_data"));
}

// =====================================================================
// 5. stat_t_test (JSON返却)
// =====================================================================

/// @brief 正常系: mu0=mean → t=0, p=1, df=9
TEST_F(ParameterizedAggregates, TTestMeanEqualsNull) {
    std::string result = query_text(
        db_, "SELECT stat_t_test(val, 5.5) FROM data");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    double df = json_double(db_, result, "$.df");
    EXPECT_NEAR(statistic, 0.0, 1e-4);
    EXPECT_NEAR(p_value, 1.0, 1e-4);
    EXPECT_NEAR(df, 9.0, 1e-4);
}

/// @brief mu0=0: 平均から離れた帰無仮説 → p_value が小さい
TEST_F(ParameterizedAggregates, TTestSignificant) {
    std::string result = query_text(
        db_, "SELECT stat_t_test(val, 0) FROM data");
    EXPECT_FALSE(result.empty());

    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_LT(p_value, 0.05);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, TTestEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_t_test(val, 5.5) FROM empty_data"));
}

// =====================================================================
// 6. stat_chisq_gof_uniform (JSON返却)
// =====================================================================

/// @brief 正常系: data2に対する一様性カイ二乗検定
TEST_F(ParameterizedAggregates, ChisqGofUniformNormal) {
    std::string result = query_text(
        db_, "SELECT stat_chisq_gof_uniform(val) FROM data2");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_TRUE(std::isfinite(p_value));
    EXPECT_GE(statistic, 0.0);
    EXPECT_GE(p_value, 0.0);
    EXPECT_LE(p_value, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, ChisqGofUniformEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_chisq_gof_uniform(val) FROM empty_data"));
}

// =====================================================================
// 7. stat_shapiro_wilk (JSON返却)
// =====================================================================

/// @brief 正常系: data(1-10)は正規分布に近い → p_value > 0.05
TEST_F(ParameterizedAggregates, ShapiroWilkNormal) {
    std::string result = query_text(
        db_, "SELECT stat_shapiro_wilk(val) FROM data");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_GT(p_value, 0.05);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, ShapiroWilkEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_shapiro_wilk(val) FROM empty_data"));
}

// =====================================================================
// 8. stat_ks_test (JSON返却)
// =====================================================================

/// @brief 正常系: data(1-10)に対するKS検定
TEST_F(ParameterizedAggregates, KsTestNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ks_test(val) FROM data");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_TRUE(std::isfinite(p_value));
    EXPECT_GE(statistic, 0.0);
    EXPECT_GE(p_value, 0.0);
    EXPECT_LE(p_value, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, KsTestEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ks_test(val) FROM empty_data"));
}

// =====================================================================
// 9. stat_wilcoxon (JSON返却)
// =====================================================================

/// @brief 正常系: data(1-10)に対するウィルコクソン符号順位検定
TEST_F(ParameterizedAggregates, WilcoxonNormal) {
    std::string result = query_text(
        db_, "SELECT stat_wilcoxon(val, 5.5) FROM data");
    EXPECT_FALSE(result.empty());

    double statistic = json_double(db_, result, "$.statistic");
    double p_value = json_double(db_, result, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_TRUE(std::isfinite(p_value));
    EXPECT_GE(p_value, 0.0);
    EXPECT_LE(p_value, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, WilcoxonEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_wilcoxon(val, 5.5) FROM empty_data"));
}

// =====================================================================
// 10. stat_ci_mean (JSON返却)
// =====================================================================

/// @brief 正常系: 95%信頼区間 → lower < 5.5 < upper
TEST_F(ParameterizedAggregates, CiMeanNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ci_mean(val, 0.95) FROM data");
    EXPECT_FALSE(result.empty());

    double lower = json_double(db_, result, "$.lower");
    double upper = json_double(db_, result, "$.upper");
    double point_estimate = json_double(db_, result, "$.point_estimate");
    EXPECT_NEAR(point_estimate, 5.5, 1e-4);
    EXPECT_LT(lower, 5.5);
    EXPECT_GT(upper, 5.5);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, CiMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ci_mean(val, 0.95) FROM empty_data"));
}

// =====================================================================
// 11. stat_ci_mean_z (JSON返却)
// =====================================================================

/// @brief 正常系: 既知の母標準偏差によるZ信頼区間
TEST_F(ParameterizedAggregates, CiMeanZNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ci_mean_z(val, 3.0, 0.95) FROM data");
    EXPECT_FALSE(result.empty());

    double point_estimate = json_double(db_, result, "$.point_estimate");
    double lower = json_double(db_, result, "$.lower");
    double upper = json_double(db_, result, "$.upper");
    EXPECT_NEAR(point_estimate, 5.5, 1e-4);
    EXPECT_LT(lower, 5.5);
    EXPECT_GT(upper, 5.5);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, CiMeanZEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ci_mean_z(val, 3.0, 0.95) FROM empty_data"));
}

// =====================================================================
// 12. stat_ci_var (JSON返却)
// =====================================================================

/// @brief 正常系: 分散の95%信頼区間
TEST_F(ParameterizedAggregates, CiVarNormal) {
    std::string result = query_text(
        db_, "SELECT stat_ci_var(val, 0.95) FROM data");
    EXPECT_FALSE(result.empty());

    double lower = json_double(db_, result, "$.lower");
    double upper = json_double(db_, result, "$.upper");
    EXPECT_TRUE(std::isfinite(lower));
    EXPECT_TRUE(std::isfinite(upper));
    EXPECT_GT(lower, 0.0);
    EXPECT_GT(upper, lower);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, CiVarEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_ci_var(val, 0.95) FROM empty_data"));
}

// =====================================================================
// 13. stat_moe_mean
// =====================================================================

/// @brief 正常系: 誤差限界 → 正の値(約2.16)
TEST_F(ParameterizedAggregates, MoeMeanNormal) {
    double result = query_double(
        db_, "SELECT stat_moe_mean(val, 0.95) FROM data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 2.16, 0.5);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, MoeMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_moe_mean(val, 0.95) FROM empty_data"));
}

// =====================================================================
// 14. stat_cohens_d
// =====================================================================

/// @brief 正常系: mu0=0 → mean/sd ≈ 5.5/3.028 ≈ 1.816
TEST_F(ParameterizedAggregates, CohensDNormal) {
    double result = query_double(
        db_, "SELECT stat_cohens_d(val, 0) FROM data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 1.816, 0.1);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, CohensDEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_cohens_d(val, 0) FROM empty_data"));
}

// =====================================================================
// 15. stat_hedges_g
// =====================================================================

/// @brief 正常系: mu0=0 → cohens_dに近い値(小標本補正で少し小さい)
TEST_F(ParameterizedAggregates, HedgesGNormal) {
    double result = query_double(
        db_, "SELECT stat_hedges_g(val, 0) FROM data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);

    // Cohen's d と比較: Hedges' g は小標本補正で少し小さくなる
    double cohens_d = query_double(
        db_, "SELECT stat_cohens_d(val, 0) FROM data");
    EXPECT_LT(result, cohens_d);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, HedgesGEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_hedges_g(val, 0) FROM empty_data"));
}

// =====================================================================
// 16. stat_acf_lag
// =====================================================================

/// @brief lag=0: 自己相関 → 常に1.0
TEST_F(ParameterizedAggregates, AcfLagZero) {
    double result = query_double(
        db_, "SELECT stat_acf_lag(val, 0) FROM data");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

/// @brief lag=1: -1から1の範囲
TEST_F(ParameterizedAggregates, AcfLagOne) {
    double result = query_double(
        db_, "SELECT stat_acf_lag(val, 1) FROM data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GE(result, -1.0);
    EXPECT_LE(result, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, AcfLagEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_acf_lag(val, 0) FROM empty_data"));
}

// =====================================================================
// 17. stat_biweight_midvar
// =====================================================================

/// @brief 正常系: 正の有限値を返す
TEST_F(ParameterizedAggregates, BiweightMidvarNormal) {
    double result = query_double(
        db_, "SELECT stat_biweight_midvar(val, 9.0) FROM data");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, BiweightMidvarEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_biweight_midvar(val, 9.0) FROM empty_data"));
}

// =====================================================================
// 18. stat_bootstrap_mean (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: estimate ≈ 5.5 (許容 ±1.0)
TEST_F(ParameterizedAggregates, BootstrapMeanNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap_mean(val, 500) FROM data");
    EXPECT_FALSE(result.empty());

    double estimate = json_double(db_, result, "$.estimate");
    double se = json_double(db_, result, "$.standard_error");
    EXPECT_NEAR(estimate, 5.5, 1.0);
    EXPECT_TRUE(std::isfinite(se));
    EXPECT_GT(se, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, BootstrapMeanEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap_mean(val, 500) FROM empty_data"));
}

// =====================================================================
// 19. stat_bootstrap_median (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: estimate ≈ 5.5 (許容 ±1.0)
TEST_F(ParameterizedAggregates, BootstrapMedianNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap_median(val, 500) FROM data");
    EXPECT_FALSE(result.empty());

    double estimate = json_double(db_, result, "$.estimate");
    EXPECT_NEAR(estimate, 5.5, 1.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, BootstrapMedianEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap_median(val, 500) FROM empty_data"));
}

// =====================================================================
// 20. stat_bootstrap_stddev (JSON返却, 非決定的)
// =====================================================================

/// @brief 正常系: estimate が正の有限値
TEST_F(ParameterizedAggregates, BootstrapStddevNormal) {
    std::string result = query_text(
        db_, "SELECT stat_bootstrap_stddev(val, 500) FROM data");
    EXPECT_FALSE(result.empty());

    double estimate = json_double(db_, result, "$.estimate");
    EXPECT_TRUE(std::isfinite(estimate));
    EXPECT_GT(estimate, 0.0);
}

/// @brief 空テーブル → NULL
TEST_F(ParameterizedAggregates, BootstrapStddevEmpty) {
    EXPECT_TRUE(query_is_null(
        db_, "SELECT stat_bootstrap_stddev(val, 500) FROM empty_data"));
}
