/**
 * @file scalar_distributions_test.cpp
 * @brief スカラー分布関数・特殊関数・効果量・検出力分析のテスト
 *
 * 連続分布(pdf/cdf/quantile/rand),離散分布(pmf/cdf/quantile/rand),
 * 組合せ,特殊関数,効果量変換,効果量解釈,検出力分析,MoE/サンプルサイズ
 * の合計83関数をテストする.
 */

#include "test_helpers.hpp"

/// @brief スカラー分布関数テスト用フィクスチャ
class ScalarDistributions : public StatFuncTest {};

// =====================================================================
// 連続分布 (24関数)
// =====================================================================

// ----- 均一分布 -----

/// @brief 均一分布 PDF: U(0,1) で x=0.5 → 1.0
TEST_F(ScalarDistributions, UniformPdf) {
    double result = query_double(db_, "SELECT stat_uniform_pdf(0.5, 0, 1)");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

/// @brief 均一分布 CDF: U(0,1) で x=0.5 → 0.5
TEST_F(ScalarDistributions, UniformCdf) {
    double result = query_double(db_, "SELECT stat_uniform_cdf(0.5, 0, 1)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief 均一分布 Quantile: U(0,1) で p=0.5 → 0.5
TEST_F(ScalarDistributions, UniformQuantile) {
    double result = query_double(db_, "SELECT stat_uniform_quantile(0.5, 0, 1)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief 均一分布 Rand: U(0,1) の乱数は NULL でなく 0-1 の範囲
TEST_F(ScalarDistributions, UniformRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_uniform_rand(0, 1)"));
    double result = query_double(db_, "SELECT stat_uniform_rand(0, 1)");
    EXPECT_GE(result, 0.0);
    EXPECT_LE(result, 1.0);
}

// ----- 指数分布 -----

/// @brief 指数分布 PDF: lambda=1 で x=1 → exp(-1) ≈ 0.367879
TEST_F(ScalarDistributions, ExponentialPdf) {
    double result = query_double(db_, "SELECT stat_exponential_pdf(1.0, 1.0)");
    EXPECT_NEAR(result, 0.367879, 1e-4);
}

/// @brief 指数分布 CDF: lambda=1 で x=1 → 1-exp(-1) ≈ 0.632121
TEST_F(ScalarDistributions, ExponentialCdf) {
    double result = query_double(db_, "SELECT stat_exponential_cdf(1.0, 1.0)");
    EXPECT_NEAR(result, 0.632121, 1e-4);
}

/// @brief 指数分布 Quantile: lambda=1 で p=0.5 → ln(2) ≈ 0.693147
TEST_F(ScalarDistributions, ExponentialQuantile) {
    double result = query_double(db_,
        "SELECT stat_exponential_quantile(0.5, 1.0)");
    EXPECT_NEAR(result, 0.693147, 1e-4);
}

/// @brief 指数分布 Rand: lambda=1 の乱数は NULL でなく正の値
TEST_F(ScalarDistributions, ExponentialRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_exponential_rand(1.0)"));
    double result = query_double(db_, "SELECT stat_exponential_rand(1.0)");
    EXPECT_GT(result, 0.0);
}

// ----- ガンマ分布 -----

/// @brief ガンマ分布 PDF: shape=2, scale=1 で x=2 → 約0.270671
TEST_F(ScalarDistributions, GammaPdf) {
    double result = query_double(db_,
        "SELECT stat_gamma_pdf(2.0, 2.0, 1.0)");
    EXPECT_NEAR(result, 0.270671, 1e-4);
}

/// @brief ガンマ分布 CDF: shape=2, scale=1 で x=2 → 約0.593994
TEST_F(ScalarDistributions, GammaCdf) {
    double result = query_double(db_,
        "SELECT stat_gamma_cdf(2.0, 2.0, 1.0)");
    EXPECT_NEAR(result, 0.593994, 1e-4);
}

/// @brief ガンマ分布 Quantile: shape=2, scale=1 で p=0.5 → 約1.678347
TEST_F(ScalarDistributions, GammaQuantile) {
    double result = query_double(db_,
        "SELECT stat_gamma_quantile(0.5, 2.0, 1.0)");
    EXPECT_NEAR(result, 1.678347, 1e-4);
}

/// @brief ガンマ分布 Rand: NULL でない
TEST_F(ScalarDistributions, GammaRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_gamma_rand(2.0, 1.0)"));
}

// ----- ベータ分布 -----

/// @brief ベータ分布 PDF: alpha=2, beta=2 で x=0.5 → 1.5
TEST_F(ScalarDistributions, BetaPdf) {
    double result = query_double(db_,
        "SELECT stat_beta_pdf(0.5, 2.0, 2.0)");
    EXPECT_NEAR(result, 1.5, 1e-4);
}

/// @brief ベータ分布 CDF: alpha=2, beta=2 で x=0.5 → 0.5
TEST_F(ScalarDistributions, BetaCdf) {
    double result = query_double(db_,
        "SELECT stat_beta_cdf(0.5, 2.0, 2.0)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief ベータ分布 Quantile: alpha=2, beta=2 で p=0.5 → 0.5
TEST_F(ScalarDistributions, BetaQuantile) {
    double result = query_double(db_,
        "SELECT stat_beta_quantile(0.5, 2.0, 2.0)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief ベータ分布 Rand: NULL でない
TEST_F(ScalarDistributions, BetaRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_beta_rand(2.0, 2.0)"));
}

// ----- 対数正規分布 -----

/// @brief 対数正規分布 PDF: mu=0, sigma=1 で x=1 → 約0.398942
TEST_F(ScalarDistributions, LognormalPdf) {
    double result = query_double(db_,
        "SELECT stat_lognormal_pdf(1.0, 0.0, 1.0)");
    EXPECT_NEAR(result, 0.398942, 1e-4);
}

/// @brief 対数正規分布 CDF: mu=0, sigma=1 で x=1 → 0.5
TEST_F(ScalarDistributions, LognormalCdf) {
    double result = query_double(db_,
        "SELECT stat_lognormal_cdf(1.0, 0.0, 1.0)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief 対数正規分布 Quantile: mu=0, sigma=1 で p=0.5 → 1.0
TEST_F(ScalarDistributions, LognormalQuantile) {
    double result = query_double(db_,
        "SELECT stat_lognormal_quantile(0.5, 0.0, 1.0)");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

/// @brief 対数正規分布 Rand: NULL でない
TEST_F(ScalarDistributions, LognormalRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_lognormal_rand(0.0, 1.0)"));
}

// ----- Weibull分布 -----

/// @brief Weibull分布 PDF: shape=1, scale=1 で x=1 → exp(-1) ≈ 0.367879
TEST_F(ScalarDistributions, WeibullPdf) {
    double result = query_double(db_,
        "SELECT stat_weibull_pdf(1.0, 1.0, 1.0)");
    EXPECT_NEAR(result, 0.367879, 1e-4);
}

/// @brief Weibull分布 CDF: shape=1, scale=1 で x=1 → 1-exp(-1) ≈ 0.632121
TEST_F(ScalarDistributions, WeibullCdf) {
    double result = query_double(db_,
        "SELECT stat_weibull_cdf(1.0, 1.0, 1.0)");
    EXPECT_NEAR(result, 0.632121, 1e-4);
}

/// @brief Weibull分布 Quantile: shape=1, scale=1 で p=0.5 → ln(2) ≈ 0.693147
TEST_F(ScalarDistributions, WeibullQuantile) {
    double result = query_double(db_,
        "SELECT stat_weibull_quantile(0.5, 1.0, 1.0)");
    EXPECT_NEAR(result, 0.693147, 1e-4);
}

/// @brief Weibull分布 Rand: NULL でない
TEST_F(ScalarDistributions, WeibullRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_weibull_rand(1.0, 1.0)"));
}

// =====================================================================
// 離散分布 (28関数)
// =====================================================================

// ----- 二項分布 -----

/// @brief 二項分布 PMF: B(10,0.5) で k=3 → 約0.117188
TEST_F(ScalarDistributions, BinomialPmf) {
    double result = query_double(db_,
        "SELECT stat_binomial_pmf(3, 10, 0.5)");
    EXPECT_NEAR(result, 0.117188, 1e-4);
}

/// @brief 二項分布 CDF: B(10,0.5) で k=5 → 約0.623047
TEST_F(ScalarDistributions, BinomialCdf) {
    double result = query_double(db_,
        "SELECT stat_binomial_cdf(5, 10, 0.5)");
    EXPECT_NEAR(result, 0.623047, 1e-4);
}

/// @brief 二項分布 Quantile: B(10,0.5) で p=0.5 → 5
TEST_F(ScalarDistributions, BinomialQuantile) {
    double result = query_double(db_,
        "SELECT stat_binomial_quantile(0.5, 10, 0.5)");
    EXPECT_NEAR(result, 5.0, 1e-4);
}

/// @brief 二項分布 Rand: NULL でない
TEST_F(ScalarDistributions, BinomialRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_binomial_rand(10, 0.5)"));
}

// ----- Poisson分布 -----

/// @brief Poisson分布 PMF: lambda=2 で k=3 → 約0.180447
TEST_F(ScalarDistributions, PoissonPmf) {
    double result = query_double(db_,
        "SELECT stat_poisson_pmf(3, 2.0)");
    EXPECT_NEAR(result, 0.180447, 1e-4);
}

/// @brief Poisson分布 CDF: lambda=2 で k=3 → 約0.857123
TEST_F(ScalarDistributions, PoissonCdf) {
    double result = query_double(db_,
        "SELECT stat_poisson_cdf(3, 2.0)");
    EXPECT_NEAR(result, 0.857123, 1e-4);
}

/// @brief Poisson分布 Quantile: lambda=2 で p=0.5 → 2
TEST_F(ScalarDistributions, PoissonQuantile) {
    double result = query_double(db_,
        "SELECT stat_poisson_quantile(0.5, 2.0)");
    EXPECT_NEAR(result, 2.0, 1e-4);
}

/// @brief Poisson分布 Rand: NULL でない
TEST_F(ScalarDistributions, PoissonRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_poisson_rand(2.0)"));
}

// ----- 幾何分布 -----

/// @brief 幾何分布 PMF: p=0.5 で k=3 → 0.0625 (0-indexed: k回失敗後に成功)
TEST_F(ScalarDistributions, GeometricPmf) {
    double result = query_double(db_,
        "SELECT stat_geometric_pmf(3, 0.5)");
    EXPECT_NEAR(result, 0.0625, 1e-4);
}

/// @brief 幾何分布 CDF: p=0.5 で k=3 → 0.9375 (0-indexed)
TEST_F(ScalarDistributions, GeometricCdf) {
    double result = query_double(db_,
        "SELECT stat_geometric_cdf(3, 0.5)");
    EXPECT_NEAR(result, 0.9375, 1e-4);
}

/// @brief 幾何分布 Quantile: p=0.5 で q=0.5 → 0 (0-indexed)
TEST_F(ScalarDistributions, GeometricQuantile) {
    double result = query_double(db_,
        "SELECT stat_geometric_quantile(0.5, 0.5)");
    EXPECT_NEAR(result, 0.0, 1e-4);
}

/// @brief 幾何分布 Rand: NULL でない
TEST_F(ScalarDistributions, GeometricRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_geometric_rand(0.5)"));
}

// ----- 負二項分布 -----

/// @brief 負二項分布 PMF: r=5, p=0.5 で k=3 → 約0.136719
TEST_F(ScalarDistributions, NbinomPmf) {
    double result = query_double(db_,
        "SELECT stat_nbinom_pmf(3, 5, 0.5)");
    EXPECT_NEAR(result, 0.136719, 1e-4);
}

/// @brief 負二項分布 CDF: r=5, p=0.5 で k=3 → 約0.363281
TEST_F(ScalarDistributions, NbinomCdf) {
    double result = query_double(db_,
        "SELECT stat_nbinom_cdf(3, 5, 0.5)");
    EXPECT_NEAR(result, 0.363281, 1e-4);
}

/// @brief 負二項分布 Quantile: r=5, p=0.5 で q=0.5 → 5付近
TEST_F(ScalarDistributions, NbinomQuantile) {
    double result = query_double(db_,
        "SELECT stat_nbinom_quantile(0.5, 5, 0.5)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GE(result, 3.0);
    EXPECT_LE(result, 7.0);
}

/// @brief 負二項分布 Rand: NULL でない
TEST_F(ScalarDistributions, NbinomRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_nbinom_rand(5, 0.5)"));
}

// ----- 超幾何分布 -----

/// @brief 超幾何分布 PMF: N=20, K=10, n=5 で k=2 → C(10,2)*C(10,3)/C(20,5)
TEST_F(ScalarDistributions, HypergeomPmf) {
    double result = query_double(db_,
        "SELECT stat_hypergeom_pmf(2, 20, 10, 5)");
    // C(10,2)*C(10,3)/C(20,5) = 45*120/15504 ≈ 0.347912
    EXPECT_NEAR(result, 0.3483, 1e-2);
}

/// @brief 超幾何分布 CDF: N=20, K=10, n=5 で k=2
TEST_F(ScalarDistributions, HypergeomCdf) {
    double result = query_double(db_,
        "SELECT stat_hypergeom_cdf(2, 20, 10, 5)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
    EXPECT_LE(result, 1.0);
}

/// @brief 超幾何分布 Quantile: N=20, K=10, n=5 で p=0.5 → 2 か 3
TEST_F(ScalarDistributions, HypergeomQuantile) {
    double result = query_double(db_,
        "SELECT stat_hypergeom_quantile(0.5, 20, 10, 5)");
    EXPECT_TRUE(result == 2.0 || result == 3.0);
}

/// @brief 超幾何分布 Rand: NULL でない
TEST_F(ScalarDistributions, HypergeomRand) {
    EXPECT_FALSE(query_is_null(db_,
        "SELECT stat_hypergeom_rand(20, 10, 5)"));
}

// ----- Bernoulli分布 -----

/// @brief Bernoulli分布 PMF: p=0.3 で k=1 → 0.3
TEST_F(ScalarDistributions, BernoulliPmf) {
    double result = query_double(db_,
        "SELECT stat_bernoulli_pmf(1, 0.3)");
    EXPECT_NEAR(result, 0.3, 1e-4);
}

/// @brief Bernoulli分布 CDF: p=0.3 で k=0 → 0.7
TEST_F(ScalarDistributions, BernoulliCdf) {
    double result = query_double(db_,
        "SELECT stat_bernoulli_cdf(0, 0.3)");
    EXPECT_NEAR(result, 0.7, 1e-4);
}

/// @brief Bernoulli分布 Quantile: p=0.3 で q=0.5 → 0 (P(X<=0)=0.7 >= 0.5)
TEST_F(ScalarDistributions, BernoulliQuantile) {
    double result = query_double(db_,
        "SELECT stat_bernoulli_quantile(0.5, 0.3)");
    EXPECT_NEAR(result, 0.0, 1e-4);
}

/// @brief Bernoulli分布 Rand: NULL でない
TEST_F(ScalarDistributions, BernoulliRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_bernoulli_rand(0.3)"));
}

// ----- 離散均一分布 -----

/// @brief 離散均一分布 PMF: [1,6] で k=3 → 1/6 ≈ 0.166667
TEST_F(ScalarDistributions, DuniformPmf) {
    double result = query_double(db_,
        "SELECT stat_duniform_pmf(3, 1, 6)");
    EXPECT_NEAR(result, 0.166667, 1e-4);
}

/// @brief 離散均一分布 CDF: [1,6] で k=3 → 0.5
TEST_F(ScalarDistributions, DuniformCdf) {
    double result = query_double(db_,
        "SELECT stat_duniform_cdf(3, 1, 6)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief 離散均一分布 Quantile: [1,6] で p=0.5 → 3
TEST_F(ScalarDistributions, DuniformQuantile) {
    double result = query_double(db_,
        "SELECT stat_duniform_quantile(0.5, 1, 6)");
    EXPECT_NEAR(result, 3.0, 1e-4);
}

/// @brief 離散均一分布 Rand: NULL でない
TEST_F(ScalarDistributions, DuniformRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_duniform_rand(1, 6)"));
}

// =====================================================================
// 組合せ (3関数)
// =====================================================================

/// @brief 二項係数: C(10,3) = 120
TEST_F(ScalarDistributions, BinomialCoef) {
    double result = query_double(db_, "SELECT stat_binomial_coef(10, 3)");
    EXPECT_NEAR(result, 120.0, 1e-4);
}

/// @brief 対数二項係数: log(C(10,3)) = log(120) ≈ 4.787492
TEST_F(ScalarDistributions, LogBinomialCoef) {
    double result = query_double(db_,
        "SELECT stat_log_binomial_coef(10, 3)");
    EXPECT_NEAR(result, 4.787492, 1e-4);
}

/// @brief 対数階乗: log(5!) = log(120) ≈ 4.787492
TEST_F(ScalarDistributions, LogFactorial) {
    double result = query_double(db_, "SELECT stat_log_factorial(5)");
    EXPECT_NEAR(result, 4.787492, 1e-4);
}

// =====================================================================
// 特殊関数 (7関数)
// =====================================================================

/// @brief lgamma: lgamma(5) = log(4!) = log(24) ≈ 3.178054
TEST_F(ScalarDistributions, Lgamma) {
    double result = query_double(db_, "SELECT stat_lgamma(5)");
    EXPECT_NEAR(result, 3.178054, 1e-4);
}

/// @brief tgamma: tgamma(5) = 4! = 24.0
TEST_F(ScalarDistributions, Tgamma) {
    double result = query_double(db_, "SELECT stat_tgamma(5)");
    EXPECT_NEAR(result, 24.0, 1e-4);
}

/// @brief ベータ関数: B(2,3) = 1/12 ≈ 0.083333
TEST_F(ScalarDistributions, BetaFunc) {
    double result = query_double(db_, "SELECT stat_beta_func(2, 3)");
    EXPECT_NEAR(result, 0.083333, 1e-4);
}

/// @brief 対数ベータ関数: lbeta(2,3) = log(1/12) ≈ -2.484907
TEST_F(ScalarDistributions, Lbeta) {
    double result = query_double(db_, "SELECT stat_lbeta(2, 3)");
    EXPECT_NEAR(result, -2.484907, 1e-4);
}

/// @brief 誤差関数: erf(1.0) ≈ 0.842701
TEST_F(ScalarDistributions, Erf) {
    double result = query_double(db_, "SELECT stat_erf(1.0)");
    EXPECT_NEAR(result, 0.842701, 1e-4);
}

/// @brief 相補誤差関数: erfc(1.0) ≈ 0.157299
TEST_F(ScalarDistributions, Erfc) {
    double result = query_double(db_, "SELECT stat_erfc(1.0)");
    EXPECT_NEAR(result, 0.157299, 1e-4);
}

/// @brief 対数平均: logarithmic_mean(2, 8) = (8-2)/ln(8/2) = 6/ln(4) ≈ 4.328085
TEST_F(ScalarDistributions, LogarithmicMean) {
    double result = query_double(db_,
        "SELECT stat_logarithmic_mean(2, 8)");
    EXPECT_NEAR(result, 4.328085, 1e-4);
}

// =====================================================================
// 効果量変換 (8関数)
// =====================================================================

/// @brief Hedges' J 補正: df=10 → 有限の正値
TEST_F(ScalarDistributions, HedgesJ) {
    double result = query_double(db_, "SELECT stat_hedges_j(10)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
}

/// @brief t値からrへの変換: t=2.0, df=10 → 約0.534522
TEST_F(ScalarDistributions, TToR) {
    double result = query_double(db_, "SELECT stat_t_to_r(2.0, 10)");
    EXPECT_NEAR(result, 0.534522, 1e-4);
}

/// @brief Cohen's d から r への変換: d=0.5 → 約0.242536
TEST_F(ScalarDistributions, DToR) {
    double result = query_double(db_, "SELECT stat_d_to_r(0.5)");
    EXPECT_NEAR(result, 0.242536, 1e-4);
}

/// @brief r から Cohen's d への変換: r=0.5 → 約1.154701
TEST_F(ScalarDistributions, RToD) {
    double result = query_double(db_, "SELECT stat_r_to_d(0.5)");
    EXPECT_NEAR(result, 1.154701, 1e-4);
}

/// @brief イータ二乗: ss_effect=100, ss_total=400 → 0.25
TEST_F(ScalarDistributions, EtaSquaredEf) {
    double result = query_double(db_,
        "SELECT stat_eta_squared_ef(100, 400)");
    EXPECT_NEAR(result, 0.25, 1e-4);
}

/// @brief 偏イータ二乗: F=5.0, df1=2, df2=30 → F*df1/(F*df1+df2) = 10/40 = 0.25
TEST_F(ScalarDistributions, PartialEtaSq) {
    double result = query_double(db_,
        "SELECT stat_partial_eta_sq(5.0, 2, 30)");
    EXPECT_NEAR(result, 0.25, 1e-4);
}

/// @brief オメガ二乗: 結果が有限
TEST_F(ScalarDistributions, OmegaSquaredEf) {
    double result = query_double(db_,
        "SELECT stat_omega_squared_ef(100, 400, 10, 2)");
    EXPECT_TRUE(std::isfinite(result));
}

/// @brief Cohen's h: 2*arcsin(sqrt(0.6)) - 2*arcsin(sqrt(0.4)) ≈ 0.407...
TEST_F(ScalarDistributions, CohensH) {
    double result = query_double(db_,
        "SELECT stat_cohens_h(0.6, 0.4)");
    EXPECT_NEAR(result, 0.4072, 1e-2);
}

// =====================================================================
// 効果量解釈 (3関数) - TEXT返却
// =====================================================================

/// @brief Cohen's d の解釈: d=0.3 → "small"
TEST_F(ScalarDistributions, InterpretD) {
    std::string result = query_text(db_, "SELECT stat_interpret_d(0.3)");
    EXPECT_EQ(result, "small");
}

/// @brief 相関係数 r の解釈: r=0.1 → "small"
TEST_F(ScalarDistributions, InterpretR) {
    std::string result = query_text(db_, "SELECT stat_interpret_r(0.1)");
    EXPECT_EQ(result, "small");
}

/// @brief イータ二乗の解釈: eta2=0.01 → "small"
TEST_F(ScalarDistributions, InterpretEta2) {
    std::string result = query_text(db_,
        "SELECT stat_interpret_eta2(0.01)");
    EXPECT_EQ(result, "small");
}

// =====================================================================
// 検出力分析 (6関数)
// =====================================================================

/// @brief 1標本t検定の検出力: d=0.5, n=30, alpha=0.05 → 0.0-1.0の範囲
TEST_F(ScalarDistributions, PowerT1) {
    double result = query_double(db_,
        "SELECT stat_power_t1(0.5, 30, 0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
}

/// @brief 1標本t検定のサンプルサイズ: d=0.5, power=0.8, alpha=0.05 → 正の整数値
TEST_F(ScalarDistributions, NT1) {
    double result = query_double(db_,
        "SELECT stat_n_t1(0.5, 0.8, 0.05)");
    EXPECT_GT(result, 0.0);
}

/// @brief 2標本t検定の検出力: d=0.5, n1=30, n2=30, alpha=0.05 → 0.0-1.0
TEST_F(ScalarDistributions, PowerT2) {
    double result = query_double(db_,
        "SELECT stat_power_t2(0.5, 30, 30, 0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
}

/// @brief 2標本t検定のサンプルサイズ: d=0.5, power=0.8, alpha=0.05 → 正の値
TEST_F(ScalarDistributions, NT2) {
    double result = query_double(db_,
        "SELECT stat_n_t2(0.5, 0.8, 0.05)");
    EXPECT_GT(result, 0.0);
}

/// @brief 比率検定の検出力: p1=0.5, p2=0.3, n=100, alpha=0.05 → 0.0-1.0
TEST_F(ScalarDistributions, PowerProp) {
    double result = query_double(db_,
        "SELECT stat_power_prop(0.5, 0.3, 100, 0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
}

/// @brief 比率検定のサンプルサイズ: p1=0.5, p2=0.3, power=0.8, alpha=0.05 → 正の値
TEST_F(ScalarDistributions, NProp) {
    double result = query_double(db_,
        "SELECT stat_n_prop(0.5, 0.3, 0.8, 0.05)");
    EXPECT_GT(result, 0.0);
}

// =====================================================================
// MoE/サンプルサイズ (4関数)
// =====================================================================

/// @brief 比率の誤差限界(MoE): x=50, n=100 → 正の値(約0.098)
TEST_F(ScalarDistributions, MoeProp) {
    double result = query_double(db_, "SELECT stat_moe_prop(50, 100)");
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 0.098, 1e-2);
}

/// @brief 比率の最悪ケースMoE: n=100 → 正の値(約0.098)
TEST_F(ScalarDistributions, MoePropWorst) {
    double result = query_double(db_,
        "SELECT stat_moe_prop_worst(100)");
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 0.098, 1e-2);
}

/// @brief 比率のMoEに必要なサンプルサイズ: moe=0.05 → 正の値(約385)
TEST_F(ScalarDistributions, NMoeProp) {
    double result = query_double(db_, "SELECT stat_n_moe_prop(0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 385.0, 5.0);
}

/// @brief 平均のMoEに必要なサンプルサイズ: moe=1.0, sd=5.0 → 正の値
TEST_F(ScalarDistributions, NMoeMean) {
    double result = query_double(db_,
        "SELECT stat_n_moe_mean(1.0, 5.0)");
    EXPECT_GT(result, 0.0);
}
