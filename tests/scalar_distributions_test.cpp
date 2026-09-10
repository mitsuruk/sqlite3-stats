/**
 * @file scalar_distributions_test.cpp
 * @brief Tests for the scalar distribution, special, effect size and power functions
 *
 * Covers continuous distributions (pdf/cdf/quantile/rand), discrete distributions
 * (pmf/cdf/quantile/rand), combinatorics, special functions, effect size conversion
 * and interpretation, power analysis and MoE/sample size: 83 functions in total.
 */

#include "test_helpers.hpp"

/// @brief Fixture for the scalar distribution function tests
class ScalarDistributions : public StatFuncTest {};

// =====================================================================
// Continuous distributions (24 functions)
// =====================================================================

// ----- Uniform distribution -----

/// @brief Uniform PDF: U(0,1) at x=0.5 -> 1.0
TEST_F(ScalarDistributions, UniformPdf) {
    double result = query_double(db_, "SELECT stat_uniform_pdf(0.5, 0, 1)");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

/// @brief Uniform CDF: U(0,1) at x=0.5 -> 0.5
TEST_F(ScalarDistributions, UniformCdf) {
    double result = query_double(db_, "SELECT stat_uniform_cdf(0.5, 0, 1)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Uniform Quantile: U(0,1) at p=0.5 -> 0.5
TEST_F(ScalarDistributions, UniformQuantile) {
    double result = query_double(db_, "SELECT stat_uniform_quantile(0.5, 0, 1)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Uniform Rand: a U(0,1) draw is not NULL and lies within 0-1
TEST_F(ScalarDistributions, UniformRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_uniform_rand(0, 1)"));
    double result = query_double(db_, "SELECT stat_uniform_rand(0, 1)");
    EXPECT_GE(result, 0.0);
    EXPECT_LE(result, 1.0);
}

// ----- Exponential distribution -----

/// @brief Exponential PDF: lambda=1 at x=1 -> exp(-1) ~ 0.367879
TEST_F(ScalarDistributions, ExponentialPdf) {
    double result = query_double(db_, "SELECT stat_exponential_pdf(1.0, 1.0)");
    EXPECT_NEAR(result, 0.367879, 1e-4);
}

/// @brief Exponential CDF: lambda=1 at x=1 -> 1-exp(-1) ~ 0.632121
TEST_F(ScalarDistributions, ExponentialCdf) {
    double result = query_double(db_, "SELECT stat_exponential_cdf(1.0, 1.0)");
    EXPECT_NEAR(result, 0.632121, 1e-4);
}

/// @brief Exponential Quantile: lambda=1 at p=0.5 -> ln(2) ~ 0.693147
TEST_F(ScalarDistributions, ExponentialQuantile) {
    double result = query_double(db_,
        "SELECT stat_exponential_quantile(0.5, 1.0)");
    EXPECT_NEAR(result, 0.693147, 1e-4);
}

/// @brief Exponential Rand: a lambda=1 draw is not NULL and positive
TEST_F(ScalarDistributions, ExponentialRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_exponential_rand(1.0)"));
    double result = query_double(db_, "SELECT stat_exponential_rand(1.0)");
    EXPECT_GT(result, 0.0);
}

// ----- Gamma distribution -----

/// @brief Gamma PDF: shape=2, scale=1 at x=2 -> about 0.270671
TEST_F(ScalarDistributions, GammaPdf) {
    double result = query_double(db_,
        "SELECT stat_gamma_pdf(2.0, 2.0, 1.0)");
    EXPECT_NEAR(result, 0.270671, 1e-4);
}

/// @brief Gamma CDF: shape=2, scale=1 at x=2 -> about 0.593994
TEST_F(ScalarDistributions, GammaCdf) {
    double result = query_double(db_,
        "SELECT stat_gamma_cdf(2.0, 2.0, 1.0)");
    EXPECT_NEAR(result, 0.593994, 1e-4);
}

/// @brief Gamma Quantile: shape=2, scale=1 at p=0.5 -> about 1.678347
TEST_F(ScalarDistributions, GammaQuantile) {
    double result = query_double(db_,
        "SELECT stat_gamma_quantile(0.5, 2.0, 1.0)");
    EXPECT_NEAR(result, 1.678347, 1e-4);
}

/// @brief Gamma Rand: not NULL
TEST_F(ScalarDistributions, GammaRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_gamma_rand(2.0, 1.0)"));
}

// ----- Beta distribution -----

/// @brief Beta PDF: alpha=2, beta=2 at x=0.5 -> 1.5
TEST_F(ScalarDistributions, BetaPdf) {
    double result = query_double(db_,
        "SELECT stat_beta_pdf(0.5, 2.0, 2.0)");
    EXPECT_NEAR(result, 1.5, 1e-4);
}

/// @brief Beta CDF: alpha=2, beta=2 at x=0.5 -> 0.5
TEST_F(ScalarDistributions, BetaCdf) {
    double result = query_double(db_,
        "SELECT stat_beta_cdf(0.5, 2.0, 2.0)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Beta Quantile: alpha=2, beta=2 at p=0.5 -> 0.5
TEST_F(ScalarDistributions, BetaQuantile) {
    double result = query_double(db_,
        "SELECT stat_beta_quantile(0.5, 2.0, 2.0)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Beta Rand: not NULL
TEST_F(ScalarDistributions, BetaRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_beta_rand(2.0, 2.0)"));
}

// ----- Log-normal distribution -----

/// @brief Log-normal PDF: mu=0, sigma=1 at x=1 -> about 0.398942
TEST_F(ScalarDistributions, LognormalPdf) {
    double result = query_double(db_,
        "SELECT stat_lognormal_pdf(1.0, 0.0, 1.0)");
    EXPECT_NEAR(result, 0.398942, 1e-4);
}

/// @brief Log-normal CDF: mu=0, sigma=1 at x=1 -> 0.5
TEST_F(ScalarDistributions, LognormalCdf) {
    double result = query_double(db_,
        "SELECT stat_lognormal_cdf(1.0, 0.0, 1.0)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Log-normal Quantile: mu=0, sigma=1 at p=0.5 -> 1.0
TEST_F(ScalarDistributions, LognormalQuantile) {
    double result = query_double(db_,
        "SELECT stat_lognormal_quantile(0.5, 0.0, 1.0)");
    EXPECT_NEAR(result, 1.0, 1e-4);
}

/// @brief Log-normal Rand: not NULL
TEST_F(ScalarDistributions, LognormalRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_lognormal_rand(0.0, 1.0)"));
}

// ----- Weibull distribution -----

/// @brief Weibull PDF: shape=1, scale=1 at x=1 -> exp(-1) ~ 0.367879
TEST_F(ScalarDistributions, WeibullPdf) {
    double result = query_double(db_,
        "SELECT stat_weibull_pdf(1.0, 1.0, 1.0)");
    EXPECT_NEAR(result, 0.367879, 1e-4);
}

/// @brief Weibull CDF: shape=1, scale=1 at x=1 -> 1-exp(-1) ~ 0.632121
TEST_F(ScalarDistributions, WeibullCdf) {
    double result = query_double(db_,
        "SELECT stat_weibull_cdf(1.0, 1.0, 1.0)");
    EXPECT_NEAR(result, 0.632121, 1e-4);
}

/// @brief Weibull Quantile: shape=1, scale=1 at p=0.5 -> ln(2) ~ 0.693147
TEST_F(ScalarDistributions, WeibullQuantile) {
    double result = query_double(db_,
        "SELECT stat_weibull_quantile(0.5, 1.0, 1.0)");
    EXPECT_NEAR(result, 0.693147, 1e-4);
}

/// @brief Weibull Rand: not NULL
TEST_F(ScalarDistributions, WeibullRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_weibull_rand(1.0, 1.0)"));
}

// =====================================================================
// Discrete distributions (28 functions)
// =====================================================================

// ----- Binomial distribution -----

/// @brief Binomial PMF: B(10,0.5) at k=3 -> about 0.117188
TEST_F(ScalarDistributions, BinomialPmf) {
    double result = query_double(db_,
        "SELECT stat_binomial_pmf(3, 10, 0.5)");
    EXPECT_NEAR(result, 0.117188, 1e-4);
}

/// @brief Binomial CDF: B(10,0.5) at k=5 -> about 0.623047
TEST_F(ScalarDistributions, BinomialCdf) {
    double result = query_double(db_,
        "SELECT stat_binomial_cdf(5, 10, 0.5)");
    EXPECT_NEAR(result, 0.623047, 1e-4);
}

/// @brief Binomial Quantile: B(10,0.5) at p=0.5 -> 5
TEST_F(ScalarDistributions, BinomialQuantile) {
    double result = query_double(db_,
        "SELECT stat_binomial_quantile(0.5, 10, 0.5)");
    EXPECT_NEAR(result, 5.0, 1e-4);
}

/// @brief Binomial Rand: not NULL
TEST_F(ScalarDistributions, BinomialRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_binomial_rand(10, 0.5)"));
}

// ----- Poisson distribution -----

/// @brief Poisson PMF: lambda=2 at k=3 -> about 0.180447
TEST_F(ScalarDistributions, PoissonPmf) {
    double result = query_double(db_,
        "SELECT stat_poisson_pmf(3, 2.0)");
    EXPECT_NEAR(result, 0.180447, 1e-4);
}

/// @brief Poisson CDF: lambda=2 at k=3 -> about 0.857123
TEST_F(ScalarDistributions, PoissonCdf) {
    double result = query_double(db_,
        "SELECT stat_poisson_cdf(3, 2.0)");
    EXPECT_NEAR(result, 0.857123, 1e-4);
}

/// @brief Poisson Quantile: lambda=2 at p=0.5 -> 2
TEST_F(ScalarDistributions, PoissonQuantile) {
    double result = query_double(db_,
        "SELECT stat_poisson_quantile(0.5, 2.0)");
    EXPECT_NEAR(result, 2.0, 1e-4);
}

/// @brief Poisson Rand: not NULL
TEST_F(ScalarDistributions, PoissonRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_poisson_rand(2.0)"));
}

// ----- Geometric distribution -----

/// @brief Geometric PMF: p=0.5 at k=3 -> 0.0625 (0-indexed: success after k failures)
TEST_F(ScalarDistributions, GeometricPmf) {
    double result = query_double(db_,
        "SELECT stat_geometric_pmf(3, 0.5)");
    EXPECT_NEAR(result, 0.0625, 1e-4);
}

/// @brief Geometric CDF: p=0.5 at k=3 -> 0.9375 (0-indexed)
TEST_F(ScalarDistributions, GeometricCdf) {
    double result = query_double(db_,
        "SELECT stat_geometric_cdf(3, 0.5)");
    EXPECT_NEAR(result, 0.9375, 1e-4);
}

/// @brief Geometric Quantile: p=0.5 at q=0.5 -> 0 (0-indexed)
TEST_F(ScalarDistributions, GeometricQuantile) {
    double result = query_double(db_,
        "SELECT stat_geometric_quantile(0.5, 0.5)");
    EXPECT_NEAR(result, 0.0, 1e-4);
}

/// @brief Geometric Rand: not NULL
TEST_F(ScalarDistributions, GeometricRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_geometric_rand(0.5)"));
}

// ----- Negative binomial distribution -----

/// @brief Negative binomial PMF: r=5, p=0.5 at k=3 -> about 0.136719
TEST_F(ScalarDistributions, NbinomPmf) {
    double result = query_double(db_,
        "SELECT stat_nbinom_pmf(3, 5, 0.5)");
    EXPECT_NEAR(result, 0.136719, 1e-4);
}

/// @brief Negative binomial CDF: r=5, p=0.5 at k=3 -> about 0.363281
TEST_F(ScalarDistributions, NbinomCdf) {
    double result = query_double(db_,
        "SELECT stat_nbinom_cdf(3, 5, 0.5)");
    EXPECT_NEAR(result, 0.363281, 1e-4);
}

/// @brief Negative binomial Quantile: r=5, p=0.5 at q=0.5 -> around 5
TEST_F(ScalarDistributions, NbinomQuantile) {
    double result = query_double(db_,
        "SELECT stat_nbinom_quantile(0.5, 5, 0.5)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GE(result, 3.0);
    EXPECT_LE(result, 7.0);
}

/// @brief Negative binomial Rand: not NULL
TEST_F(ScalarDistributions, NbinomRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_nbinom_rand(5, 0.5)"));
}

// ----- Hypergeometric distribution -----

/// @brief Hypergeometric PMF: N=20, K=10, n=5 at k=2 -> C(10,2)*C(10,3)/C(20,5)
TEST_F(ScalarDistributions, HypergeomPmf) {
    double result = query_double(db_,
        "SELECT stat_hypergeom_pmf(2, 20, 10, 5)");
    // C(10,2)*C(10,3)/C(20,5) = 45*120/15504 ≈ 0.347912
    EXPECT_NEAR(result, 0.3483, 1e-2);
}

/// @brief Hypergeometric CDF: N=20, K=10, n=5 at k=2
TEST_F(ScalarDistributions, HypergeomCdf) {
    double result = query_double(db_,
        "SELECT stat_hypergeom_cdf(2, 20, 10, 5)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
    EXPECT_LE(result, 1.0);
}

/// @brief Hypergeometric Quantile: N=20, K=10, n=5 at p=0.5 -> 2 or 3
TEST_F(ScalarDistributions, HypergeomQuantile) {
    double result = query_double(db_,
        "SELECT stat_hypergeom_quantile(0.5, 20, 10, 5)");
    EXPECT_TRUE(result == 2.0 || result == 3.0);
}

/// @brief Hypergeometric Rand: not NULL
TEST_F(ScalarDistributions, HypergeomRand) {
    EXPECT_FALSE(query_is_null(db_,
        "SELECT stat_hypergeom_rand(20, 10, 5)"));
}

// ----- Bernoulli distribution -----

/// @brief Bernoulli PMF: p=0.3 at k=1 -> 0.3
TEST_F(ScalarDistributions, BernoulliPmf) {
    double result = query_double(db_,
        "SELECT stat_bernoulli_pmf(1, 0.3)");
    EXPECT_NEAR(result, 0.3, 1e-4);
}

/// @brief Bernoulli CDF: p=0.3 at k=0 -> 0.7
TEST_F(ScalarDistributions, BernoulliCdf) {
    double result = query_double(db_,
        "SELECT stat_bernoulli_cdf(0, 0.3)");
    EXPECT_NEAR(result, 0.7, 1e-4);
}

/// @brief Bernoulli Quantile: p=0.3 at q=0.5 -> 0 (P(X<=0)=0.7 >= 0.5)
TEST_F(ScalarDistributions, BernoulliQuantile) {
    double result = query_double(db_,
        "SELECT stat_bernoulli_quantile(0.5, 0.3)");
    EXPECT_NEAR(result, 0.0, 1e-4);
}

/// @brief Bernoulli Rand: not NULL
TEST_F(ScalarDistributions, BernoulliRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_bernoulli_rand(0.3)"));
}

// ----- Discrete uniform distribution -----

/// @brief Discrete uniform PMF: [1,6] at k=3 -> 1/6 ~ 0.166667
TEST_F(ScalarDistributions, DuniformPmf) {
    double result = query_double(db_,
        "SELECT stat_duniform_pmf(3, 1, 6)");
    EXPECT_NEAR(result, 0.166667, 1e-4);
}

/// @brief Discrete uniform CDF: [1,6] at k=3 -> 0.5
TEST_F(ScalarDistributions, DuniformCdf) {
    double result = query_double(db_,
        "SELECT stat_duniform_cdf(3, 1, 6)");
    EXPECT_NEAR(result, 0.5, 1e-4);
}

/// @brief Discrete uniform Quantile: [1,6] at p=0.5 -> 3
TEST_F(ScalarDistributions, DuniformQuantile) {
    double result = query_double(db_,
        "SELECT stat_duniform_quantile(0.5, 1, 6)");
    EXPECT_NEAR(result, 3.0, 1e-4);
}

/// @brief Discrete uniform Rand: not NULL
TEST_F(ScalarDistributions, DuniformRand) {
    EXPECT_FALSE(query_is_null(db_, "SELECT stat_duniform_rand(1, 6)"));
}

// =====================================================================
// Combinatorics (3 functions)
// =====================================================================

/// @brief Binomial coefficient: C(10,3) = 120
TEST_F(ScalarDistributions, BinomialCoef) {
    double result = query_double(db_, "SELECT stat_binomial_coef(10, 3)");
    EXPECT_NEAR(result, 120.0, 1e-4);
}

/// @brief Log binomial coefficient: log(C(10,3)) = log(120) ~ 4.787492
TEST_F(ScalarDistributions, LogBinomialCoef) {
    double result = query_double(db_,
        "SELECT stat_log_binomial_coef(10, 3)");
    EXPECT_NEAR(result, 4.787492, 1e-4);
}

/// @brief Log factorial: log(5!) = log(120) ~ 4.787492
TEST_F(ScalarDistributions, LogFactorial) {
    double result = query_double(db_, "SELECT stat_log_factorial(5)");
    EXPECT_NEAR(result, 4.787492, 1e-4);
}

// =====================================================================
// Special functions (7 functions)
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

/// @brief Beta function: B(2,3) = 1/12 ~ 0.083333
TEST_F(ScalarDistributions, BetaFunc) {
    double result = query_double(db_, "SELECT stat_beta_func(2, 3)");
    EXPECT_NEAR(result, 0.083333, 1e-4);
}

/// @brief Log beta function: lbeta(2,3) = log(1/12) ~ -2.484907
TEST_F(ScalarDistributions, Lbeta) {
    double result = query_double(db_, "SELECT stat_lbeta(2, 3)");
    EXPECT_NEAR(result, -2.484907, 1e-4);
}

/// @brief Error function: erf(1.0) ~ 0.842701
TEST_F(ScalarDistributions, Erf) {
    double result = query_double(db_, "SELECT stat_erf(1.0)");
    EXPECT_NEAR(result, 0.842701, 1e-4);
}

/// @brief Complementary error function: erfc(1.0) ~ 0.157299
TEST_F(ScalarDistributions, Erfc) {
    double result = query_double(db_, "SELECT stat_erfc(1.0)");
    EXPECT_NEAR(result, 0.157299, 1e-4);
}

/// @brief Logarithmic mean: logarithmic_mean(2, 8) = (8-2)/ln(8/2) = 6/ln(4) ~ 4.328085
TEST_F(ScalarDistributions, LogarithmicMean) {
    double result = query_double(db_,
        "SELECT stat_logarithmic_mean(2, 8)");
    EXPECT_NEAR(result, 4.328085, 1e-4);
}

// =====================================================================
// Effect size conversion (8 functions)
// =====================================================================

/// @brief Hedges' J correction: df=10 -> a finite positive value
TEST_F(ScalarDistributions, HedgesJ) {
    double result = query_double(db_, "SELECT stat_hedges_j(10)");
    EXPECT_TRUE(std::isfinite(result));
    EXPECT_GT(result, 0.0);
}

/// @brief t to r conversion: t=2.0, df=10 -> about 0.534522
TEST_F(ScalarDistributions, TToR) {
    double result = query_double(db_, "SELECT stat_t_to_r(2.0, 10)");
    EXPECT_NEAR(result, 0.534522, 1e-4);
}

/// @brief Cohen's d to r conversion: d=0.5 -> about 0.242536
TEST_F(ScalarDistributions, DToR) {
    double result = query_double(db_, "SELECT stat_d_to_r(0.5)");
    EXPECT_NEAR(result, 0.242536, 1e-4);
}

/// @brief r to Cohen's d conversion: r=0.5 -> about 1.154701
TEST_F(ScalarDistributions, RToD) {
    double result = query_double(db_, "SELECT stat_r_to_d(0.5)");
    EXPECT_NEAR(result, 1.154701, 1e-4);
}

/// @brief Eta squared: ss_effect=100, ss_total=400 -> 0.25
TEST_F(ScalarDistributions, EtaSquaredEf) {
    double result = query_double(db_,
        "SELECT stat_eta_squared_ef(100, 400)");
    EXPECT_NEAR(result, 0.25, 1e-4);
}

/// @brief Partial eta squared: F=5.0, df1=2, df2=30 -> F*df1/(F*df1+df2) = 10/40 = 0.25
TEST_F(ScalarDistributions, PartialEtaSq) {
    double result = query_double(db_,
        "SELECT stat_partial_eta_sq(5.0, 2, 30)");
    EXPECT_NEAR(result, 0.25, 1e-4);
}

/// @brief Omega squared: the result is finite
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
// Effect size interpretation (3 functions) - return TEXT
// =====================================================================

/// @brief Interpreting Cohen's d: d=0.3 -> "small"
TEST_F(ScalarDistributions, InterpretD) {
    std::string result = query_text(db_, "SELECT stat_interpret_d(0.3)");
    EXPECT_EQ(result, "small");
}

/// @brief Interpreting the correlation r: r=0.1 -> "small"
TEST_F(ScalarDistributions, InterpretR) {
    std::string result = query_text(db_, "SELECT stat_interpret_r(0.1)");
    EXPECT_EQ(result, "small");
}

/// @brief Interpreting eta squared: eta2=0.01 -> "small"
TEST_F(ScalarDistributions, InterpretEta2) {
    std::string result = query_text(db_,
        "SELECT stat_interpret_eta2(0.01)");
    EXPECT_EQ(result, "small");
}

// =====================================================================
// Power analysis (6 functions)
// =====================================================================

/// @brief One-sample t-test power: d=0.5, n=30, alpha=0.05 -> within 0.0-1.0
TEST_F(ScalarDistributions, PowerT1) {
    double result = query_double(db_,
        "SELECT stat_power_t1(0.5, 30, 0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
}

/// @brief One-sample t-test sample size: d=0.5, power=0.8, alpha=0.05 -> a positive integer
TEST_F(ScalarDistributions, NT1) {
    double result = query_double(db_,
        "SELECT stat_n_t1(0.5, 0.8, 0.05)");
    EXPECT_GT(result, 0.0);
}

/// @brief Two-sample t-test power: d=0.5, n1=30, n2=30, alpha=0.05 -> within 0.0-1.0
TEST_F(ScalarDistributions, PowerT2) {
    double result = query_double(db_,
        "SELECT stat_power_t2(0.5, 30, 30, 0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
}

/// @brief Two-sample t-test sample size: d=0.5, power=0.8, alpha=0.05 -> a positive value
TEST_F(ScalarDistributions, NT2) {
    double result = query_double(db_,
        "SELECT stat_n_t2(0.5, 0.8, 0.05)");
    EXPECT_GT(result, 0.0);
}

/// @brief Proportion test power: p1=0.5, p2=0.3, n=100, alpha=0.05 -> within 0.0-1.0
TEST_F(ScalarDistributions, PowerProp) {
    double result = query_double(db_,
        "SELECT stat_power_prop(0.5, 0.3, 100, 0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_LT(result, 1.0);
}

/// @brief Proportion test sample size: p1=0.5, p2=0.3, power=0.8, alpha=0.05 -> a positive value
TEST_F(ScalarDistributions, NProp) {
    double result = query_double(db_,
        "SELECT stat_n_prop(0.5, 0.3, 0.8, 0.05)");
    EXPECT_GT(result, 0.0);
}

// =====================================================================
// MoE and sample size (4 functions)
// =====================================================================

/// @brief Margin of error for a proportion: x=50, n=100 -> a positive value (about 0.098)
TEST_F(ScalarDistributions, MoeProp) {
    double result = query_double(db_, "SELECT stat_moe_prop(50, 100)");
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 0.098, 1e-2);
}

/// @brief Worst-case MoE for a proportion: n=100 -> a positive value (about 0.098)
TEST_F(ScalarDistributions, MoePropWorst) {
    double result = query_double(db_,
        "SELECT stat_moe_prop_worst(100)");
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 0.098, 1e-2);
}

/// @brief Sample size for a proportion MoE: moe=0.05 -> a positive value (about 385)
TEST_F(ScalarDistributions, NMoeProp) {
    double result = query_double(db_, "SELECT stat_n_moe_prop(0.05)");
    EXPECT_GT(result, 0.0);
    EXPECT_NEAR(result, 385.0, 5.0);
}

/// @brief Sample size for a mean MoE: moe=1.0, sd=5.0 -> a positive value
TEST_F(ScalarDistributions, NMoeMean) {
    double result = query_double(db_,
        "SELECT stat_n_moe_mean(1.0, 5.0)");
    EXPECT_GT(result, 0.0);
}

// =====================================================================
// Quantiles of distributions with unbounded support: p = 1.0 has no finite value
// =====================================================================

/// @brief Boundary: Poisson at p=1.0 -> NULL (no finite quantile)
TEST_F(ScalarDistributions, PoissonQuantileAtOneIsNull) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_poisson_quantile(1.0, 2.5)"));
}

/// @brief Boundary: geometric at p=1.0 -> NULL (no finite quantile)
TEST_F(ScalarDistributions, GeometricQuantileAtOneIsNull) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_geometric_quantile(1.0, 0.3)"));
}

/// @brief Boundary: negative binomial at p=1.0 -> NULL (no finite quantile)
TEST_F(ScalarDistributions, NbinomQuantileAtOneIsNull) {
    EXPECT_TRUE(query_is_null(db_, "SELECT stat_nbinom_quantile(1.0, 5, 0.5)"));
}

/// @brief Normal case: p<1 still returns a finite quantile (not caught by the NULL rule)
TEST_F(ScalarDistributions, UnboundedQuantilesBelowOneAreFinite) {
    EXPECT_EQ(query_int(db_, "SELECT stat_poisson_quantile(0.99, 2.5)"), 7);
    EXPECT_EQ(query_int(db_, "SELECT stat_poisson_quantile(0.5, 2.5)"), 2);
    EXPECT_EQ(query_int(db_, "SELECT stat_geometric_quantile(0.5, 0.3)"), 1);
    EXPECT_EQ(query_int(db_, "SELECT stat_nbinom_quantile(0.5, 5, 0.5)"), 4);
}

/// @brief Boundary: a distribution with bounded support still returns its maximum at p=1.0
TEST_F(ScalarDistributions, BoundedQuantileAtOneIsNotNull) {
    EXPECT_EQ(query_int(db_, "SELECT stat_binomial_quantile(1.0, 10, 0.5)"), 10);
}

// =====================================================================
// Exception boundary: an invalid argument becomes a SQL error, not a crash
//
// statcpp throws std::invalid_argument when an argument is out of range.
// SQLite invokes callbacks across the C ABI, so an uncaught exception reaches
// std::terminate and takes the test process down with it.
// That the tests below run at all and return a value is itself the proof the guard works.
// =====================================================================

/// @brief Error case: an out-of-range scalar argument -> SQL error (no abort)
TEST_F(ScalarDistributions, InvalidScalarArgumentRaisesSqlError) {
    std::string msg = query_error(db_, "SELECT stat_poisson_quantile(1.5, 2.5)");
    EXPECT_NE(msg.find("p must be in"), std::string::npos) << "actual: " << msg;
}

/// @brief Error case: the same connection still runs queries after an out-of-range argument
TEST_F(ScalarDistributions, ConnectionSurvivesInvalidArgument) {
    query_error(db_, "SELECT stat_poisson_quantile(-1.0, 2.5)");
    EXPECT_NEAR(query_double(db_, "SELECT stat_normal_cdf(0.0)"), 0.5, 1e-12);
}
