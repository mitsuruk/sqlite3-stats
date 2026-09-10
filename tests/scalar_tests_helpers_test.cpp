/**
 * @file scalar_tests_helpers_test.cpp
 * @brief Tests for the scalar statistical functions (38 functions)
 *
 * Verifies the normal, chi-square, t and F distributions, the special functions,
 * proportion tests, multiple testing corrections, Fisher's exact test and risk
 * measures, proportion confidence intervals, model selection and the Box-Cox transform.
 */

#include "test_helpers.hpp"

/// @brief Fixture for the scalar statistical function tests
class ScalarTestsHelpers : public StatFuncTest {};

// =====================================================================
// Normal distribution (4 functions)
// =====================================================================

/// @brief Normal case: stat_normal_pdf(0) -> standard normal PDF, about 0.398942
TEST_F(ScalarTestsHelpers, NormalPdfStandard) {
    double result = query_double(db_, "SELECT stat_normal_pdf(0)");
    EXPECT_NEAR(result, 0.398942, 1e-4);
}

/// @brief Normal case: stat_normal_pdf(x, mu, sigma), the three-argument form
TEST_F(ScalarTestsHelpers, NormalPdfWithParams) {
    double result = query_double(db_, "SELECT stat_normal_pdf(0, 0, 1)");
    EXPECT_NEAR(result, 0.398942, 1e-4);
}

/// @brief Normal case: stat_normal_cdf(1.96) -> about 0.975
TEST_F(ScalarTestsHelpers, NormalCdf) {
    double result = query_double(db_, "SELECT stat_normal_cdf(1.96)");
    EXPECT_NEAR(result, 0.975, 1e-3);
}

/// @brief Normal case: stat_normal_quantile(0.975) -> about 1.96
TEST_F(ScalarTestsHelpers, NormalQuantile) {
    double result = query_double(db_, "SELECT stat_normal_quantile(0.975)");
    EXPECT_NEAR(result, 1.96, 1e-2);
}

/// @brief Normal case: stat_normal_rand() -> not NULL (non-deterministic)
TEST_F(ScalarTestsHelpers, NormalRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_normal_rand()"));
}

// =====================================================================
// Chi-square distribution (4 functions)
// =====================================================================

/// @brief Normal case: stat_chisq_pdf(5, 3) -> a finite positive value
TEST_F(ScalarTestsHelpers, ChisqPdf) {
    double result = query_double(db_, "SELECT stat_chisq_pdf(5, 3)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
}

/// @brief Normal case: stat_chisq_cdf(7.815, 3) -> about 0.95
TEST_F(ScalarTestsHelpers, ChisqCdf) {
    double result = query_double(db_, "SELECT stat_chisq_cdf(7.815, 3)");
    EXPECT_NEAR(result, 0.95, 1e-2);
}

/// @brief Normal case: stat_chisq_quantile(0.95, 3) -> about 7.815
TEST_F(ScalarTestsHelpers, ChisqQuantile) {
    double result = query_double(db_, "SELECT stat_chisq_quantile(0.95, 3)");
    EXPECT_NEAR(result, 7.815, 1e-2);
}

/// @brief Normal case: stat_chisq_rand(5) -> not NULL (non-deterministic)
TEST_F(ScalarTestsHelpers, ChisqRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_chisq_rand(5)"));
}

// =====================================================================
// t distribution (4 functions)
// =====================================================================

/// @brief Normal case: stat_t_pdf(0, 10) -> a finite positive value
TEST_F(ScalarTestsHelpers, TPdf) {
    double result = query_double(db_, "SELECT stat_t_pdf(0, 10)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
}

/// @brief Normal case: stat_t_cdf(2.228, 10) -> about 0.975
TEST_F(ScalarTestsHelpers, TCdf) {
    double result = query_double(db_, "SELECT stat_t_cdf(2.228, 10)");
    EXPECT_NEAR(result, 0.975, 1e-2);
}

/// @brief Normal case: stat_t_quantile(0.975, 10) -> about 2.228
TEST_F(ScalarTestsHelpers, TQuantile) {
    double result = query_double(db_, "SELECT stat_t_quantile(0.975, 10)");
    EXPECT_NEAR(result, 2.228, 1e-2);
}

/// @brief Normal case: stat_t_rand(10) -> not NULL (non-deterministic)
TEST_F(ScalarTestsHelpers, TRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_t_rand(10)"));
}

// =====================================================================
// F distribution (4 functions)
// =====================================================================

/// @brief Normal case: stat_f_pdf(1, 5, 10) -> a finite positive value
TEST_F(ScalarTestsHelpers, FPdf) {
    double result = query_double(db_, "SELECT stat_f_pdf(1, 5, 10)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
}

/// @brief Normal case: stat_f_cdf(3.326, 5, 10) -> about 0.95
TEST_F(ScalarTestsHelpers, FCdf) {
    double result = query_double(db_, "SELECT stat_f_cdf(3.326, 5, 10)");
    EXPECT_NEAR(result, 0.95, 1e-2);
}

/// @brief Normal case: stat_f_quantile(0.95, 5, 10) -> about 3.326
TEST_F(ScalarTestsHelpers, FQuantile) {
    double result = query_double(db_, "SELECT stat_f_quantile(0.95, 5, 10)");
    EXPECT_NEAR(result, 3.326, 1e-2);
}

/// @brief Normal case: stat_f_rand(5, 10) -> not NULL (non-deterministic)
TEST_F(ScalarTestsHelpers, FRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_f_rand(5, 10)"));
}

// =====================================================================
// Special functions (7 functions)
// =====================================================================

/// @brief Normal case: stat_betainc(1.0, 1.0, 0.5) -> 0.5
TEST_F(ScalarTestsHelpers, BetaInc) {
    double result = query_double(
        db_, "SELECT stat_betainc(1.0, 1.0, 0.5)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Normal case: stat_betaincinv(1.0, 1.0, 0.5) -> 0.5
TEST_F(ScalarTestsHelpers, BetaIncInv) {
    double result = query_double(
        db_, "SELECT stat_betaincinv(1.0, 1.0, 0.5)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Normal case: stat_norm_cdf(0) -> 0.5
TEST_F(ScalarTestsHelpers, NormCdf) {
    double result = query_double(db_, "SELECT stat_norm_cdf(0)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Normal case: stat_norm_quantile(0.5) -> 0.0
TEST_F(ScalarTestsHelpers, NormQuantile) {
    double result = query_double(db_, "SELECT stat_norm_quantile(0.5)");
    EXPECT_NEAR(result, 0.0, 1e-4);
}

/// @brief Normal case: stat_gammainc_lower(1.0, 1.0) -> about 0.632121
TEST_F(ScalarTestsHelpers, GammaIncLower) {
    double result = query_double(
        db_, "SELECT stat_gammainc_lower(1.0, 1.0)");
    EXPECT_NEAR(result, 0.632121, 1e-4);
}

/// @brief Normal case: stat_gammainc_upper(1.0, 1.0) -> about 0.367879
TEST_F(ScalarTestsHelpers, GammaIncUpper) {
    double result = query_double(
        db_, "SELECT stat_gammainc_upper(1.0, 1.0)");
    EXPECT_NEAR(result, 0.367879, 1e-4);
}

/// @brief Normal case: stat_gammainc_lower_inv(1.0, 0.5) -> about 0.693147
TEST_F(ScalarTestsHelpers, GammaIncLowerInv) {
    double result = query_double(
        db_, "SELECT stat_gammainc_lower_inv(1.0, 0.5)");
    EXPECT_NEAR(result, 0.693147, 1e-4);
}

// =====================================================================
// Proportion tests (2 functions) - return JSON
// =====================================================================

/// @brief Normal case: stat_z_test_prop(50, 100, 0.5) -> statistic=0, p_value=1.0
TEST_F(ScalarTestsHelpers, ZTestProp) {
    std::string json = query_text(
        db_, "SELECT stat_z_test_prop(50, 100, 0.5)");
    EXPECT_FALSE(json.empty());
    double statistic = json_double(db_, json, "$.statistic");
    double p_value = json_double(db_, json, "$.p_value");
    EXPECT_NEAR(statistic, 0.0, 1e-4);
    EXPECT_NEAR(p_value, 1.0, 1e-4);
}

/// @brief Normal case: stat_z_test_prop2(30, 100, 50, 100) -> JSON with finite values
TEST_F(ScalarTestsHelpers, ZTestProp2) {
    std::string json = query_text(
        db_, "SELECT stat_z_test_prop2(30, 100, 50, 100)");
    EXPECT_FALSE(json.empty());
    double statistic = json_double(db_, json, "$.statistic");
    double p_value = json_double(db_, json, "$.p_value");
    EXPECT_TRUE(std::isfinite(statistic));
    EXPECT_TRUE(std::isfinite(p_value));
    EXPECT_GE(p_value, 0.0);
    EXPECT_LE(p_value, 1.0);
}

// =====================================================================
// Multiple testing corrections (1 function - BH and Holm moved to window functions)
// =====================================================================

/// @brief Normal case: stat_bonferroni(0.01, 5) -> 0.05
TEST_F(ScalarTestsHelpers, Bonferroni) {
    double result = query_double(db_, "SELECT stat_bonferroni(0.01, 5)");
    EXPECT_NEAR(result, 0.05, 1e-4);
}

/// @brief Boundary: stat_bonferroni clamps at 1.0
TEST_F(ScalarTestsHelpers, BonferroniClampsAtOne) {
    double result = query_double(db_, "SELECT stat_bonferroni(0.5, 10)");
    EXPECT_NEAR(result, 1.0, 1e-9);
}

// BH and Holm corrections need the whole set of p-values to enforce monotonicity,
// so no scalar form is provided. Their window-function tests live in
// window_functions_test.cpp.

// =====================================================================
// Fisher's exact test and risk measures (5 functions)
// =====================================================================

/// @brief Normal case: stat_fisher_exact(10, 5, 3, 12) -> JSON with a finite p_value
TEST_F(ScalarTestsHelpers, FisherExact) {
    std::string json = query_text(
        db_, "SELECT stat_fisher_exact(10, 5, 3, 12)");
    EXPECT_FALSE(json.empty());
    double p_value = json_double(db_, json, "$.p_value");
    EXPECT_TRUE(std::isfinite(p_value));
    EXPECT_GE(p_value, 0.0);
    EXPECT_LE(p_value, 1.0);
}

/// @brief Normal case: stat_odds_ratio(10, 5, 3, 12) -> JSON, odds_ratio = 8.0
TEST_F(ScalarTestsHelpers, OddsRatio) {
    std::string json = query_text(
        db_, "SELECT stat_odds_ratio(10, 5, 3, 12)");
    EXPECT_FALSE(json.empty());
    double odds_ratio = json_double(db_, json, "$.odds_ratio");
    EXPECT_NEAR(odds_ratio, 8.0, 1e-4);
}

/// @brief Normal case: stat_relative_risk(10, 5, 3, 12) -> JSON, relative_risk ~ 3.333
TEST_F(ScalarTestsHelpers, RelativeRisk) {
    std::string json = query_text(
        db_, "SELECT stat_relative_risk(10, 5, 3, 12)");
    EXPECT_FALSE(json.empty());
    double rr = json_double(db_, json, "$.relative_risk");
    EXPECT_NEAR(rr, 10.0 / 3.0, 1e-4);
}

/// @brief Normal case: stat_risk_difference(10, 5, 3, 12) -> JSON, risk_difference ~ 0.467
TEST_F(ScalarTestsHelpers, RiskDifference) {
    std::string json = query_text(
        db_, "SELECT stat_risk_difference(10, 5, 3, 12)");
    EXPECT_FALSE(json.empty());
    double rd = json_double(db_, json, "$.risk_difference");
    EXPECT_NEAR(rd, 7.0 / 15.0, 1e-4);
}

/// @brief Normal case: stat_nnt(10, 5, 3, 12) -> 1/risk_difference ~ 2.143
TEST_F(ScalarTestsHelpers, Nnt) {
    double result = query_double(
        db_, "SELECT stat_nnt(10, 5, 3, 12)");
    EXPECT_NEAR(result, 15.0 / 7.0, 1e-4);
}

// =====================================================================
// Proportion confidence intervals (3 functions) - return JSON
// =====================================================================

/// @brief Normal case: stat_ci_prop(50, 100, 0.95) -> lower and upper within 0-1
TEST_F(ScalarTestsHelpers, CiProp) {
    std::string json = query_text(
        db_, "SELECT stat_ci_prop(50, 100, 0.95)");
    EXPECT_FALSE(json.empty());
    double lower = json_double(db_, json, "$.lower");
    double upper = json_double(db_, json, "$.upper");
    EXPECT_GE(lower, 0.0);
    EXPECT_LE(upper, 1.0);
    EXPECT_LT(lower, upper);
}

/// @brief Normal case: stat_ci_prop_wilson(50, 100, 0.95) -> lower and upper within 0-1
TEST_F(ScalarTestsHelpers, CiPropWilson) {
    std::string json = query_text(
        db_, "SELECT stat_ci_prop_wilson(50, 100, 0.95)");
    EXPECT_FALSE(json.empty());
    double lower = json_double(db_, json, "$.lower");
    double upper = json_double(db_, json, "$.upper");
    EXPECT_GE(lower, 0.0);
    EXPECT_LE(upper, 1.0);
    EXPECT_LT(lower, upper);
}

/// @brief Normal case: stat_ci_prop_diff(30, 100, 50, 100, 0.95) -> JSON with finite values
TEST_F(ScalarTestsHelpers, CiPropDiff) {
    std::string json = query_text(
        db_, "SELECT stat_ci_prop_diff(30, 100, 50, 100, 0.95)");
    EXPECT_FALSE(json.empty());
    double lower = json_double(db_, json, "$.lower");
    double upper = json_double(db_, json, "$.upper");
    EXPECT_TRUE(std::isfinite(lower));
    EXPECT_TRUE(std::isfinite(upper));
    EXPECT_LT(lower, upper);
}

// =====================================================================
// Model selection (3 functions)
// =====================================================================

/// @brief Normal case: stat_aic(-100, 3) -> -2*(-100) + 2*3 = 206.0
TEST_F(ScalarTestsHelpers, Aic) {
    double result = query_double(db_, "SELECT stat_aic(-100, 3)");
    EXPECT_NEAR(result, 206.0, 1e-4);
}

/// @brief Normal case: stat_aicc(-100, 50, 3) -> 206 + 2*3*(3+1)/(50-3-1) ~ 206.261
TEST_F(ScalarTestsHelpers, Aicc) {
    double result = query_double(db_, "SELECT stat_aicc(-100, 50, 3)");
    double expected = 206.0 + 2.0 * 3.0 * 4.0 / 46.0;
    EXPECT_NEAR(result, expected, 1e-2);
}

/// @brief Normal case: stat_bic(-100, 50, 3) -> -2*(-100) + 3*log(50) ~ 211.733
TEST_F(ScalarTestsHelpers, Bic) {
    double result = query_double(db_, "SELECT stat_bic(-100, 50, 3)");
    double expected = 200.0 + 3.0 * std::log(50.0);
    EXPECT_NEAR(result, expected, 1e-2);
}

// =====================================================================
// Box-Cox transform (1 function)
// =====================================================================

/// @brief Normal case: stat_boxcox(2.0, 1.0) -> x-1 = 1.0 (lambda=1)
TEST_F(ScalarTestsHelpers, BoxCoxLambda1) {
    double result = query_double(db_, "SELECT stat_boxcox(2.0, 1.0)");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

/// @brief Normal case: stat_boxcox(2.0, 0.0) -> log(2) ~ 0.693147 (lambda=0)
TEST_F(ScalarTestsHelpers, BoxCoxLambda0) {
    double result = query_double(db_, "SELECT stat_boxcox(2.0, 0.0)");
    EXPECT_NEAR(result, 0.693147, 1e-4);
}

/// @brief Normal case: stat_boxcox(2.0, 2.0) -> (2^2 - 1)/2 = 1.5 (lambda=2)
TEST_F(ScalarTestsHelpers, BoxCoxLambda2) {
    double result = query_double(db_, "SELECT stat_boxcox(2.0, 2.0)");
    EXPECT_NEAR(result, 1.5, 1e-4);
}
