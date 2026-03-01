/*
 * SQLite3 Loadable Extension — Statistical Functions (249 functions)
 *
 * Basic aggregates:         24 single-column aggregate functions (Template A)
 * Parameterized aggregates: 20 parameterized aggregate functions (Template B)
 * Two-column aggregates:    27 two-column aggregate functions (Template C)
 * Window functions:         23 window functions (Template D)
 * Complex aggregates:       32 complex aggregate functions (Template E — SingleColumnAggregateText,
 *                           TwoColumnAggregateText, and direct registration)
 * Scalar — tests/helpers:   40 scalar functions (Template F — ScalarFunction)
 * Scalar — distributions:   83 scalar functions (Template F — ScalarFunction)
 */

#include <sqlite3ext.h>
#include <vector>
#include <array>
#include <deque>
#include <map>
#include <string>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstring>
#include <limits>

#include <statcpp/statcpp.hpp>

SQLITE_EXTENSION_INIT1

// ===========================================================================
// Template A — SingleColumnAggregate
//
// Collects double values via xStep, computes a statistic in xFinal.
// The State is heap-allocated because sqlite3_aggregate_context only provides
// a flat byte buffer — we store a pointer to the heap object there.
//
// Func signature: double(const std::vector<double>&)
// ===========================================================================

struct SingleColumnState {
    std::vector<double> values;
};

template <double (*Func)(const std::vector<double>&)>
class SingleColumnAggregate {
public:
    static void xStep(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
        if (sqlite3_value_type(argv[0]) == SQLITE_NULL) return;
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        state->values.push_back(sqlite3_value_double(argv[0]));
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<SingleColumnState**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp || (*pp)->values.empty()) {
            sqlite3_result_null(ctx);
            cleanupState(pp);
            return;
        }
        double result = Func((*pp)->values);
        cleanupState(pp);
        if (std::isnan(result) || std::isinf(result)) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_double(ctx, result);
        }
    }

    static int register_func(sqlite3* db, const char* name) {
        return sqlite3_create_function_v2(
            db, name, 1,
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr,    // pApp
            nullptr,    // xFunc (scalar) — not used for aggregate
            xStep,
            xFinal,
            nullptr     // xDestroy
        );
    }

private:
    static SingleColumnState* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<SingleColumnState**>(
            sqlite3_aggregate_context(ctx, sizeof(SingleColumnState*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) SingleColumnState();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }

    static void cleanupState(SingleColumnState** pp) {
        if (pp && *pp) {
            delete *pp;
            *pp = nullptr;
        }
    }
};

// ===========================================================================
// Wrapper functions: adapt statcpp API to (const vector<double>&) -> double
//
// statcpp functions use iterator pairs. Some (median, iqr) require sorted
// input, so we sort a local copy where needed.
// ===========================================================================

// --- Basic statistics ---

static double calc_mean(const std::vector<double>& v) {
    return statcpp::mean(v.begin(), v.end());
}

static double calc_median(const std::vector<double>& v) {
    std::vector<double> sorted(v);
    std::sort(sorted.begin(), sorted.end());
    return statcpp::median(sorted.begin(), sorted.end());
}

static double calc_mode(const std::vector<double>& v) {
    return statcpp::mode(v.begin(), v.end());
}

static double calc_geometric_mean(const std::vector<double>& v) {
    return statcpp::geometric_mean(v.begin(), v.end());
}

static double calc_harmonic_mean(const std::vector<double>& v) {
    return statcpp::harmonic_mean(v.begin(), v.end());
}

// --- Dispersion / spread ---

static double calc_range(const std::vector<double>& v) {
    return statcpp::range(v.begin(), v.end());
}

static double calc_var(const std::vector<double>& v) {
    // var with ddof=0 (population variance) — same as population_variance
    // Note: stat_var uses ddof=0 by default as per statcpp convention
    return statcpp::var(v.begin(), v.end(), static_cast<std::size_t>(0));
}

static double calc_population_variance(const std::vector<double>& v) {
    return statcpp::population_variance(v.begin(), v.end());
}

static double calc_sample_variance(const std::vector<double>& v) {
    if (v.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::sample_variance(v.begin(), v.end());
}

static double calc_stdev(const std::vector<double>& v) {
    return statcpp::stdev(v.begin(), v.end(), static_cast<std::size_t>(0));
}

static double calc_population_stddev(const std::vector<double>& v) {
    return statcpp::population_stddev(v.begin(), v.end());
}

static double calc_sample_stddev(const std::vector<double>& v) {
    if (v.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::sample_stddev(v.begin(), v.end());
}

static double calc_cv(const std::vector<double>& v) {
    return statcpp::coefficient_of_variation(v.begin(), v.end());
}

static double calc_iqr(const std::vector<double>& v) {
    std::vector<double> sorted(v);
    std::sort(sorted.begin(), sorted.end());
    return statcpp::iqr(sorted.begin(), sorted.end());
}

static double calc_mad_mean(const std::vector<double>& v) {
    return statcpp::mean_absolute_deviation(v.begin(), v.end());
}

static double calc_geometric_stddev(const std::vector<double>& v) {
    return statcpp::geometric_stddev(v.begin(), v.end());
}

// --- Shape of distribution ---

static double calc_population_skewness(const std::vector<double>& v) {
    if (v.size() < 3) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::population_skewness(v.begin(), v.end());
}

static double calc_skewness(const std::vector<double>& v) {
    if (v.size() < 3) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::sample_skewness(v.begin(), v.end());
}

static double calc_population_kurtosis(const std::vector<double>& v) {
    if (v.size() < 4) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::population_kurtosis(v.begin(), v.end());
}

static double calc_kurtosis(const std::vector<double>& v) {
    if (v.size() < 4) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::sample_kurtosis(v.begin(), v.end());
}

// --- Estimation ---

static double calc_se(const std::vector<double>& v) {
    if (v.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::standard_error(v.begin(), v.end());
}

// --- Robust statistics ---

static double calc_mad(const std::vector<double>& v) {
    return statcpp::mad(v.begin(), v.end());
}

static double calc_mad_scaled(const std::vector<double>& v) {
    return statcpp::mad_scaled(v.begin(), v.end());
}

static double calc_hodges_lehmann(const std::vector<double>& v) {
    return statcpp::hodges_lehmann(v.begin(), v.end());
}

// ===========================================================================
// Template B — SingleColumnParamAggregate
//
// Like Template A but with additional parameters (0–2) captured from the
// first row's extra SQL arguments.  Two variants:
//   - Returning double (same as A but with params)
//   - Returning text (JSON string) for struct results
//
// Func signature: double/std::string (const vector<double>&, const array<double,N>&)
// ===========================================================================

template <std::size_t NParams>
struct SingleColumnParamState {
    std::vector<double> values;
    std::array<double, NParams> params{};
    bool params_set = false;
};

// --- B1: returns double ---

template <std::size_t NParams,
          double (*Func)(const std::vector<double>&,
                         const std::array<double, NParams>&)>
class SingleColumnParamAggregate {
public:
    using State = SingleColumnParamState<NParams>;

    static void xStep(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
        if (sqlite3_value_type(argv[0]) == SQLITE_NULL) return;
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        state->values.push_back(sqlite3_value_double(argv[0]));
        if (!state->params_set) {
            for (std::size_t i = 0; i < NParams; ++i) {
                if (static_cast<int>(i + 1) < argc &&
                    sqlite3_value_type(argv[i + 1]) != SQLITE_NULL) {
                    state->params[i] = sqlite3_value_double(argv[i + 1]);
                }
            }
            state->params_set = true;
        }
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<State**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp || (*pp)->values.empty()) {
            sqlite3_result_null(ctx);
            cleanupState(pp);
            return;
        }
        double result = Func((*pp)->values, (*pp)->params);
        cleanupState(pp);
        if (std::isnan(result) || std::isinf(result)) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_double(ctx, result);
        }
    }

    static int register_func(sqlite3* db, const char* name) {
        return sqlite3_create_function_v2(
            db, name, 1 + static_cast<int>(NParams),
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, nullptr, xStep, xFinal, nullptr);
    }

private:
    static State* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<State**>(
            sqlite3_aggregate_context(ctx, sizeof(State*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) State();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }

    static void cleanupState(State** pp) {
        if (pp && *pp) {
            delete *pp;
            *pp = nullptr;
        }
    }
};

// --- B2: returns JSON text ---

template <std::size_t NParams,
          std::string (*Func)(const std::vector<double>&,
                              const std::array<double, NParams>&)>
class SingleColumnParamAggregateText {
public:
    using State = SingleColumnParamState<NParams>;

    static void xStep(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
        if (sqlite3_value_type(argv[0]) == SQLITE_NULL) return;
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        state->values.push_back(sqlite3_value_double(argv[0]));
        if (!state->params_set) {
            for (std::size_t i = 0; i < NParams; ++i) {
                if (static_cast<int>(i + 1) < argc &&
                    sqlite3_value_type(argv[i + 1]) != SQLITE_NULL) {
                    state->params[i] = sqlite3_value_double(argv[i + 1]);
                }
            }
            state->params_set = true;
        }
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<State**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp || (*pp)->values.empty()) {
            sqlite3_result_null(ctx);
            cleanupState(pp);
            return;
        }
        std::string result = Func((*pp)->values, (*pp)->params);
        cleanupState(pp);
        if (result.empty()) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_text(ctx, result.c_str(),
                                static_cast<int>(result.size()),
                                SQLITE_TRANSIENT);
        }
    }

    static int register_func(sqlite3* db, const char* name) {
        return sqlite3_create_function_v2(
            db, name, 1 + static_cast<int>(NParams),
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, nullptr, xStep, xFinal, nullptr);
    }

private:
    static State* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<State**>(
            sqlite3_aggregate_context(ctx, sizeof(State*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) State();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }

    static void cleanupState(State** pp) {
        if (pp && *pp) {
            delete *pp;
            *pp = nullptr;
        }
    }
};

// ===========================================================================
// JSON formatting helpers (minimal, no external dependency)
// ===========================================================================

static std::string json_double(double v) {
    if (std::isnan(v) || std::isinf(v)) return "null";
    char buf[64];
    std::snprintf(buf, sizeof(buf), "%.15g", v);
    return buf;
}

static std::string json_test_result(const statcpp::test_result& r) {
    return "{\"statistic\":" + json_double(r.statistic)
         + ",\"p_value\":" + json_double(r.p_value)
         + ",\"df\":" + json_double(r.df) + "}";
}

static std::string json_ci(const statcpp::confidence_interval& ci) {
    return "{\"lower\":" + json_double(ci.lower)
         + ",\"upper\":" + json_double(ci.upper)
         + ",\"point_estimate\":" + json_double(ci.point_estimate)
         + ",\"confidence_level\":" + json_double(ci.confidence_level) + "}";
}

static std::string json_bootstrap(const statcpp::bootstrap_result& br) {
    return "{\"estimate\":" + json_double(br.estimate)
         + ",\"standard_error\":" + json_double(br.standard_error)
         + ",\"ci_lower\":" + json_double(br.ci_lower)
         + ",\"ci_upper\":" + json_double(br.ci_upper)
         + ",\"bias\":" + json_double(br.bias) + "}";
}

// ===========================================================================
// Parameterized aggregate wrapper functions
// ===========================================================================

// --- Basic statistics (parameterized) ---

static double calc_trimmed_mean(const std::vector<double>& v,
                                const std::array<double, 1>& p) {
    std::vector<double> sorted(v);
    std::sort(sorted.begin(), sorted.end());
    return statcpp::trimmed_mean(sorted.begin(), sorted.end(), p[0]);
}

// --- Order statistics (parameterized) ---

static std::string calc_quartile(const std::vector<double>& v,
                                 const std::array<double, 0>&) {
    std::vector<double> sorted(v);
    std::sort(sorted.begin(), sorted.end());
    auto qr = statcpp::quartiles(sorted.begin(), sorted.end());
    return "{\"q1\":" + json_double(qr.q1)
         + ",\"q2\":" + json_double(qr.q2)
         + ",\"q3\":" + json_double(qr.q3) + "}";
}

static double calc_percentile(const std::vector<double>& v,
                               const std::array<double, 1>& p) {
    std::vector<double> sorted(v);
    std::sort(sorted.begin(), sorted.end());
    return statcpp::percentile(sorted.begin(), sorted.end(), p[0]);
}

// --- Parametric tests ---

static std::string calc_z_test(const std::vector<double>& v,
                                const std::array<double, 2>& p) {
    auto r = statcpp::z_test(v.begin(), v.end(), p[0], p[1]);
    return json_test_result(r);
}

static std::string calc_t_test(const std::vector<double>& v,
                                const std::array<double, 1>& p) {
    if (v.size() < 2) return "";
    auto r = statcpp::t_test(v.begin(), v.end(), p[0]);
    return json_test_result(r);
}

static std::string calc_chisq_gof_uniform(const std::vector<double>& v,
                                           const std::array<double, 0>&) {
    if (v.size() < 2) return "";
    auto r = statcpp::chisq_test_gof_uniform(v.begin(), v.end());
    return json_test_result(r);
}

// --- Nonparametric tests ---

static std::string calc_shapiro_wilk(const std::vector<double>& v,
                                      const std::array<double, 0>&) {
    if (v.size() < 3) return "";
    auto r = statcpp::shapiro_wilk_test(v.begin(), v.end());
    return json_test_result(r);
}

static std::string calc_ks_test(const std::vector<double>& v,
                                 const std::array<double, 0>&) {
    if (v.size() < 2) return "";
    auto r = statcpp::lilliefors_test(v.begin(), v.end());
    return json_test_result(r);
}

static std::string calc_wilcoxon(const std::vector<double>& v,
                                  const std::array<double, 1>& p) {
    if (v.size() < 2) return "";
    auto r = statcpp::wilcoxon_signed_rank_test(v.begin(), v.end(), p[0]);
    return json_test_result(r);
}

// --- Estimation ---

static std::string calc_ci_mean(const std::vector<double>& v,
                                 const std::array<double, 1>& p) {
    if (v.size() < 2) return "";
    auto ci = statcpp::ci_mean(v.begin(), v.end(), p[0]);
    return json_ci(ci);
}

static std::string calc_ci_mean_z(const std::vector<double>& v,
                                   const std::array<double, 2>& p) {
    if (v.size() < 2) return "";
    auto ci = statcpp::ci_mean_z(v.begin(), v.end(), p[0], p[1]);
    return json_ci(ci);
}

static std::string calc_ci_var(const std::vector<double>& v,
                                const std::array<double, 1>& p) {
    if (v.size() < 2) return "";
    auto ci = statcpp::ci_variance(v.begin(), v.end(), p[0]);
    return json_ci(ci);
}

static double calc_moe_mean(const std::vector<double>& v,
                             const std::array<double, 1>& p) {
    if (v.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::margin_of_error_mean(v.begin(), v.end(), p[0]);
}

// --- Effect size ---

static double calc_cohens_d(const std::vector<double>& v,
                             const std::array<double, 1>& p) {
    if (v.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::cohens_d(v.begin(), v.end(), p[0]);
}

static double calc_hedges_g(const std::vector<double>& v,
                             const std::array<double, 1>& p) {
    if (v.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::hedges_g(v.begin(), v.end(), p[0]);
}

// --- Time series ---

static double calc_acf_lag(const std::vector<double>& v,
                            const std::array<double, 1>& p) {
    auto lag = static_cast<std::size_t>(p[0]);
    if (lag >= v.size()) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::autocorrelation(v.begin(), v.end(), lag);
}

// --- Robust statistics (parameterized) ---

static double calc_biweight_midvar(const std::vector<double>& v,
                                    const std::array<double, 1>& p) {
    return statcpp::biweight_midvariance(v.begin(), v.end(), p[0]);
}

// --- Resampling ---

static std::string calc_bootstrap_mean(const std::vector<double>& v,
                                        const std::array<double, 1>& p) {
    auto n = static_cast<std::size_t>(p[0]);
    if (n < 2) n = 1000;
    auto r = statcpp::bootstrap_mean(v.begin(), v.end(), n);
    return json_bootstrap(r);
}

static std::string calc_bootstrap_median(const std::vector<double>& v,
                                          const std::array<double, 1>& p) {
    auto n = static_cast<std::size_t>(p[0]);
    if (n < 2) n = 1000;
    auto r = statcpp::bootstrap_median(v.begin(), v.end(), n);
    return json_bootstrap(r);
}

static std::string calc_bootstrap_stddev(const std::vector<double>& v,
                                          const std::array<double, 1>& p) {
    if (v.size() < 2) return "";
    auto n = static_cast<std::size_t>(p[0]);
    if (n < 2) n = 1000;
    auto r = statcpp::bootstrap_stddev(v.begin(), v.end(), n);
    return json_bootstrap(r);
}

// ===========================================================================
// Template C — TwoColumnAggregate
//
// Collects (x, y) pairs via xStep, computes a statistic in xFinal.
// If either x or y is NULL for a row, that row is skipped.
//
// Func signature: double(const std::vector<double>&, const std::vector<double>&)
// ===========================================================================

struct TwoColumnState {
    std::vector<double> xs;
    std::vector<double> ys;
};

template <double (*Func)(const std::vector<double>&, const std::vector<double>&)>
class TwoColumnAggregate {
public:
    static void xStep(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
        if (sqlite3_value_type(argv[0]) == SQLITE_NULL ||
            sqlite3_value_type(argv[1]) == SQLITE_NULL) return;
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        state->xs.push_back(sqlite3_value_double(argv[0]));
        state->ys.push_back(sqlite3_value_double(argv[1]));
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<TwoColumnState**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp || (*pp)->xs.empty()) {
            sqlite3_result_null(ctx);
            cleanupState(pp);
            return;
        }
        double result = Func((*pp)->xs, (*pp)->ys);
        cleanupState(pp);
        if (std::isnan(result) || std::isinf(result)) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_double(ctx, result);
        }
    }

    static int register_func(sqlite3* db, const char* name) {
        return sqlite3_create_function_v2(
            db, name, 2,
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, nullptr, xStep, xFinal, nullptr);
    }

private:
    static TwoColumnState* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<TwoColumnState**>(
            sqlite3_aggregate_context(ctx, sizeof(TwoColumnState*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) TwoColumnState();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }

    static void cleanupState(TwoColumnState** pp) {
        if (pp && *pp) {
            delete *pp;
            *pp = nullptr;
        }
    }
};

// --- C variant: returns JSON text ---

template <std::string (*Func)(const std::vector<double>&, const std::vector<double>&)>
class TwoColumnAggregateText {
public:
    static void xStep(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
        if (sqlite3_value_type(argv[0]) == SQLITE_NULL ||
            sqlite3_value_type(argv[1]) == SQLITE_NULL) return;
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        state->xs.push_back(sqlite3_value_double(argv[0]));
        state->ys.push_back(sqlite3_value_double(argv[1]));
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<TwoColumnState**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp || (*pp)->xs.empty()) {
            sqlite3_result_null(ctx);
            cleanupState(pp);
            return;
        }
        std::string result = Func((*pp)->xs, (*pp)->ys);
        cleanupState(pp);
        if (result.empty()) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_text(ctx, result.c_str(),
                                static_cast<int>(result.size()),
                                SQLITE_TRANSIENT);
        }
    }

    static int register_func(sqlite3* db, const char* name) {
        return sqlite3_create_function_v2(
            db, name, 2,
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, nullptr, xStep, xFinal, nullptr);
    }

private:
    static TwoColumnState* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<TwoColumnState**>(
            sqlite3_aggregate_context(ctx, sizeof(TwoColumnState*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) TwoColumnState();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }

    static void cleanupState(TwoColumnState** pp) {
        if (pp && *pp) {
            delete *pp;
            *pp = nullptr;
        }
    }
};

// --- C variant: with extra parameter(s), returns double ---

template <std::size_t NParams>
struct TwoColumnParamState {
    std::vector<double> xs;
    std::vector<double> ys;
    std::array<double, NParams> params{};
    bool params_set = false;
};

template <std::size_t NParams,
          double (*Func)(const std::vector<double>&,
                         const std::vector<double>&,
                         const std::array<double, NParams>&)>
class TwoColumnParamAggregate {
public:
    using State = TwoColumnParamState<NParams>;

    static void xStep(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
        if (sqlite3_value_type(argv[0]) == SQLITE_NULL ||
            sqlite3_value_type(argv[1]) == SQLITE_NULL) return;
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        state->xs.push_back(sqlite3_value_double(argv[0]));
        state->ys.push_back(sqlite3_value_double(argv[1]));
        if (!state->params_set) {
            for (std::size_t i = 0; i < NParams; ++i) {
                if (static_cast<int>(i + 2) < argc &&
                    sqlite3_value_type(argv[i + 2]) != SQLITE_NULL) {
                    state->params[i] = sqlite3_value_double(argv[i + 2]);
                }
            }
            state->params_set = true;
        }
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<State**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp || (*pp)->xs.empty()) {
            sqlite3_result_null(ctx);
            cleanupState(pp);
            return;
        }
        double result = Func((*pp)->xs, (*pp)->ys, (*pp)->params);
        cleanupState(pp);
        if (std::isnan(result) || std::isinf(result)) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_double(ctx, result);
        }
    }

    static int register_func(sqlite3* db, const char* name) {
        return sqlite3_create_function_v2(
            db, name, 2 + static_cast<int>(NParams),
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, nullptr, xStep, xFinal, nullptr);
    }

private:
    static State* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<State**>(
            sqlite3_aggregate_context(ctx, sizeof(State*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) State();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }

    static void cleanupState(State** pp) {
        if (pp && *pp) {
            delete *pp;
            *pp = nullptr;
        }
    }
};

// ===========================================================================
// Template D — Window Functions
//
// D1: FullScanWindowFunction — collects all rows, then returns per-row results
//     Used for: rolling_*, rank, fillna_*, outliers, diff, etc.
//     All window functions use this approach (collect all data first,
//     then compute the result vector, then return one element per row).
//
// Implementation note: SQLite3 window function API (xStep/xInverse/xValue/
// xFinal) is complex. Instead, we use a simpler approach:
// - Collect all data in xStep
// - In xFinal, compute all results and return JSON array
// This is then used via a subquery pattern. However, for true window function
// behavior (returning one value per row), we implement proper window functions.
// ===========================================================================

struct WindowState {
    std::vector<double> values;
    std::vector<bool> nulls;
    std::vector<double> results;
    int param = 0;
    double dparam = 0.0;
    bool param_set = false;
    std::size_t result_idx = 0;
    bool computed = false;
};

// Helper: compute results from collected data and return per-row
// Func: (const vector<double>& values, const vector<bool>& nulls, int param) -> vector<double>
template <std::vector<double> (*Func)(const std::vector<double>&,
                                       const std::vector<bool>&,
                                       int)>
class FullScanWindowFunction {
public:
    static void xStep(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        bool is_null = (sqlite3_value_type(argv[0]) == SQLITE_NULL);
        state->nulls.push_back(is_null);
        state->values.push_back(is_null ? std::numeric_limits<double>::quiet_NaN()
                                        : sqlite3_value_double(argv[0]));
        if (!state->param_set && argc > 1 &&
            sqlite3_value_type(argv[1]) != SQLITE_NULL) {
            state->param = sqlite3_value_int(argv[1]);
            state->dparam = sqlite3_value_double(argv[1]);
            state->param_set = true;
        }
    }

    static void xInverse(sqlite3_context* ctx, int /*argc*/, sqlite3_value** /*argv*/) {
        // No-op: we need all data, so we don't remove anything
    }

    static void xValue(sqlite3_context* ctx) {
        auto* state = getOrCreateState(ctx);
        if (!state) { sqlite3_result_null(ctx); return; }
        if (!state->computed) {
            state->results = Func(state->values, state->nulls, state->param);
            state->computed = true;
            state->result_idx = 0;
        }
        if (state->result_idx < state->results.size()) {
            double r = state->results[state->result_idx];
            state->result_idx++;
            if (std::isnan(r)) {
                sqlite3_result_null(ctx);
            } else {
                sqlite3_result_double(ctx, r);
            }
        } else {
            sqlite3_result_null(ctx);
        }
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<WindowState**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp) {
            sqlite3_result_null(ctx);
            return;
        }
        if (!(*pp)->computed) {
            (*pp)->results = Func((*pp)->values, (*pp)->nulls, (*pp)->param);
            (*pp)->computed = true;
        }
        if ((*pp)->result_idx < (*pp)->results.size()) {
            double r = (*pp)->results[(*pp)->result_idx];
            if (std::isnan(r)) {
                sqlite3_result_null(ctx);
            } else {
                sqlite3_result_double(ctx, r);
            }
        } else {
            sqlite3_result_null(ctx);
        }
        delete *pp;
        *pp = nullptr;
    }

    // argc=1: no parameter (e.g. stat_rank, stat_fillna_mean)
    static int register_func_1(sqlite3* db, const char* name) {
        return sqlite3_create_window_function(
            db, name, 1,
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, xStep, xFinal, xValue, xInverse, nullptr);
    }

    // argc=2: with parameter (e.g. stat_rolling_mean(col, window_size))
    static int register_func_2(sqlite3* db, const char* name) {
        return sqlite3_create_window_function(
            db, name, 2,
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, xStep, xFinal, xValue, xInverse, nullptr);
    }

private:
    static WindowState* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<WindowState**>(
            sqlite3_aggregate_context(ctx, sizeof(WindowState*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) WindowState();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }
};

// ===========================================================================
// Two-column aggregate wrapper functions
// ===========================================================================

// --- Correlation / Covariance ---

static double calc_population_covariance(const std::vector<double>& x,
                                          const std::vector<double>& y) {
    if (x.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::population_covariance(x.begin(), x.end(), y.begin(), y.end());
}

static double calc_covariance(const std::vector<double>& x,
                               const std::vector<double>& y) {
    if (x.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::covariance(x.begin(), x.end(), y.begin(), y.end());
}

static double calc_pearson_r(const std::vector<double>& x,
                              const std::vector<double>& y) {
    if (x.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::pearson_correlation(x.begin(), x.end(), y.begin(), y.end());
}

static double calc_spearman_r(const std::vector<double>& x,
                               const std::vector<double>& y) {
    if (x.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::spearman_correlation(x.begin(), x.end(), y.begin(), y.end());
}

static double calc_kendall_tau(const std::vector<double>& x,
                                const std::vector<double>& y) {
    if (x.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::kendall_tau(x.begin(), x.end(), y.begin(), y.end());
}

static double calc_weighted_covariance(const std::vector<double>& values,
                                        const std::vector<double>& weights) {
    if (values.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    // weighted_covariance(val_first, val_last, val2_first, val2_last, weight_first)
    // Here col_x = col_y = values, weights = weights (self-covariance = weighted variance)
    // Actually, weighted_covariance needs two data columns and weights.
    // In our 2-column interface, x=values, y=weights is ambiguous.
    // We treat this as: covariance of x with itself, weighted by y — i.e., weighted variance.
    // But the design says stat_weighted_covariance(col_x, col_y) computes covariance
    // with values=col_x, weights=col_y... Let's follow the statcpp API:
    // weighted_covariance(first1, last1, first2, last2, weight_first) needs 3 series.
    // With 2 columns, we compute weighted self-covariance: cov(x, x, w=y)
    // Actually, re-reading the doc: stat_weighted_covariance(values, weights)
    // computes the weighted covariance. This needs 3 columns conceptually,
    // but with 2 columns we treat it as weighted variance of x using weights y.
    return statcpp::weighted_covariance(values.begin(), values.end(),
                                         values.begin(), values.end(),
                                         weights.begin());
}

// --- Weighted statistics ---

static double calc_weighted_mean(const std::vector<double>& values,
                                  const std::vector<double>& weights) {
    return statcpp::weighted_mean(values.begin(), values.end(), weights.begin());
}

static double calc_weighted_harmonic_mean(const std::vector<double>& values,
                                           const std::vector<double>& weights) {
    return statcpp::weighted_harmonic_mean(values.begin(), values.end(), weights.begin());
}

static double calc_weighted_variance(const std::vector<double>& values,
                                      const std::vector<double>& weights) {
    return statcpp::weighted_variance(values.begin(), values.end(), weights.begin());
}

static double calc_weighted_stddev(const std::vector<double>& values,
                                    const std::vector<double>& weights) {
    return statcpp::weighted_stddev(values.begin(), values.end(), weights.begin());
}

static double calc_weighted_median(const std::vector<double>& values,
                                    const std::vector<double>& weights) {
    return statcpp::weighted_median(values.begin(), values.end(), weights.begin());
}

static double calc_weighted_percentile(const std::vector<double>& values,
                                        const std::vector<double>& weights,
                                        const std::array<double, 1>& p) {
    return statcpp::weighted_percentile(values.begin(), values.end(),
                                         weights.begin(), p[0]);
}

// --- Linear regression ---

static std::string json_regression(const statcpp::simple_regression_result& r) {
    return "{\"intercept\":" + json_double(r.intercept)
         + ",\"slope\":" + json_double(r.slope)
         + ",\"r_squared\":" + json_double(r.r_squared)
         + ",\"adj_r_squared\":" + json_double(r.adj_r_squared)
         + ",\"intercept_se\":" + json_double(r.intercept_se)
         + ",\"slope_se\":" + json_double(r.slope_se)
         + ",\"intercept_t\":" + json_double(r.intercept_t)
         + ",\"slope_t\":" + json_double(r.slope_t)
         + ",\"intercept_p\":" + json_double(r.intercept_p)
         + ",\"slope_p\":" + json_double(r.slope_p)
         + ",\"f_statistic\":" + json_double(r.f_statistic)
         + ",\"f_p_value\":" + json_double(r.f_p_value)
         + ",\"residual_se\":" + json_double(r.residual_se) + "}";
}

static std::string calc_simple_regression(const std::vector<double>& x,
                                           const std::vector<double>& y) {
    if (x.size() < 3) return "";
    auto r = statcpp::simple_linear_regression(x.begin(), x.end(),
                                                y.begin(), y.end());
    return json_regression(r);
}

static double calc_r_squared(const std::vector<double>& actual,
                              const std::vector<double>& predicted) {
    if (actual.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::r_squared(actual.begin(), actual.end(),
                               predicted.begin(), predicted.end());
}

static double calc_adjusted_r_squared(const std::vector<double>& actual,
                                       const std::vector<double>& predicted) {
    if (actual.size() < 3) return std::numeric_limits<double>::quiet_NaN();
    // num_predictors = 1 for simple regression (2-column case)
    return statcpp::adjusted_r_squared(actual.begin(), actual.end(),
                                        predicted.begin(), predicted.end(), 1);
}

// --- Parametric tests (two-column) ---

static std::string calc_t_test_paired(const std::vector<double>& x,
                                       const std::vector<double>& y) {
    if (x.size() < 2) return "";
    auto r = statcpp::t_test_paired(x.begin(), x.end(), y.begin(), y.end());
    return json_test_result(r);
}

static std::string calc_chisq_gof(const std::vector<double>& observed,
                                   const std::vector<double>& expected) {
    if (observed.size() < 2) return "";
    auto r = statcpp::chisq_test_gof(observed.begin(), observed.end(),
                                      expected.begin(), expected.end());
    return json_test_result(r);
}

// --- Error metrics ---

static double calc_mae(const std::vector<double>& actual,
                        const std::vector<double>& predicted) {
    return statcpp::mae(actual.begin(), actual.end(), predicted.begin());
}

static double calc_mse(const std::vector<double>& actual,
                        const std::vector<double>& predicted) {
    return statcpp::mse(actual.begin(), actual.end(), predicted.begin());
}

static double calc_rmse(const std::vector<double>& actual,
                         const std::vector<double>& predicted) {
    return statcpp::rmse(actual.begin(), actual.end(), predicted.begin());
}

static double calc_mape(const std::vector<double>& actual,
                         const std::vector<double>& predicted) {
    return statcpp::mape(actual.begin(), actual.end(), predicted.begin());
}

// --- Distance metrics ---

static double calc_euclidean_dist(const std::vector<double>& a,
                                   const std::vector<double>& b) {
    return statcpp::euclidean_distance(a.begin(), a.end(), b.begin(), b.end());
}

static double calc_manhattan_dist(const std::vector<double>& a,
                                   const std::vector<double>& b) {
    return statcpp::manhattan_distance(a.begin(), a.end(), b.begin(), b.end());
}

static double calc_cosine_sim(const std::vector<double>& a,
                               const std::vector<double>& b) {
    return statcpp::cosine_similarity(a.begin(), a.end(), b.begin(), b.end());
}

static double calc_cosine_dist(const std::vector<double>& a,
                                const std::vector<double>& b) {
    return statcpp::cosine_distance(a.begin(), a.end(), b.begin(), b.end());
}

static double calc_minkowski_dist(const std::vector<double>& a,
                                   const std::vector<double>& b,
                                   const std::array<double, 1>& p) {
    return statcpp::minkowski_distance(a.begin(), a.end(),
                                        b.begin(), b.end(), p[0]);
}

static double calc_chebyshev_dist(const std::vector<double>& a,
                                   const std::vector<double>& b) {
    return statcpp::chebyshev_distance(a.begin(), a.end(), b.begin(), b.end());
}

// ===========================================================================
// Window function wrappers
// ===========================================================================

// Helper: extract non-NaN values for statcpp functions that don't handle NaN
static std::vector<double> extract_valid(const std::vector<double>& values,
                                          const std::vector<bool>& nulls) {
    std::vector<double> valid;
    valid.reserve(values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) valid.push_back(values[i]);
    }
    return valid;
}

// --- Rolling functions ---

static std::vector<double> wf_rolling_mean(const std::vector<double>& values,
                                            const std::vector<bool>& nulls,
                                            int window) {
    if (window <= 0) window = 1;
    auto w = static_cast<std::size_t>(window);
    auto result = statcpp::rolling_mean(values, w);
    // Mark rows with NULL input as NaN in result
    for (std::size_t i = 0; i < result.size() && i < nulls.size(); ++i) {
        if (nulls[i]) result[i] = std::numeric_limits<double>::quiet_NaN();
    }
    return result;
}

static std::vector<double> wf_rolling_std(const std::vector<double>& values,
                                           const std::vector<bool>& nulls,
                                           int window) {
    if (window <= 0) window = 1;
    auto w = static_cast<std::size_t>(window);
    auto result = statcpp::rolling_std(values, w);
    for (std::size_t i = 0; i < result.size() && i < nulls.size(); ++i) {
        if (nulls[i]) result[i] = std::numeric_limits<double>::quiet_NaN();
    }
    return result;
}

static std::vector<double> wf_rolling_min(const std::vector<double>& values,
                                           const std::vector<bool>& nulls,
                                           int window) {
    if (window <= 0) window = 1;
    auto w = static_cast<std::size_t>(window);
    auto result = statcpp::rolling_min(values, w);
    for (std::size_t i = 0; i < result.size() && i < nulls.size(); ++i) {
        if (nulls[i]) result[i] = std::numeric_limits<double>::quiet_NaN();
    }
    return result;
}

static std::vector<double> wf_rolling_max(const std::vector<double>& values,
                                           const std::vector<bool>& nulls,
                                           int window) {
    if (window <= 0) window = 1;
    auto w = static_cast<std::size_t>(window);
    auto result = statcpp::rolling_max(values, w);
    for (std::size_t i = 0; i < result.size() && i < nulls.size(); ++i) {
        if (nulls[i]) result[i] = std::numeric_limits<double>::quiet_NaN();
    }
    return result;
}

static std::vector<double> wf_rolling_sum(const std::vector<double>& values,
                                           const std::vector<bool>& nulls,
                                           int window) {
    if (window <= 0) window = 1;
    auto w = static_cast<std::size_t>(window);
    auto result = statcpp::rolling_sum(values, w);
    for (std::size_t i = 0; i < result.size() && i < nulls.size(); ++i) {
        if (nulls[i]) result[i] = std::numeric_limits<double>::quiet_NaN();
    }
    return result;
}

static std::vector<double> wf_moving_avg(const std::vector<double>& values,
                                          const std::vector<bool>& /*nulls*/,
                                          int window) {
    if (window <= 0) window = 1;
    auto w = static_cast<std::size_t>(window);
    auto result = statcpp::moving_average(values.begin(), values.end(), w);
    return result;
}

static std::vector<double> wf_ema(const std::vector<double>& values,
                                   const std::vector<bool>& /*nulls*/,
                                   int span) {
    if (span <= 0) span = 10;
    double alpha = 2.0 / (static_cast<double>(span) + 1.0);
    auto result = statcpp::exponential_moving_average(values.begin(), values.end(), alpha);
    return result;
}

// --- Rank ---

static std::vector<double> wf_rank(const std::vector<double>& values,
                                    const std::vector<bool>& nulls,
                                    int /*param*/) {
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto ranks = statcpp::rank_transform(valid);
    // Map back to original positions
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = ranks[vi++];
        }
    }
    return result;
}

// --- fillna functions ---

static std::vector<double> wf_fillna_mean(const std::vector<double>& values,
                                            const std::vector<bool>& nulls,
                                            int /*param*/) {
    auto result = statcpp::fillna_mean(values);
    return result;
}

static std::vector<double> wf_fillna_median(const std::vector<double>& values,
                                              const std::vector<bool>& nulls,
                                              int /*param*/) {
    auto result = statcpp::fillna_median(values);
    return result;
}

static std::vector<double> wf_fillna_ffill(const std::vector<double>& values,
                                             const std::vector<bool>& /*nulls*/,
                                             int /*param*/) {
    auto result = statcpp::fillna_ffill(values);
    return result;
}

static std::vector<double> wf_fillna_bfill(const std::vector<double>& values,
                                             const std::vector<bool>& /*nulls*/,
                                             int /*param*/) {
    auto result = statcpp::fillna_bfill(values);
    return result;
}

static std::vector<double> wf_fillna_interp(const std::vector<double>& values,
                                              const std::vector<bool>& /*nulls*/,
                                              int /*param*/) {
    auto result = statcpp::fillna_interpolate(values);
    return result;
}

// --- Label encode ---

static std::vector<double> wf_label_encode(const std::vector<double>& values,
                                             const std::vector<bool>& nulls,
                                             int /*param*/) {
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto enc = statcpp::label_encode(valid);
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = static_cast<double>(enc.encoded[vi++]);
        }
    }
    return result;
}

// --- Binning ---

static std::vector<double> wf_bin_width(const std::vector<double>& values,
                                          const std::vector<bool>& nulls,
                                          int n_bins) {
    if (n_bins <= 0) n_bins = 10;
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto bins = statcpp::bin_equal_width(valid, static_cast<std::size_t>(n_bins));
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = static_cast<double>(bins[vi++]);
        }
    }
    return result;
}

static std::vector<double> wf_bin_freq(const std::vector<double>& values,
                                         const std::vector<bool>& nulls,
                                         int n_bins) {
    if (n_bins <= 0) n_bins = 10;
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto bins = statcpp::bin_equal_freq(valid, static_cast<std::size_t>(n_bins));
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = static_cast<double>(bins[vi++]);
        }
    }
    return result;
}

// --- Lag ---

static std::vector<double> wf_lag(const std::vector<double>& values,
                                   const std::vector<bool>& /*nulls*/,
                                   int k) {
    if (k <= 0) k = 1;
    auto result = statcpp::lag(values.begin(), values.end(),
                                static_cast<std::size_t>(k));
    return result;
}

// --- Outlier detection ---

static std::vector<double> wf_outliers_iqr(const std::vector<double>& values,
                                             const std::vector<bool>& nulls,
                                             int /*param*/) {
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto det = statcpp::detect_outliers_iqr(valid.begin(), valid.end());
    // Return 1.0 for outlier, 0.0 for normal, NaN for NULL
    // Build a set of outlier indices for lookup
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::vector<bool> is_outlier(valid.size(), false);
    for (auto idx : det.outlier_indices) {
        if (idx < is_outlier.size()) is_outlier[idx] = true;
    }
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = is_outlier[vi] ? 1.0 : 0.0;
            vi++;
        }
    }
    return result;
}

static std::vector<double> wf_outliers_zscore(const std::vector<double>& values,
                                                const std::vector<bool>& nulls,
                                                int /*param*/) {
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto det = statcpp::detect_outliers_zscore(valid.begin(), valid.end());
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::vector<bool> is_outlier(valid.size(), false);
    for (auto idx : det.outlier_indices) {
        if (idx < is_outlier.size()) is_outlier[idx] = true;
    }
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = is_outlier[vi] ? 1.0 : 0.0;
            vi++;
        }
    }
    return result;
}

static std::vector<double> wf_outliers_mzscore(const std::vector<double>& values,
                                                 const std::vector<bool>& nulls,
                                                 int /*param*/) {
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto det = statcpp::detect_outliers_modified_zscore(valid.begin(), valid.end());
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::vector<bool> is_outlier(valid.size(), false);
    for (auto idx : det.outlier_indices) {
        if (idx < is_outlier.size()) is_outlier[idx] = true;
    }
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = is_outlier[vi] ? 1.0 : 0.0;
            vi++;
        }
    }
    return result;
}

// --- Winsorize ---

static std::vector<double> wf_winsorize(const std::vector<double>& values,
                                          const std::vector<bool>& nulls,
                                          int /*param*/) {
    auto valid = extract_valid(values, nulls);
    if (valid.empty()) {
        return std::vector<double>(values.size(), std::numeric_limits<double>::quiet_NaN());
    }
    auto wins = statcpp::winsorize(valid.begin(), valid.end());
    std::vector<double> result(values.size(), std::numeric_limits<double>::quiet_NaN());
    std::size_t vi = 0;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (!nulls[i]) {
            result[i] = wins[vi++];
        }
    }
    return result;
}

// --- Diff ---

static std::vector<double> wf_diff(const std::vector<double>& values,
                                    const std::vector<bool>& /*nulls*/,
                                    int order) {
    if (order <= 0) order = 1;
    auto result = statcpp::diff(values.begin(), values.end(),
                                 static_cast<std::size_t>(order));
    // diff returns n-order elements; pad front with NaN
    std::vector<double> padded(values.size(), std::numeric_limits<double>::quiet_NaN());
    for (std::size_t i = 0; i < result.size(); ++i) {
        padded[i + static_cast<std::size_t>(order)] = result[i];
    }
    return padded;
}

static std::vector<double> wf_seasonal_diff(const std::vector<double>& values,
                                              const std::vector<bool>& /*nulls*/,
                                              int period) {
    if (period <= 0) period = 1;
    auto result = statcpp::seasonal_diff(values.begin(), values.end(),
                                          static_cast<std::size_t>(period));
    // seasonal_diff returns n-period elements; pad front with NaN
    std::vector<double> padded(values.size(), std::numeric_limits<double>::quiet_NaN());
    for (std::size_t i = 0; i < result.size(); ++i) {
        padded[i + static_cast<std::size_t>(period)] = result[i];
    }
    return padded;
}

// ===========================================================================
// Template E — SingleColumnAggregateText
//
// Like Template A but returns JSON text. Used for P5 functions that
// collect a single column and return structured JSON results.
// ===========================================================================

template <std::string (*Func)(const std::vector<double>&)>
class SingleColumnAggregateText {
public:
    static void xStep(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
        if (sqlite3_value_type(argv[0]) == SQLITE_NULL) return;
        auto* state = getOrCreateState(ctx);
        if (!state) return;
        state->values.push_back(sqlite3_value_double(argv[0]));
    }

    static void xFinal(sqlite3_context* ctx) {
        auto** pp = static_cast<SingleColumnState**>(
            sqlite3_aggregate_context(ctx, 0));
        if (!pp || !*pp || (*pp)->values.empty()) {
            sqlite3_result_null(ctx);
            cleanupState(pp);
            return;
        }
        std::string result = Func((*pp)->values);
        cleanupState(pp);
        if (result.empty()) {
            sqlite3_result_null(ctx);
        } else {
            sqlite3_result_text(ctx, result.c_str(),
                                static_cast<int>(result.size()),
                                SQLITE_TRANSIENT);
        }
    }

    static int register_func(sqlite3* db, const char* name) {
        return sqlite3_create_function_v2(
            db, name, 1,
            SQLITE_UTF8 | SQLITE_DETERMINISTIC,
            nullptr, nullptr, xStep, xFinal, nullptr);
    }

private:
    static SingleColumnState* getOrCreateState(sqlite3_context* ctx) {
        auto** pp = static_cast<SingleColumnState**>(
            sqlite3_aggregate_context(ctx, sizeof(SingleColumnState*)));
        if (!pp) return nullptr;
        if (!*pp) {
            *pp = new (std::nothrow) SingleColumnState();
            if (!*pp) {
                sqlite3_result_error_nomem(ctx);
                return nullptr;
            }
        }
        return *pp;
    }

    static void cleanupState(SingleColumnState** pp) {
        if (pp && *pp) {
            delete *pp;
            *pp = nullptr;
        }
    }
};

// ===========================================================================
// Template F — ScalarFunction
//
// For P6/P7 scalar functions that take only parameters (no aggregation).
// Uses sqlite3_create_function_v2 with xFunc callback.
// ===========================================================================

// Generic scalar registration helper (implemented per-function via lambdas)
static int register_scalar(sqlite3* db, const char* name, int nArgs,
                           void (*xFunc)(sqlite3_context*, int, sqlite3_value**)) {
    return sqlite3_create_function_v2(
        db, name, nArgs,
        SQLITE_UTF8 | SQLITE_DETERMINISTIC,
        nullptr, xFunc, nullptr, nullptr, nullptr);
}

// Non-deterministic variant (for random functions)
static int register_scalar_nd(sqlite3* db, const char* name, int nArgs,
                              void (*xFunc)(sqlite3_context*, int, sqlite3_value**)) {
    return sqlite3_create_function_v2(
        db, name, nArgs,
        SQLITE_UTF8,
        nullptr, xFunc, nullptr, nullptr, nullptr);
}

// Helper: set double result with NaN/Inf → NULL
static void result_double_or_null(sqlite3_context* ctx, double v) {
    if (std::isnan(v) || std::isinf(v)) {
        sqlite3_result_null(ctx);
    } else {
        sqlite3_result_double(ctx, v);
    }
}

// Helper: set int64 result
static void result_int64(sqlite3_context* ctx, std::int64_t v) {
    sqlite3_result_int64(ctx, v);
}

// Helper: set text result
static void result_text(sqlite3_context* ctx, const std::string& s) {
    if (s.empty()) {
        sqlite3_result_null(ctx);
    } else {
        sqlite3_result_text(ctx, s.c_str(), static_cast<int>(s.size()),
                            SQLITE_TRANSIENT);
    }
}

// ===========================================================================
// Complex aggregate wrapper functions — two-sample / JSON
// ===========================================================================

// --- P5-1: stat_modes (single column → JSON array of modes) ---
static std::string calc_modes(const std::vector<double>& v) {
    auto m = statcpp::modes(v.begin(), v.end());
    std::string s = "[";
    for (std::size_t i = 0; i < m.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(m[i]);
    }
    s += "]";
    return s;
}

// --- P5-2: stat_five_number_summary ---
static std::string calc_five_number_summary(const std::vector<double>& v) {
    std::vector<double> sorted(v);
    std::sort(sorted.begin(), sorted.end());
    double mn = sorted.front();
    auto qr = statcpp::quartiles(sorted.begin(), sorted.end());
    double q1 = qr.q1;
    double med = qr.q2;
    double q3 = qr.q3;
    double mx = sorted.back();
    return "{\"min\":" + json_double(mn)
         + ",\"q1\":" + json_double(q1)
         + ",\"median\":" + json_double(med)
         + ",\"q3\":" + json_double(q3)
         + ",\"max\":" + json_double(mx) + "}";
}

// --- P5-3: stat_frequency_table ---
static std::string calc_frequency_table(const std::vector<double>& v) {
    auto ft = statcpp::frequency_table(v.begin(), v.end());
    std::string s = "[";
    for (std::size_t i = 0; i < ft.entries.size(); ++i) {
        if (i > 0) s += ",";
        s += "{\"value\":" + json_double(ft.entries[i].value)
           + ",\"count\":" + std::to_string(ft.entries[i].count)
           + ",\"relative_frequency\":" + json_double(ft.entries[i].relative_frequency) + "}";
    }
    s += "]";
    return s;
}

// --- P5-4: stat_frequency_count (single column → JSON {value:count,...}) ---
static std::string calc_frequency_count(const std::vector<double>& v) {
    auto fc = statcpp::frequency_count(v.begin(), v.end());
    std::string s = "{";
    bool first = true;
    for (auto& p : fc) {
        if (!first) s += ",";
        first = false;
        s += "\"" + json_double(p.first) + "\":" + std::to_string(p.second);
    }
    s += "}";
    return s;
}

// --- P5-5: stat_relative_frequency ---
static std::string calc_relative_frequency(const std::vector<double>& v) {
    auto rf = statcpp::relative_frequency(v.begin(), v.end());
    std::string s = "{";
    bool first = true;
    for (auto& p : rf) {
        if (!first) s += ",";
        first = false;
        s += "\"" + json_double(p.first) + "\":" + json_double(p.second);
    }
    s += "}";
    return s;
}

// --- P5-6: stat_cumulative_frequency ---
static std::string calc_cumulative_frequency(const std::vector<double>& v) {
    auto cf = statcpp::cumulative_frequency(v.begin(), v.end());
    std::string s = "{";
    bool first = true;
    for (auto& p : cf) {
        if (!first) s += ",";
        first = false;
        s += "\"" + json_double(p.first) + "\":" + std::to_string(p.second);
    }
    s += "}";
    return s;
}

// --- P5-7: stat_cumulative_relative_frequency ---
static std::string calc_cumulative_relative_frequency(const std::vector<double>& v) {
    auto crf = statcpp::cumulative_relative_frequency(v.begin(), v.end());
    std::string s = "{";
    bool first = true;
    for (auto& p : crf) {
        if (!first) s += ",";
        first = false;
        s += "\"" + json_double(p.first) + "\":" + json_double(p.second);
    }
    s += "}";
    return s;
}

// --- P5-8..12: Two-sample tests (use TwoColumnAggregateText) ---

static std::string calc_t_test2(const std::vector<double>& x,
                                 const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return "";
    auto r = statcpp::t_test_two_sample(x.begin(), x.end(), y.begin(), y.end());
    return json_test_result(r);
}

static std::string calc_t_test_welch(const std::vector<double>& x,
                                      const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return "";
    auto r = statcpp::t_test_welch(x.begin(), x.end(), y.begin(), y.end());
    return json_test_result(r);
}

static std::string calc_chisq_independence(const std::vector<double>& x,
                                            const std::vector<double>& y) {
    if (x.size() < 2) return "";
    // Build a 2-column contingency table from x and y values
    // x = row category, y = col category
    std::map<double, std::size_t> row_map, col_map;
    for (auto v : x) row_map.insert({v, row_map.size()});
    for (auto v : y) col_map.insert({v, col_map.size()});
    std::vector<std::vector<double>> table(row_map.size(),
        std::vector<double>(col_map.size(), 0.0));
    for (std::size_t i = 0; i < x.size(); ++i) {
        table[row_map[x[i]]][col_map[y[i]]] += 1.0;
    }
    auto r = statcpp::chisq_test_independence(table);
    return json_test_result(r);
}

static std::string calc_f_test(const std::vector<double>& x,
                                const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return "";
    auto r = statcpp::f_test(x.begin(), x.end(), y.begin(), y.end());
    return json_test_result(r);
}

static std::string calc_mann_whitney(const std::vector<double>& x,
                                      const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return "";
    auto r = statcpp::mann_whitney_u_test(x.begin(), x.end(), y.begin(), y.end());
    return json_test_result(r);
}

// --- P5-13: stat_anova1 (two-column: value, group → JSON) ---
static std::string calc_anova1(const std::vector<double>& values,
                                const std::vector<double>& groups) {
    // Split values by group
    std::map<double, std::vector<double>> grouped;
    for (std::size_t i = 0; i < values.size(); ++i) {
        grouped[groups[i]].push_back(values[i]);
    }
    std::vector<std::vector<double>> group_vecs;
    for (auto& p : grouped) group_vecs.push_back(std::move(p.second));
    if (group_vecs.size() < 2) return "";
    auto r = statcpp::one_way_anova(group_vecs);
    return "{\"f_statistic\":" + json_double(r.between.f_statistic)
         + ",\"p_value\":" + json_double(r.between.p_value)
         + ",\"df_between\":" + json_double(r.between.df)
         + ",\"df_within\":" + json_double(r.within.df)
         + ",\"ss_between\":" + json_double(r.between.ss)
         + ",\"ss_within\":" + json_double(r.within.ss)
         + ",\"n_groups\":" + std::to_string(r.n_groups) + "}";
}

// --- P5-14: stat_contingency_table ---
static std::string calc_contingency_table(const std::vector<double>& x,
                                           const std::vector<double>& y) {
    // Convert to size_t indices
    std::map<double, std::size_t> row_map, col_map;
    std::vector<std::size_t> rows, cols;
    for (auto v : x) {
        auto it = row_map.insert({v, row_map.size()});
        rows.push_back(it.first->second);
    }
    for (auto v : y) {
        auto it = col_map.insert({v, col_map.size()});
        cols.push_back(it.first->second);
    }
    auto ct = statcpp::contingency_table(rows, cols);
    // Output as JSON 2D array
    std::string s = "{\"table\":[";
    for (std::size_t r = 0; r < ct.n_rows; ++r) {
        if (r > 0) s += ",";
        s += "[";
        for (std::size_t c = 0; c < ct.n_cols; ++c) {
            if (c > 0) s += ",";
            s += std::to_string(ct.table[r][c]);
        }
        s += "]";
    }
    s += "],\"total\":" + std::to_string(ct.total) + "}";
    return s;
}

// --- P5-15..17: Two-sample effect sizes ---
static double calc_cohens_d2(const std::vector<double>& x,
                              const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::cohens_d_two_sample(x.begin(), x.end(), y.begin(), y.end());
}

static double calc_hedges_g2(const std::vector<double>& x,
                              const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::hedges_g_two_sample(x.begin(), x.end(), y.begin(), y.end());
}

static double calc_glass_delta(const std::vector<double>& x,
                                const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return std::numeric_limits<double>::quiet_NaN();
    return statcpp::glass_delta(x.begin(), x.end(), y.begin(), y.end());
}

// --- P5-18..19: CI for mean difference ---
static std::string calc_ci_mean_diff(const std::vector<double>& x,
                                      const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return "";
    auto ci = statcpp::ci_mean_diff(x.begin(), x.end(), y.begin(), y.end());
    return json_ci(ci);
}

static std::string calc_ci_mean_diff_welch(const std::vector<double>& x,
                                            const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return "";
    auto ci = statcpp::ci_mean_diff_welch(x.begin(), x.end(), y.begin(), y.end());
    return json_ci(ci);
}

// --- P5-20: stat_kaplan_meier (two-column: time, event → JSON) ---
static std::string calc_kaplan_meier(const std::vector<double>& times,
                                      const std::vector<double>& events) {
    std::vector<bool> ev;
    ev.reserve(events.size());
    for (auto e : events) ev.push_back(e != 0.0);
    auto km = statcpp::kaplan_meier(times, ev);
    std::string s = "{\"times\":[";
    for (std::size_t i = 0; i < km.times.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(km.times[i]);
    }
    s += "],\"survival\":[";
    for (std::size_t i = 0; i < km.survival.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(km.survival[i]);
    }
    s += "],\"se\":[";
    for (std::size_t i = 0; i < km.se.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(km.se[i]);
    }
    s += "]}";
    return s;
}

// --- P5-21: stat_logrank (needs 4 columns — we use col1=time, col2=event_or_group) ---
// For logrank, we split by group: col1=time, col2=event(0/1)*10+group(0/1)
// Simpler: col1=time, col2=group. Event is assumed=1 for all (no censoring in basic usage).
// OR: we register as 3-arg aggregate: time, event, group
// Let's use a direct registration with 3 columns.
// We'll handle this with a custom registration.

// --- P5-22: stat_nelson_aalen ---
static std::string calc_nelson_aalen(const std::vector<double>& times,
                                      const std::vector<double>& events) {
    std::vector<bool> ev;
    ev.reserve(events.size());
    for (auto e : events) ev.push_back(e != 0.0);
    auto na = statcpp::nelson_aalen(times, ev);
    std::string s = "{\"times\":[";
    for (std::size_t i = 0; i < na.times.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(na.times[i]);
    }
    s += "],\"cumulative_hazard\":[";
    for (std::size_t i = 0; i < na.cumulative_hazard.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(na.cumulative_hazard[i]);
    }
    s += "]}";
    return s;
}

// --- P5-23: stat_bootstrap (single column → JSON, uses mean as default statistic) ---
static std::string calc_bootstrap_generic(const std::vector<double>& v,
                                           const std::array<double, 1>& p) {
    auto n = static_cast<std::size_t>(p[0]);
    if (n < 2) n = 1000;
    auto stat_func = [](auto f, auto l) { return statcpp::mean(f, l); };
    auto r = statcpp::bootstrap(v.begin(), v.end(), stat_func, n);
    return json_bootstrap(r);
}

// --- P5-24: stat_bootstrap_bca ---
static std::string calc_bootstrap_bca(const std::vector<double>& v,
                                       const std::array<double, 1>& p) {
    if (v.size() < 3) return "";
    auto n = static_cast<std::size_t>(p[0]);
    if (n < 2) n = 1000;
    auto stat_func = [](auto f, auto l) { return statcpp::mean(f, l); };
    auto r = statcpp::bootstrap_bca(v.begin(), v.end(), stat_func, n);
    return json_bootstrap(r);
}

// --- P5-25: stat_bootstrap_sample (returns JSON array of sampled values) ---
static std::string calc_bootstrap_sample_text(const std::vector<double>& v,
                                               const std::array<double, 0>&) {
    auto sample = statcpp::bootstrap_sample(v.begin(), v.end());
    std::string s = "[";
    for (std::size_t i = 0; i < sample.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(sample[i]);
    }
    s += "]";
    return s;
}

// --- P5-26..28: Permutation tests (two-column) ---
static std::string json_permutation(const statcpp::permutation_result& r) {
    return "{\"observed_statistic\":" + json_double(r.observed_statistic)
         + ",\"p_value\":" + json_double(r.p_value)
         + ",\"n_permutations\":" + std::to_string(r.n_permutations) + "}";
}

static std::string calc_permutation_test2(const std::vector<double>& x,
                                           const std::vector<double>& y) {
    if (x.size() < 2 || y.size() < 2) return "";
    auto r = statcpp::permutation_test_two_sample(x.begin(), x.end(),
                                                    y.begin(), y.end(), 1000);
    return json_permutation(r);
}

static std::string calc_permutation_paired(const std::vector<double>& x,
                                            const std::vector<double>& y) {
    if (x.size() < 2) return "";
    auto r = statcpp::permutation_test_paired(x.begin(), x.end(),
                                               y.begin(), y.end(), 1000);
    return json_permutation(r);
}

static std::string calc_permutation_corr(const std::vector<double>& x,
                                          const std::vector<double>& y) {
    if (x.size() < 3) return "";
    auto r = statcpp::permutation_test_correlation(x.begin(), x.end(),
                                                    y.begin(), y.end(), 1000);
    return json_permutation(r);
}

// --- P5-29..30: ACF / PACF (single column + max_lag param → JSON array) ---
static std::string calc_acf(const std::vector<double>& v,
                              const std::array<double, 1>& p) {
    auto max_lag = static_cast<std::size_t>(p[0]);
    if (max_lag == 0 || max_lag >= v.size()) max_lag = v.size() / 2;
    auto acf_vals = statcpp::acf(v.begin(), v.end(), max_lag);
    std::string s = "[";
    for (std::size_t i = 0; i < acf_vals.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(acf_vals[i]);
    }
    s += "]";
    return s;
}

static std::string calc_pacf(const std::vector<double>& v,
                               const std::array<double, 1>& p) {
    auto max_lag = static_cast<std::size_t>(p[0]);
    if (max_lag == 0 || max_lag >= v.size()) max_lag = v.size() / 2;
    auto pacf_vals = statcpp::pacf(v.begin(), v.end(), max_lag);
    std::string s = "[";
    for (std::size_t i = 0; i < pacf_vals.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(pacf_vals[i]);
    }
    s += "]";
    return s;
}

// --- P5-31..32: Sampling ---
static std::string calc_sample_replace(const std::vector<double>& v,
                                        const std::array<double, 1>& p) {
    auto n = static_cast<std::size_t>(p[0]);
    if (n == 0) n = v.size();
    auto sample = statcpp::sample_with_replacement(v, n);
    std::string s = "[";
    for (std::size_t i = 0; i < sample.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(sample[i]);
    }
    s += "]";
    return s;
}

static std::string calc_sample_noreplace(const std::vector<double>& v,
                                          const std::array<double, 1>& p) {
    auto n = static_cast<std::size_t>(p[0]);
    if (n == 0 || n > v.size()) n = v.size();
    auto sample = statcpp::sample_without_replacement(v, n);
    std::string s = "[";
    for (std::size_t i = 0; i < sample.size(); ++i) {
        if (i > 0) s += ",";
        s += json_double(sample[i]);
    }
    s += "]";
    return s;
}

// --- P5-21 (logrank): Direct registration with 3 columns ---
struct ThreeColumnState {
    std::vector<double> c1, c2, c3;
};

static void logrank_step(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    if (sqlite3_value_type(argv[0]) == SQLITE_NULL ||
        sqlite3_value_type(argv[1]) == SQLITE_NULL ||
        sqlite3_value_type(argv[2]) == SQLITE_NULL) return;
    auto** pp = static_cast<ThreeColumnState**>(
        sqlite3_aggregate_context(ctx, sizeof(ThreeColumnState*)));
    if (!pp) return;
    if (!*pp) {
        *pp = new (std::nothrow) ThreeColumnState();
        if (!*pp) { sqlite3_result_error_nomem(ctx); return; }
    }
    (*pp)->c1.push_back(sqlite3_value_double(argv[0])); // time
    (*pp)->c2.push_back(sqlite3_value_double(argv[1])); // event
    (*pp)->c3.push_back(sqlite3_value_double(argv[2])); // group
}

static void logrank_final(sqlite3_context* ctx) {
    auto** pp = static_cast<ThreeColumnState**>(
        sqlite3_aggregate_context(ctx, 0));
    if (!pp || !*pp || (*pp)->c1.empty()) {
        sqlite3_result_null(ctx);
        if (pp && *pp) { delete *pp; *pp = nullptr; }
        return;
    }
    auto* st = *pp;
    // Split by group (0 vs non-0)
    std::vector<double> t1, t2;
    std::vector<bool> e1, e2;
    for (std::size_t i = 0; i < st->c1.size(); ++i) {
        if (st->c3[i] == 0.0) {
            t1.push_back(st->c1[i]);
            e1.push_back(st->c2[i] != 0.0);
        } else {
            t2.push_back(st->c1[i]);
            e2.push_back(st->c2[i] != 0.0);
        }
    }
    delete st; *pp = nullptr;
    if (t1.empty() || t2.empty()) { sqlite3_result_null(ctx); return; }
    auto r = statcpp::logrank_test(t1, e1, t2, e2);
    std::string s = "{\"statistic\":" + json_double(r.statistic)
                  + ",\"p_value\":" + json_double(r.p_value)
                  + ",\"df\":" + std::to_string(r.df) + "}";
    sqlite3_result_text(ctx, s.c_str(), static_cast<int>(s.size()), SQLITE_TRANSIENT);
}

// ===========================================================================
// Scalar functions — tests and helpers
// ===========================================================================

// --- P6-1..16: Distribution functions (normal, chi-sq, t, F) ---

static void sf_normal_pdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double mu = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double sigma = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::normal_pdf(x, mu, sigma));
}
static void sf_normal_cdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double mu = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double sigma = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::normal_cdf(x, mu, sigma));
}
static void sf_normal_quantile(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    double mu = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double sigma = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::normal_quantile(p, mu, sigma));
}
static void sf_normal_rand(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double mu = (argc > 0 && sqlite3_value_type(argv[0]) != SQLITE_NULL) ? sqlite3_value_double(argv[0]) : 0.0;
    double sigma = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 1.0;
    result_double_or_null(ctx, statcpp::normal_rand(mu, sigma));
}

static void sf_chisq_pdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::chisq_pdf(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1])));
}
static void sf_chisq_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::chisq_cdf(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1])));
}
static void sf_chisq_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::chisq_quantile(sqlite3_value_double(argv[0]),
                                                        sqlite3_value_double(argv[1])));
}
static void sf_chisq_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::chisq_rand(sqlite3_value_double(argv[0])));
}

static void sf_t_pdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::t_pdf(sqlite3_value_double(argv[0]),
                                               sqlite3_value_double(argv[1])));
}
static void sf_t_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::t_cdf(sqlite3_value_double(argv[0]),
                                               sqlite3_value_double(argv[1])));
}
static void sf_t_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::t_quantile(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1])));
}
static void sf_t_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::t_rand(sqlite3_value_double(argv[0])));
}

static void sf_f_pdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::f_pdf(sqlite3_value_double(argv[0]),
                                               sqlite3_value_double(argv[1]),
                                               sqlite3_value_double(argv[2])));
}
static void sf_f_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::f_cdf(sqlite3_value_double(argv[0]),
                                               sqlite3_value_double(argv[1]),
                                               sqlite3_value_double(argv[2])));
}
static void sf_f_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::f_quantile(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1]),
                                                    sqlite3_value_double(argv[2])));
}
static void sf_f_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::f_rand(sqlite3_value_double(argv[0]),
                                                sqlite3_value_double(argv[1])));
}

// --- P6-17..23: Special functions ---
static void sf_betainc(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::betainc(sqlite3_value_double(argv[0]),
                                                 sqlite3_value_double(argv[1]),
                                                 sqlite3_value_double(argv[2])));
}
static void sf_betaincinv(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::betaincinv(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1]),
                                                    sqlite3_value_double(argv[2])));
}
static void sf_norm_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::norm_cdf(sqlite3_value_double(argv[0])));
}
static void sf_norm_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::norm_quantile(sqlite3_value_double(argv[0])));
}
static void sf_gammainc_lower(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::gammainc_lower(sqlite3_value_double(argv[0]),
                                                        sqlite3_value_double(argv[1])));
}
static void sf_gammainc_upper(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::gammainc_upper(sqlite3_value_double(argv[0]),
                                                        sqlite3_value_double(argv[1])));
}
static void sf_gammainc_lower_inv(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::gammainc_lower_inv(sqlite3_value_double(argv[0]),
                                                            sqlite3_value_double(argv[1])));
}

// --- P6-24..25: Proportion z-tests ---
static void sf_z_test_prop(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto succ = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto trials = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    double p0 = sqlite3_value_double(argv[2]);
    auto r = statcpp::z_test_proportion(succ, trials, p0);
    std::string s = json_test_result(r);
    result_text(ctx, s);
}
static void sf_z_test_prop2(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto s1 = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto n1 = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto s2 = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    auto n2 = static_cast<std::size_t>(sqlite3_value_int64(argv[3]));
    auto r = statcpp::z_test_proportion_two_sample(s1, n1, s2, n2);
    std::string s = json_test_result(r);
    result_text(ctx, s);
}

// --- P6-26..28: Multiple testing corrections (scalar: takes comma-separated p-values as JSON) ---
// These take individual p-value and correction count, returning adjusted p-value
// For simplicity: stat_bonferroni(p, n) = min(p*n, 1.0)
static void sf_bonferroni(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    int n = sqlite3_value_int(argv[1]);
    double adj = std::min(p * n, 1.0);
    result_double_or_null(ctx, adj);
}
static void sf_bh_correction(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    // BH: stat_bh_correction(p, rank, total) = min(p * total / rank, 1.0)
    double p = sqlite3_value_double(argv[0]);
    int rank = sqlite3_value_int(argv[1]);
    int total = sqlite3_value_int(argv[2]);
    if (rank <= 0) { sqlite3_result_null(ctx); return; }
    double adj = std::min(p * static_cast<double>(total) / static_cast<double>(rank), 1.0);
    result_double_or_null(ctx, adj);
}
static void sf_holm_correction(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    // Holm: stat_holm_correction(p, rank, total) = min(p * (total - rank + 1), 1.0)
    double p = sqlite3_value_double(argv[0]);
    int rank = sqlite3_value_int(argv[1]);
    int total = sqlite3_value_int(argv[2]);
    double adj = std::min(p * static_cast<double>(total - rank + 1), 1.0);
    result_double_or_null(ctx, adj);
}

// --- P6-29: Fisher exact test ---
static void sf_fisher_exact(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto a = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto b = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    auto c = static_cast<std::uint64_t>(sqlite3_value_int64(argv[2]));
    auto d = static_cast<std::uint64_t>(sqlite3_value_int64(argv[3]));
    auto r = statcpp::fisher_exact_test(a, b, c, d);
    std::string s = json_test_result(r);
    result_text(ctx, s);
}

// --- P6-30..33: Categorical measures ---
static void sf_odds_ratio(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto a = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto b = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto c = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    auto d = static_cast<std::size_t>(sqlite3_value_int64(argv[3]));
    auto r = statcpp::odds_ratio(a, b, c, d);
    std::string s = "{\"odds_ratio\":" + json_double(r.odds_ratio)
                  + ",\"ci_lower\":" + json_double(r.ci_lower)
                  + ",\"ci_upper\":" + json_double(r.ci_upper) + "}";
    result_text(ctx, s);
}
static void sf_relative_risk(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto a = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto b = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto c = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    auto d = static_cast<std::size_t>(sqlite3_value_int64(argv[3]));
    auto r = statcpp::relative_risk(a, b, c, d);
    std::string s = "{\"relative_risk\":" + json_double(r.relative_risk)
                  + ",\"ci_lower\":" + json_double(r.ci_lower)
                  + ",\"ci_upper\":" + json_double(r.ci_upper) + "}";
    result_text(ctx, s);
}
static void sf_risk_difference(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto a = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto b = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto c = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    auto d = static_cast<std::size_t>(sqlite3_value_int64(argv[3]));
    auto r = statcpp::risk_difference(a, b, c, d);
    std::string s = "{\"risk_difference\":" + json_double(r.risk_difference)
                  + ",\"ci_lower\":" + json_double(r.ci_lower)
                  + ",\"ci_upper\":" + json_double(r.ci_upper) + "}";
    result_text(ctx, s);
}
static void sf_nnt(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto a = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto b = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto c = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    auto d = static_cast<std::size_t>(sqlite3_value_int64(argv[3]));
    std::vector<std::vector<std::size_t>> table = {{a, b}, {c, d}};
    double nnt = statcpp::number_needed_to_treat(table);
    result_double_or_null(ctx, nnt);
}

// --- P6-34..36: Proportion confidence intervals ---
static void sf_ci_prop(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    auto succ = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto trials = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    double conf = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 0.95;
    auto ci = statcpp::ci_proportion(succ, trials, conf);
    std::string s = json_ci(ci);
    result_text(ctx, s);
}
static void sf_ci_prop_wilson(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    auto succ = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto trials = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    double conf = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 0.95;
    auto ci = statcpp::ci_proportion_wilson(succ, trials, conf);
    std::string s = json_ci(ci);
    result_text(ctx, s);
}
static void sf_ci_prop_diff(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    auto s1 = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto n1 = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto s2 = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    auto n2 = static_cast<std::size_t>(sqlite3_value_int64(argv[3]));
    double conf = (argc > 4 && sqlite3_value_type(argv[4]) != SQLITE_NULL) ? sqlite3_value_double(argv[4]) : 0.95;
    auto ci = statcpp::ci_proportion_diff(s1, n1, s2, n2, conf);
    std::string s = json_ci(ci);
    result_text(ctx, s);
}

// --- P6-37..39: Model selection ---
static void sf_aic(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double ll = sqlite3_value_double(argv[0]);
    auto k = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    result_double_or_null(ctx, statcpp::aic(ll, k));
}
static void sf_aicc(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double ll = sqlite3_value_double(argv[0]);
    auto n = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto k = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    result_double_or_null(ctx, statcpp::aicc(ll, n, k));
}
static void sf_bic(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double ll = sqlite3_value_double(argv[0]);
    auto n = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto k = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    result_double_or_null(ctx, statcpp::bic(ll, n, k));
}

// --- P6-40: Box-Cox transform ---
static void sf_boxcox(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double lambda = sqlite3_value_double(argv[1]);
    // Box-Cox: (x^lambda - 1) / lambda if lambda != 0, else log(x)
    double result;
    if (std::abs(lambda) < 1e-10) {
        result = std::log(x);
    } else {
        result = (std::pow(x, lambda) - 1.0) / lambda;
    }
    result_double_or_null(ctx, result);
}

// ===========================================================================
// Scalar functions — distributions and transforms
// ===========================================================================

// --- P7-1..24: Continuous distributions (uniform, exponential, gamma, beta, lognormal, weibull) ---

static void sf_uniform_pdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double a = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double b = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::uniform_pdf(x, a, b));
}
static void sf_uniform_cdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double a = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double b = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::uniform_cdf(x, a, b));
}
static void sf_uniform_quantile(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    double a = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double b = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::uniform_quantile(p, a, b));
}
static void sf_uniform_rand(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double a = (argc > 0 && sqlite3_value_type(argv[0]) != SQLITE_NULL) ? sqlite3_value_double(argv[0]) : 0.0;
    double b = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 1.0;
    result_double_or_null(ctx, statcpp::uniform_rand(a, b));
}

static void sf_exponential_pdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double lambda = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 1.0;
    result_double_or_null(ctx, statcpp::exponential_pdf(x, lambda));
}
static void sf_exponential_cdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double lambda = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 1.0;
    result_double_or_null(ctx, statcpp::exponential_cdf(x, lambda));
}
static void sf_exponential_quantile(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    double lambda = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 1.0;
    result_double_or_null(ctx, statcpp::exponential_quantile(p, lambda));
}
static void sf_exponential_rand(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double lambda = (argc > 0 && sqlite3_value_type(argv[0]) != SQLITE_NULL) ? sqlite3_value_double(argv[0]) : 1.0;
    result_double_or_null(ctx, statcpp::exponential_rand(lambda));
}

static void sf_gamma_pdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::gamma_pdf(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1]),
                                                    sqlite3_value_double(argv[2])));
}
static void sf_gamma_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::gamma_cdf(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1]),
                                                    sqlite3_value_double(argv[2])));
}
static void sf_gamma_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::gamma_quantile(sqlite3_value_double(argv[0]),
                                                        sqlite3_value_double(argv[1]),
                                                        sqlite3_value_double(argv[2])));
}
static void sf_gamma_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::gamma_rand(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1])));
}

static void sf_beta_pdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::beta_pdf(sqlite3_value_double(argv[0]),
                                                   sqlite3_value_double(argv[1]),
                                                   sqlite3_value_double(argv[2])));
}
static void sf_beta_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::beta_cdf(sqlite3_value_double(argv[0]),
                                                   sqlite3_value_double(argv[1]),
                                                   sqlite3_value_double(argv[2])));
}
static void sf_beta_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::beta_quantile(sqlite3_value_double(argv[0]),
                                                       sqlite3_value_double(argv[1]),
                                                       sqlite3_value_double(argv[2])));
}
static void sf_beta_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::beta_rand(sqlite3_value_double(argv[0]),
                                                    sqlite3_value_double(argv[1])));
}

static void sf_lognormal_pdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double mu = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double sigma = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::lognormal_pdf(x, mu, sigma));
}
static void sf_lognormal_cdf(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double x = sqlite3_value_double(argv[0]);
    double mu = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double sigma = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::lognormal_cdf(x, mu, sigma));
}
static void sf_lognormal_quantile(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    double mu = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.0;
    double sigma = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 1.0;
    result_double_or_null(ctx, statcpp::lognormal_quantile(p, mu, sigma));
}
static void sf_lognormal_rand(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double mu = (argc > 0 && sqlite3_value_type(argv[0]) != SQLITE_NULL) ? sqlite3_value_double(argv[0]) : 0.0;
    double sigma = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 1.0;
    result_double_or_null(ctx, statcpp::lognormal_rand(mu, sigma));
}

static void sf_weibull_pdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::weibull_pdf(sqlite3_value_double(argv[0]),
                                                      sqlite3_value_double(argv[1]),
                                                      sqlite3_value_double(argv[2])));
}
static void sf_weibull_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::weibull_cdf(sqlite3_value_double(argv[0]),
                                                      sqlite3_value_double(argv[1]),
                                                      sqlite3_value_double(argv[2])));
}
static void sf_weibull_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::weibull_quantile(sqlite3_value_double(argv[0]),
                                                          sqlite3_value_double(argv[1]),
                                                          sqlite3_value_double(argv[2])));
}
static void sf_weibull_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::weibull_rand(sqlite3_value_double(argv[0]),
                                                      sqlite3_value_double(argv[1])));
}

// --- P7-25..52: Discrete distributions ---

static void sf_binomial_pmf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    double p = sqlite3_value_double(argv[2]);
    result_double_or_null(ctx, statcpp::binomial_pmf(k, n, p));
}
static void sf_binomial_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    double p = sqlite3_value_double(argv[2]);
    result_double_or_null(ctx, statcpp::binomial_cdf(k, n, p));
}
static void sf_binomial_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double prob = sqlite3_value_double(argv[0]);
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    double p = sqlite3_value_double(argv[2]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::binomial_quantile(prob, n, p)));
}
static void sf_binomial_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double p = sqlite3_value_double(argv[1]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::binomial_rand(n, p)));
}

static void sf_poisson_pmf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double lambda = sqlite3_value_double(argv[1]);
    result_double_or_null(ctx, statcpp::poisson_pmf(k, lambda));
}
static void sf_poisson_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double lambda = sqlite3_value_double(argv[1]);
    result_double_or_null(ctx, statcpp::poisson_cdf(k, lambda));
}
static void sf_poisson_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    double lambda = sqlite3_value_double(argv[1]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::poisson_quantile(p, lambda)));
}
static void sf_poisson_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double lambda = sqlite3_value_double(argv[0]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::poisson_rand(lambda)));
}

static void sf_geometric_pmf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double p = sqlite3_value_double(argv[1]);
    result_double_or_null(ctx, statcpp::geometric_pmf(k, p));
}
static void sf_geometric_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double p = sqlite3_value_double(argv[1]);
    result_double_or_null(ctx, statcpp::geometric_cdf(k, p));
}
static void sf_geometric_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double prob = sqlite3_value_double(argv[0]);
    double p = sqlite3_value_double(argv[1]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::geometric_quantile(prob, p)));
}
static void sf_geometric_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::geometric_rand(p)));
}

static void sf_nbinom_pmf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double r = sqlite3_value_double(argv[1]);
    double p = sqlite3_value_double(argv[2]);
    result_double_or_null(ctx, statcpp::nbinom_pmf(k, r, p));
}
static void sf_nbinom_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double r = sqlite3_value_double(argv[1]);
    double p = sqlite3_value_double(argv[2]);
    result_double_or_null(ctx, statcpp::nbinom_cdf(k, r, p));
}
static void sf_nbinom_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double prob = sqlite3_value_double(argv[0]);
    double r = sqlite3_value_double(argv[1]);
    double p = sqlite3_value_double(argv[2]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::nbinom_quantile(prob, r, p)));
}
static void sf_nbinom_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double r = sqlite3_value_double(argv[0]);
    double p = sqlite3_value_double(argv[1]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::nbinom_rand(r, p)));
}

static void sf_hypergeom_pmf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto N = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    auto K = static_cast<std::uint64_t>(sqlite3_value_int64(argv[2]));
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[3]));
    result_double_or_null(ctx, statcpp::hypergeom_pmf(k, N, K, n));
}
static void sf_hypergeom_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto N = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    auto K = static_cast<std::uint64_t>(sqlite3_value_int64(argv[2]));
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[3]));
    result_double_or_null(ctx, statcpp::hypergeom_cdf(k, N, K, n));
}
static void sf_hypergeom_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    auto N = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    auto K = static_cast<std::uint64_t>(sqlite3_value_int64(argv[2]));
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[3]));
    result_int64(ctx, static_cast<std::int64_t>(statcpp::hypergeom_quantile(p, N, K, n)));
}
static void sf_hypergeom_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto N = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto K = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[2]));
    result_int64(ctx, static_cast<std::int64_t>(statcpp::hypergeom_rand(N, K, n)));
}

static void sf_bernoulli_pmf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double p = sqlite3_value_double(argv[1]);
    result_double_or_null(ctx, statcpp::bernoulli_pmf(k, p));
}
static void sf_bernoulli_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    double p = sqlite3_value_double(argv[1]);
    result_double_or_null(ctx, statcpp::bernoulli_cdf(k, p));
}
static void sf_bernoulli_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double prob = sqlite3_value_double(argv[0]);
    double p = sqlite3_value_double(argv[1]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::bernoulli_quantile(prob, p)));
}
static void sf_bernoulli_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::bernoulli_rand(p)));
}

static void sf_duniform_pmf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::int64_t>(sqlite3_value_int64(argv[0]));
    auto a = static_cast<std::int64_t>(sqlite3_value_int64(argv[1]));
    auto b = static_cast<std::int64_t>(sqlite3_value_int64(argv[2]));
    result_double_or_null(ctx, statcpp::discrete_uniform_pmf(k, a, b));
}
static void sf_duniform_cdf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto k = static_cast<std::int64_t>(sqlite3_value_int64(argv[0]));
    auto a = static_cast<std::int64_t>(sqlite3_value_int64(argv[1]));
    auto b = static_cast<std::int64_t>(sqlite3_value_int64(argv[2]));
    result_double_or_null(ctx, statcpp::discrete_uniform_cdf(k, a, b));
}
static void sf_duniform_quantile(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p = sqlite3_value_double(argv[0]);
    auto a = static_cast<std::int64_t>(sqlite3_value_int64(argv[1]));
    auto b = static_cast<std::int64_t>(sqlite3_value_int64(argv[2]));
    result_int64(ctx, static_cast<std::int64_t>(statcpp::discrete_uniform_quantile(p, a, b)));
}
static void sf_duniform_rand(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto a = static_cast<std::int64_t>(sqlite3_value_int64(argv[0]));
    auto b = static_cast<std::int64_t>(sqlite3_value_int64(argv[1]));
    result_int64(ctx, static_cast<std::int64_t>(statcpp::discrete_uniform_rand(a, b)));
}

// --- P7-53..55: Combinatorics ---
static void sf_binomial_coef(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    result_double_or_null(ctx, statcpp::binomial_coef(n, k));
}
static void sf_log_binomial_coef(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    auto k = static_cast<std::uint64_t>(sqlite3_value_int64(argv[1]));
    result_double_or_null(ctx, statcpp::log_binomial_coef(n, k));
}
static void sf_log_factorial(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto n = static_cast<std::uint64_t>(sqlite3_value_int64(argv[0]));
    result_double_or_null(ctx, statcpp::log_factorial(n));
}

// --- P7-56..61: Special functions ---
static void sf_lgamma(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::lgamma(sqlite3_value_double(argv[0])));
}
static void sf_tgamma(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::tgamma(sqlite3_value_double(argv[0])));
}
static void sf_beta_func(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::beta(sqlite3_value_double(argv[0]),
                                              sqlite3_value_double(argv[1])));
}
static void sf_lbeta(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::lbeta(sqlite3_value_double(argv[0]),
                                               sqlite3_value_double(argv[1])));
}
static void sf_erf(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::erf(sqlite3_value_double(argv[0])));
}
static void sf_erfc(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::erfc(sqlite3_value_double(argv[0])));
}

// --- P7-62: Logarithmic mean ---
static void sf_logarithmic_mean(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double a = sqlite3_value_double(argv[0]);
    double b = sqlite3_value_double(argv[1]);
    double result;
    if (a <= 0.0 || b <= 0.0) { sqlite3_result_null(ctx); return; }
    if (std::abs(a - b) < 1e-15) { result = a; }
    else { result = (b - a) / (std::log(b) - std::log(a)); }
    result_double_or_null(ctx, result);
}

// --- P7-63: Hedges' correction factor J ---
static void sf_hedges_j(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double df = sqlite3_value_double(argv[0]);
    result_double_or_null(ctx, statcpp::hedges_correction_factor(df));
}

// --- P7-64..66: Effect size conversions ---
static void sf_t_to_r(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::t_to_r(sqlite3_value_double(argv[0]),
                                                sqlite3_value_double(argv[1])));
}
static void sf_d_to_r(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::d_to_r(sqlite3_value_double(argv[0])));
}
static void sf_r_to_d(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::r_to_d(sqlite3_value_double(argv[0])));
}

// --- P7-67..69: Effect sizes from ANOVA components ---
static void sf_eta_squared_ef(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::eta_squared(sqlite3_value_double(argv[0]),
                                                     sqlite3_value_double(argv[1])));
}
static void sf_partial_eta_sq(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::partial_eta_squared(sqlite3_value_double(argv[0]),
                                                             sqlite3_value_double(argv[1]),
                                                             sqlite3_value_double(argv[2])));
}
static void sf_omega_squared_ef(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::omega_squared(sqlite3_value_double(argv[0]),
                                                       sqlite3_value_double(argv[1]),
                                                       sqlite3_value_double(argv[2]),
                                                       sqlite3_value_double(argv[3])));
}

// --- P7-70: Cohen's h ---
static void sf_cohens_h(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    result_double_or_null(ctx, statcpp::cohens_h(sqlite3_value_double(argv[0]),
                                                  sqlite3_value_double(argv[1])));
}

// --- P7-71..73: Effect size interpretation ---
static const char* magnitude_to_string(statcpp::effect_size_magnitude m) {
    switch (m) {
        case statcpp::effect_size_magnitude::negligible: return "negligible";
        case statcpp::effect_size_magnitude::small:      return "small";
        case statcpp::effect_size_magnitude::medium:     return "medium";
        case statcpp::effect_size_magnitude::large:      return "large";
    }
    return "unknown";
}

static void sf_interpret_d(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto m = statcpp::interpret_cohens_d(sqlite3_value_double(argv[0]));
    const char* s = magnitude_to_string(m);
    sqlite3_result_text(ctx, s, -1, SQLITE_STATIC);
}
static void sf_interpret_r(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto m = statcpp::interpret_correlation(sqlite3_value_double(argv[0]));
    const char* s = magnitude_to_string(m);
    sqlite3_result_text(ctx, s, -1, SQLITE_STATIC);
}
static void sf_interpret_eta2(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    auto m = statcpp::interpret_eta_squared(sqlite3_value_double(argv[0]));
    const char* s = magnitude_to_string(m);
    sqlite3_result_text(ctx, s, -1, SQLITE_STATIC);
}

// --- P7-74..79: Power analysis ---
static void sf_power_t1(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double es = sqlite3_value_double(argv[0]);
    auto n = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    double alpha = sqlite3_value_double(argv[2]);
    result_double_or_null(ctx, statcpp::power_t_test_one_sample(es, n, alpha));
}
static void sf_n_t1(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double es = sqlite3_value_double(argv[0]);
    double power = sqlite3_value_double(argv[1]);
    double alpha = sqlite3_value_double(argv[2]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::sample_size_t_test_one_sample(es, power, alpha)));
}
static void sf_power_t2(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double es = sqlite3_value_double(argv[0]);
    auto n1 = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    auto n2 = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    double alpha = sqlite3_value_double(argv[3]);
    result_double_or_null(ctx, statcpp::power_t_test_two_sample(es, n1, n2, alpha));
}
static void sf_n_t2(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double es = sqlite3_value_double(argv[0]);
    double power = sqlite3_value_double(argv[1]);
    double alpha = sqlite3_value_double(argv[2]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::sample_size_t_test_two_sample(es, power, alpha)));
}
static void sf_power_prop(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p1 = sqlite3_value_double(argv[0]);
    double p2 = sqlite3_value_double(argv[1]);
    auto n = static_cast<std::size_t>(sqlite3_value_int64(argv[2]));
    double alpha = sqlite3_value_double(argv[3]);
    result_double_or_null(ctx, statcpp::power_prop_test(p1, p2, n, alpha));
}
static void sf_n_prop(sqlite3_context* ctx, int /*argc*/, sqlite3_value** argv) {
    double p1 = sqlite3_value_double(argv[0]);
    double p2 = sqlite3_value_double(argv[1]);
    double power = sqlite3_value_double(argv[2]);
    double alpha = sqlite3_value_double(argv[3]);
    result_int64(ctx, static_cast<std::int64_t>(statcpp::sample_size_prop_test(p1, p2, power, alpha)));
}

// --- P7-80..83: Margin of error / sample size estimation ---
static void sf_moe_prop(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    auto succ = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    auto n = static_cast<std::size_t>(sqlite3_value_int64(argv[1]));
    double conf = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 0.95;
    result_double_or_null(ctx, statcpp::margin_of_error_proportion(succ, n, conf));
}
static void sf_moe_prop_worst(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    auto n = static_cast<std::size_t>(sqlite3_value_int64(argv[0]));
    double conf = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.95;
    result_double_or_null(ctx, statcpp::margin_of_error_proportion_worst_case(n, conf));
}
static void sf_n_moe_prop(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double moe = sqlite3_value_double(argv[0]);
    double conf = (argc > 1 && sqlite3_value_type(argv[1]) != SQLITE_NULL) ? sqlite3_value_double(argv[1]) : 0.95;
    double p_est = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 0.5;
    result_int64(ctx, static_cast<std::int64_t>(statcpp::sample_size_for_moe_proportion(moe, conf, p_est)));
}
static void sf_n_moe_mean(sqlite3_context* ctx, int argc, sqlite3_value** argv) {
    double moe = sqlite3_value_double(argv[0]);
    double sigma = sqlite3_value_double(argv[1]);
    double conf = (argc > 2 && sqlite3_value_type(argv[2]) != SQLITE_NULL) ? sqlite3_value_double(argv[2]) : 0.95;
    result_int64(ctx, static_cast<std::int64_t>(statcpp::sample_size_for_moe_mean(moe, sigma, conf)));
}

// ===========================================================================
// Entry point — register all functions
// ===========================================================================

extern "C" int sqlite3_ext_funcs_init(sqlite3* db, char** /*pzErrMsg*/,
                                       const sqlite3_api_routines* pApi) {
    SQLITE_EXTENSION_INIT2(pApi);

    int rc = SQLITE_OK;

    // --- Basic aggregates: SingleColumnAggregate (24 functions) ---

    // Basic statistics
    rc |= SingleColumnAggregate<calc_mean>::register_func(db, "stat_mean");
    rc |= SingleColumnAggregate<calc_median>::register_func(db, "stat_median");
    rc |= SingleColumnAggregate<calc_mode>::register_func(db, "stat_mode");
    rc |= SingleColumnAggregate<calc_geometric_mean>::register_func(db, "stat_geometric_mean");
    rc |= SingleColumnAggregate<calc_harmonic_mean>::register_func(db, "stat_harmonic_mean");

    // Dispersion / spread
    rc |= SingleColumnAggregate<calc_range>::register_func(db, "stat_range");
    rc |= SingleColumnAggregate<calc_var>::register_func(db, "stat_var");
    rc |= SingleColumnAggregate<calc_population_variance>::register_func(db, "stat_population_variance");
    rc |= SingleColumnAggregate<calc_sample_variance>::register_func(db, "stat_sample_variance");
    rc |= SingleColumnAggregate<calc_stdev>::register_func(db, "stat_stdev");
    rc |= SingleColumnAggregate<calc_population_stddev>::register_func(db, "stat_population_stddev");
    rc |= SingleColumnAggregate<calc_sample_stddev>::register_func(db, "stat_sample_stddev");
    rc |= SingleColumnAggregate<calc_cv>::register_func(db, "stat_cv");
    rc |= SingleColumnAggregate<calc_iqr>::register_func(db, "stat_iqr");
    rc |= SingleColumnAggregate<calc_mad_mean>::register_func(db, "stat_mad_mean");
    rc |= SingleColumnAggregate<calc_geometric_stddev>::register_func(db, "stat_geometric_stddev");

    // Shape of distribution
    rc |= SingleColumnAggregate<calc_population_skewness>::register_func(db, "stat_population_skewness");
    rc |= SingleColumnAggregate<calc_skewness>::register_func(db, "stat_skewness");
    rc |= SingleColumnAggregate<calc_population_kurtosis>::register_func(db, "stat_population_kurtosis");
    rc |= SingleColumnAggregate<calc_kurtosis>::register_func(db, "stat_kurtosis");

    // Estimation
    rc |= SingleColumnAggregate<calc_se>::register_func(db, "stat_se");

    // Robust statistics
    rc |= SingleColumnAggregate<calc_mad>::register_func(db, "stat_mad");
    rc |= SingleColumnAggregate<calc_mad_scaled>::register_func(db, "stat_mad_scaled");
    rc |= SingleColumnAggregate<calc_hodges_lehmann>::register_func(db, "stat_hodges_lehmann");

    // --- Parameterized aggregates: SingleColumnParamAggregate (20 functions) ---

    // Basic statistics (parameterized)
    rc |= SingleColumnParamAggregate<1, calc_trimmed_mean>::register_func(db, "stat_trimmed_mean");

    // Order statistics (parameterized)
    rc |= SingleColumnParamAggregateText<0, calc_quartile>::register_func(db, "stat_quartile");
    rc |= SingleColumnParamAggregate<1, calc_percentile>::register_func(db, "stat_percentile");

    // Parametric tests
    rc |= SingleColumnParamAggregateText<2, calc_z_test>::register_func(db, "stat_z_test");
    rc |= SingleColumnParamAggregateText<1, calc_t_test>::register_func(db, "stat_t_test");
    rc |= SingleColumnParamAggregateText<0, calc_chisq_gof_uniform>::register_func(db, "stat_chisq_gof_uniform");

    // Nonparametric tests
    rc |= SingleColumnParamAggregateText<0, calc_shapiro_wilk>::register_func(db, "stat_shapiro_wilk");
    rc |= SingleColumnParamAggregateText<0, calc_ks_test>::register_func(db, "stat_ks_test");
    rc |= SingleColumnParamAggregateText<1, calc_wilcoxon>::register_func(db, "stat_wilcoxon");

    // Estimation
    rc |= SingleColumnParamAggregateText<1, calc_ci_mean>::register_func(db, "stat_ci_mean");
    rc |= SingleColumnParamAggregateText<2, calc_ci_mean_z>::register_func(db, "stat_ci_mean_z");
    rc |= SingleColumnParamAggregateText<1, calc_ci_var>::register_func(db, "stat_ci_var");
    rc |= SingleColumnParamAggregate<1, calc_moe_mean>::register_func(db, "stat_moe_mean");

    // Effect size
    rc |= SingleColumnParamAggregate<1, calc_cohens_d>::register_func(db, "stat_cohens_d");
    rc |= SingleColumnParamAggregate<1, calc_hedges_g>::register_func(db, "stat_hedges_g");

    // Time series
    rc |= SingleColumnParamAggregate<1, calc_acf_lag>::register_func(db, "stat_acf_lag");

    // Robust statistics (parameterized)
    rc |= SingleColumnParamAggregate<1, calc_biweight_midvar>::register_func(db, "stat_biweight_midvar");

    // Resampling
    rc |= SingleColumnParamAggregateText<1, calc_bootstrap_mean>::register_func(db, "stat_bootstrap_mean");
    rc |= SingleColumnParamAggregateText<1, calc_bootstrap_median>::register_func(db, "stat_bootstrap_median");
    rc |= SingleColumnParamAggregateText<1, calc_bootstrap_stddev>::register_func(db, "stat_bootstrap_stddev");

    // --- Two-column aggregates: TwoColumnAggregate (27 functions) ---

    // Correlation / covariance
    rc |= TwoColumnAggregate<calc_population_covariance>::register_func(db, "stat_population_covariance");
    rc |= TwoColumnAggregate<calc_covariance>::register_func(db, "stat_covariance");
    rc |= TwoColumnAggregate<calc_pearson_r>::register_func(db, "stat_pearson_r");
    rc |= TwoColumnAggregate<calc_spearman_r>::register_func(db, "stat_spearman_r");
    rc |= TwoColumnAggregate<calc_kendall_tau>::register_func(db, "stat_kendall_tau");
    rc |= TwoColumnAggregate<calc_weighted_covariance>::register_func(db, "stat_weighted_covariance");

    // Weighted statistics
    rc |= TwoColumnAggregate<calc_weighted_mean>::register_func(db, "stat_weighted_mean");
    rc |= TwoColumnAggregate<calc_weighted_harmonic_mean>::register_func(db, "stat_weighted_harmonic_mean");
    rc |= TwoColumnAggregate<calc_weighted_variance>::register_func(db, "stat_weighted_variance");
    rc |= TwoColumnAggregate<calc_weighted_stddev>::register_func(db, "stat_weighted_stddev");
    rc |= TwoColumnAggregate<calc_weighted_median>::register_func(db, "stat_weighted_median");
    rc |= TwoColumnParamAggregate<1, calc_weighted_percentile>::register_func(db, "stat_weighted_percentile");

    // Linear regression
    rc |= TwoColumnAggregateText<calc_simple_regression>::register_func(db, "stat_simple_regression");
    rc |= TwoColumnAggregate<calc_r_squared>::register_func(db, "stat_r_squared");
    rc |= TwoColumnAggregate<calc_adjusted_r_squared>::register_func(db, "stat_adjusted_r_squared");

    // Paired / two-sample tests
    rc |= TwoColumnAggregateText<calc_t_test_paired>::register_func(db, "stat_t_test_paired");
    rc |= TwoColumnAggregateText<calc_chisq_gof>::register_func(db, "stat_chisq_gof");

    // Error metrics
    rc |= TwoColumnAggregate<calc_mae>::register_func(db, "stat_mae");
    rc |= TwoColumnAggregate<calc_mse>::register_func(db, "stat_mse");
    rc |= TwoColumnAggregate<calc_rmse>::register_func(db, "stat_rmse");
    rc |= TwoColumnAggregate<calc_mape>::register_func(db, "stat_mape");

    // Distance metrics
    rc |= TwoColumnAggregate<calc_euclidean_dist>::register_func(db, "stat_euclidean_dist");
    rc |= TwoColumnAggregate<calc_manhattan_dist>::register_func(db, "stat_manhattan_dist");
    rc |= TwoColumnAggregate<calc_cosine_sim>::register_func(db, "stat_cosine_sim");
    rc |= TwoColumnAggregate<calc_cosine_dist>::register_func(db, "stat_cosine_dist");
    rc |= TwoColumnParamAggregate<1, calc_minkowski_dist>::register_func(db, "stat_minkowski_dist");
    rc |= TwoColumnAggregate<calc_chebyshev_dist>::register_func(db, "stat_chebyshev_dist");

    // --- Window functions: FullScanWindowFunction (23 functions) ---

    // Rolling statistics (2-arg: column, window_size)
    rc |= FullScanWindowFunction<wf_rolling_mean>::register_func_2(db, "stat_rolling_mean");
    rc |= FullScanWindowFunction<wf_rolling_std>::register_func_2(db, "stat_rolling_std");
    rc |= FullScanWindowFunction<wf_rolling_min>::register_func_2(db, "stat_rolling_min");
    rc |= FullScanWindowFunction<wf_rolling_max>::register_func_2(db, "stat_rolling_max");
    rc |= FullScanWindowFunction<wf_rolling_sum>::register_func_2(db, "stat_rolling_sum");

    // Moving averages (2-arg: column, window/span)
    rc |= FullScanWindowFunction<wf_moving_avg>::register_func_2(db, "stat_moving_avg");
    rc |= FullScanWindowFunction<wf_ema>::register_func_2(db, "stat_ema");

    // Rank (1-arg: column)
    rc |= FullScanWindowFunction<wf_rank>::register_func_1(db, "stat_rank");

    // Fill NA (1-arg: column)
    rc |= FullScanWindowFunction<wf_fillna_mean>::register_func_1(db, "stat_fillna_mean");
    rc |= FullScanWindowFunction<wf_fillna_median>::register_func_1(db, "stat_fillna_median");
    rc |= FullScanWindowFunction<wf_fillna_ffill>::register_func_1(db, "stat_fillna_ffill");
    rc |= FullScanWindowFunction<wf_fillna_bfill>::register_func_1(db, "stat_fillna_bfill");
    rc |= FullScanWindowFunction<wf_fillna_interp>::register_func_1(db, "stat_fillna_interp");

    // Encoding (1-arg: column or 2-arg: column, n_bins)
    rc |= FullScanWindowFunction<wf_label_encode>::register_func_1(db, "stat_label_encode");
    rc |= FullScanWindowFunction<wf_bin_width>::register_func_2(db, "stat_bin_width");
    rc |= FullScanWindowFunction<wf_bin_freq>::register_func_2(db, "stat_bin_freq");

    // Time series (2-arg: column, lag/order/period)
    rc |= FullScanWindowFunction<wf_lag>::register_func_2(db, "stat_lag");
    rc |= FullScanWindowFunction<wf_diff>::register_func_2(db, "stat_diff");
    rc |= FullScanWindowFunction<wf_seasonal_diff>::register_func_2(db, "stat_seasonal_diff");

    // Outlier detection (1-arg: column)
    rc |= FullScanWindowFunction<wf_outliers_iqr>::register_func_1(db, "stat_outliers_iqr");
    rc |= FullScanWindowFunction<wf_outliers_zscore>::register_func_1(db, "stat_outliers_zscore");
    rc |= FullScanWindowFunction<wf_outliers_mzscore>::register_func_1(db, "stat_outliers_mzscore");

    // Robust (2-arg: column, percentile)
    rc |= FullScanWindowFunction<wf_winsorize>::register_func_2(db, "stat_winsorize");

    // --- Complex aggregates: two-sample / JSON (32 functions) ---

    // Multiple result aggregates (single column → JSON)
    rc |= SingleColumnAggregateText<calc_modes>::register_func(db, "stat_modes");
    rc |= SingleColumnAggregateText<calc_five_number_summary>::register_func(db, "stat_five_number_summary");
    rc |= SingleColumnAggregateText<calc_frequency_table>::register_func(db, "stat_frequency_table");
    rc |= SingleColumnAggregateText<calc_frequency_count>::register_func(db, "stat_frequency_count");
    rc |= SingleColumnAggregateText<calc_relative_frequency>::register_func(db, "stat_relative_frequency");
    rc |= SingleColumnAggregateText<calc_cumulative_frequency>::register_func(db, "stat_cumulative_frequency");
    rc |= SingleColumnAggregateText<calc_cumulative_relative_frequency>::register_func(db, "stat_cumulative_relative_frequency");

    // Two-sample tests (two-column → JSON)
    rc |= TwoColumnAggregateText<calc_t_test2>::register_func(db, "stat_t_test2");
    rc |= TwoColumnAggregateText<calc_t_test_welch>::register_func(db, "stat_t_test_welch");
    rc |= TwoColumnAggregateText<calc_chisq_independence>::register_func(db, "stat_chisq_independence");
    rc |= TwoColumnAggregateText<calc_f_test>::register_func(db, "stat_f_test");
    rc |= TwoColumnAggregateText<calc_mann_whitney>::register_func(db, "stat_mann_whitney");

    // ANOVA (two-column: value, group → JSON)
    rc |= TwoColumnAggregateText<calc_anova1>::register_func(db, "stat_anova1");

    // Contingency table (two-column → JSON)
    rc |= TwoColumnAggregateText<calc_contingency_table>::register_func(db, "stat_contingency_table");

    // Two-sample effect sizes
    rc |= TwoColumnAggregate<calc_cohens_d2>::register_func(db, "stat_cohens_d2");
    rc |= TwoColumnAggregate<calc_hedges_g2>::register_func(db, "stat_hedges_g2");
    rc |= TwoColumnAggregate<calc_glass_delta>::register_func(db, "stat_glass_delta");

    // CI for differences (two-column → JSON)
    rc |= TwoColumnAggregateText<calc_ci_mean_diff>::register_func(db, "stat_ci_mean_diff");
    rc |= TwoColumnAggregateText<calc_ci_mean_diff_welch>::register_func(db, "stat_ci_mean_diff_welch");

    // Survival analysis (two-column: time, event → JSON)
    rc |= TwoColumnAggregateText<calc_kaplan_meier>::register_func(db, "stat_kaplan_meier");
    // Logrank test (three-column: time, event, group → JSON)
    rc |= sqlite3_create_function_v2(db, "stat_logrank", 3,
        SQLITE_UTF8 | SQLITE_DETERMINISTIC, nullptr, nullptr,
        logrank_step, logrank_final, nullptr);
    rc |= TwoColumnAggregateText<calc_nelson_aalen>::register_func(db, "stat_nelson_aalen");

    // Resampling (single column + param → JSON)
    rc |= SingleColumnParamAggregateText<1, calc_bootstrap_generic>::register_func(db, "stat_bootstrap");
    rc |= SingleColumnParamAggregateText<1, calc_bootstrap_bca>::register_func(db, "stat_bootstrap_bca");
    rc |= SingleColumnParamAggregateText<0, calc_bootstrap_sample_text>::register_func(db, "stat_bootstrap_sample");

    // Permutation tests (two-column → JSON)
    rc |= TwoColumnAggregateText<calc_permutation_test2>::register_func(db, "stat_permutation_test2");
    rc |= TwoColumnAggregateText<calc_permutation_paired>::register_func(db, "stat_permutation_paired");
    rc |= TwoColumnAggregateText<calc_permutation_corr>::register_func(db, "stat_permutation_corr");

    // ACF / PACF (single column + max_lag → JSON array)
    rc |= SingleColumnParamAggregateText<1, calc_acf>::register_func(db, "stat_acf");
    rc |= SingleColumnParamAggregateText<1, calc_pacf>::register_func(db, "stat_pacf");

    // Sampling (single column + n → JSON array)
    rc |= SingleColumnParamAggregateText<1, calc_sample_replace>::register_func(db, "stat_sample_replace");
    rc |= SingleColumnParamAggregateText<1, calc_sample_noreplace>::register_func(db, "stat_sample");

    // --- Scalar — tests/helpers (40 functions) ---

    // Normal distribution (1-3 args)
    rc |= register_scalar(db, "stat_normal_pdf", -1, sf_normal_pdf);
    rc |= register_scalar(db, "stat_normal_cdf", -1, sf_normal_cdf);
    rc |= register_scalar(db, "stat_normal_quantile", -1, sf_normal_quantile);
    rc |= register_scalar_nd(db, "stat_normal_rand", -1, sf_normal_rand);

    // Chi-square distribution (2 args)
    rc |= register_scalar(db, "stat_chisq_pdf", 2, sf_chisq_pdf);
    rc |= register_scalar(db, "stat_chisq_cdf", 2, sf_chisq_cdf);
    rc |= register_scalar(db, "stat_chisq_quantile", 2, sf_chisq_quantile);
    rc |= register_scalar_nd(db, "stat_chisq_rand", 1, sf_chisq_rand);

    // T distribution (2 args)
    rc |= register_scalar(db, "stat_t_pdf", 2, sf_t_pdf);
    rc |= register_scalar(db, "stat_t_cdf", 2, sf_t_cdf);
    rc |= register_scalar(db, "stat_t_quantile", 2, sf_t_quantile);
    rc |= register_scalar_nd(db, "stat_t_rand", 1, sf_t_rand);

    // F distribution (3 args)
    rc |= register_scalar(db, "stat_f_pdf", 3, sf_f_pdf);
    rc |= register_scalar(db, "stat_f_cdf", 3, sf_f_cdf);
    rc |= register_scalar(db, "stat_f_quantile", 3, sf_f_quantile);
    rc |= register_scalar_nd(db, "stat_f_rand", 2, sf_f_rand);

    // Special functions
    rc |= register_scalar(db, "stat_betainc", 3, sf_betainc);
    rc |= register_scalar(db, "stat_betaincinv", 3, sf_betaincinv);
    rc |= register_scalar(db, "stat_norm_cdf", 1, sf_norm_cdf);
    rc |= register_scalar(db, "stat_norm_quantile", 1, sf_norm_quantile);
    rc |= register_scalar(db, "stat_gammainc_lower", 2, sf_gammainc_lower);
    rc |= register_scalar(db, "stat_gammainc_upper", 2, sf_gammainc_upper);
    rc |= register_scalar(db, "stat_gammainc_lower_inv", 2, sf_gammainc_lower_inv);

    // Proportion tests
    rc |= register_scalar(db, "stat_z_test_prop", 3, sf_z_test_prop);
    rc |= register_scalar(db, "stat_z_test_prop2", 4, sf_z_test_prop2);

    // Multiple testing corrections
    rc |= register_scalar(db, "stat_bonferroni", 2, sf_bonferroni);
    rc |= register_scalar(db, "stat_bh_correction", 3, sf_bh_correction);
    rc |= register_scalar(db, "stat_holm_correction", 3, sf_holm_correction);

    // Categorical tests
    rc |= register_scalar(db, "stat_fisher_exact", 4, sf_fisher_exact);
    rc |= register_scalar(db, "stat_odds_ratio", 4, sf_odds_ratio);
    rc |= register_scalar(db, "stat_relative_risk", 4, sf_relative_risk);
    rc |= register_scalar(db, "stat_risk_difference", 4, sf_risk_difference);
    rc |= register_scalar(db, "stat_nnt", 4, sf_nnt);

    // Proportion CIs
    rc |= register_scalar(db, "stat_ci_prop", -1, sf_ci_prop);
    rc |= register_scalar(db, "stat_ci_prop_wilson", -1, sf_ci_prop_wilson);
    rc |= register_scalar(db, "stat_ci_prop_diff", -1, sf_ci_prop_diff);

    // Model selection
    rc |= register_scalar(db, "stat_aic", 2, sf_aic);
    rc |= register_scalar(db, "stat_aicc", 3, sf_aicc);
    rc |= register_scalar(db, "stat_bic", 3, sf_bic);

    // Data transformation
    rc |= register_scalar(db, "stat_boxcox", 2, sf_boxcox);

    // --- Scalar — distributions/transforms (83 functions) ---

    // Uniform distribution
    rc |= register_scalar(db, "stat_uniform_pdf", -1, sf_uniform_pdf);
    rc |= register_scalar(db, "stat_uniform_cdf", -1, sf_uniform_cdf);
    rc |= register_scalar(db, "stat_uniform_quantile", -1, sf_uniform_quantile);
    rc |= register_scalar_nd(db, "stat_uniform_rand", -1, sf_uniform_rand);

    // Exponential distribution
    rc |= register_scalar(db, "stat_exponential_pdf", -1, sf_exponential_pdf);
    rc |= register_scalar(db, "stat_exponential_cdf", -1, sf_exponential_cdf);
    rc |= register_scalar(db, "stat_exponential_quantile", -1, sf_exponential_quantile);
    rc |= register_scalar_nd(db, "stat_exponential_rand", -1, sf_exponential_rand);

    // Gamma distribution
    rc |= register_scalar(db, "stat_gamma_pdf", 3, sf_gamma_pdf);
    rc |= register_scalar(db, "stat_gamma_cdf", 3, sf_gamma_cdf);
    rc |= register_scalar(db, "stat_gamma_quantile", 3, sf_gamma_quantile);
    rc |= register_scalar_nd(db, "stat_gamma_rand", 2, sf_gamma_rand);

    // Beta distribution
    rc |= register_scalar(db, "stat_beta_pdf", 3, sf_beta_pdf);
    rc |= register_scalar(db, "stat_beta_cdf", 3, sf_beta_cdf);
    rc |= register_scalar(db, "stat_beta_quantile", 3, sf_beta_quantile);
    rc |= register_scalar_nd(db, "stat_beta_rand", 2, sf_beta_rand);

    // Log-normal distribution
    rc |= register_scalar(db, "stat_lognormal_pdf", -1, sf_lognormal_pdf);
    rc |= register_scalar(db, "stat_lognormal_cdf", -1, sf_lognormal_cdf);
    rc |= register_scalar(db, "stat_lognormal_quantile", -1, sf_lognormal_quantile);
    rc |= register_scalar_nd(db, "stat_lognormal_rand", -1, sf_lognormal_rand);

    // Weibull distribution
    rc |= register_scalar(db, "stat_weibull_pdf", 3, sf_weibull_pdf);
    rc |= register_scalar(db, "stat_weibull_cdf", 3, sf_weibull_cdf);
    rc |= register_scalar(db, "stat_weibull_quantile", 3, sf_weibull_quantile);
    rc |= register_scalar_nd(db, "stat_weibull_rand", 2, sf_weibull_rand);

    // Binomial distribution
    rc |= register_scalar(db, "stat_binomial_pmf", 3, sf_binomial_pmf);
    rc |= register_scalar(db, "stat_binomial_cdf", 3, sf_binomial_cdf);
    rc |= register_scalar(db, "stat_binomial_quantile", 3, sf_binomial_quantile);
    rc |= register_scalar_nd(db, "stat_binomial_rand", 2, sf_binomial_rand);

    // Poisson distribution
    rc |= register_scalar(db, "stat_poisson_pmf", 2, sf_poisson_pmf);
    rc |= register_scalar(db, "stat_poisson_cdf", 2, sf_poisson_cdf);
    rc |= register_scalar(db, "stat_poisson_quantile", 2, sf_poisson_quantile);
    rc |= register_scalar_nd(db, "stat_poisson_rand", 1, sf_poisson_rand);

    // Geometric distribution
    rc |= register_scalar(db, "stat_geometric_pmf", 2, sf_geometric_pmf);
    rc |= register_scalar(db, "stat_geometric_cdf", 2, sf_geometric_cdf);
    rc |= register_scalar(db, "stat_geometric_quantile", 2, sf_geometric_quantile);
    rc |= register_scalar_nd(db, "stat_geometric_rand", 1, sf_geometric_rand);

    // Negative binomial distribution
    rc |= register_scalar(db, "stat_nbinom_pmf", 3, sf_nbinom_pmf);
    rc |= register_scalar(db, "stat_nbinom_cdf", 3, sf_nbinom_cdf);
    rc |= register_scalar(db, "stat_nbinom_quantile", 3, sf_nbinom_quantile);
    rc |= register_scalar_nd(db, "stat_nbinom_rand", 2, sf_nbinom_rand);

    // Hypergeometric distribution
    rc |= register_scalar(db, "stat_hypergeom_pmf", 4, sf_hypergeom_pmf);
    rc |= register_scalar(db, "stat_hypergeom_cdf", 4, sf_hypergeom_cdf);
    rc |= register_scalar(db, "stat_hypergeom_quantile", 4, sf_hypergeom_quantile);
    rc |= register_scalar_nd(db, "stat_hypergeom_rand", 3, sf_hypergeom_rand);

    // Bernoulli distribution
    rc |= register_scalar(db, "stat_bernoulli_pmf", 2, sf_bernoulli_pmf);
    rc |= register_scalar(db, "stat_bernoulli_cdf", 2, sf_bernoulli_cdf);
    rc |= register_scalar(db, "stat_bernoulli_quantile", 2, sf_bernoulli_quantile);
    rc |= register_scalar_nd(db, "stat_bernoulli_rand", 1, sf_bernoulli_rand);

    // Discrete uniform distribution
    rc |= register_scalar(db, "stat_duniform_pmf", 3, sf_duniform_pmf);
    rc |= register_scalar(db, "stat_duniform_cdf", 3, sf_duniform_cdf);
    rc |= register_scalar(db, "stat_duniform_quantile", 3, sf_duniform_quantile);
    rc |= register_scalar_nd(db, "stat_duniform_rand", 2, sf_duniform_rand);

    // Combinatorics
    rc |= register_scalar(db, "stat_binomial_coef", 2, sf_binomial_coef);
    rc |= register_scalar(db, "stat_log_binomial_coef", 2, sf_log_binomial_coef);
    rc |= register_scalar(db, "stat_log_factorial", 1, sf_log_factorial);

    // Special functions
    rc |= register_scalar(db, "stat_lgamma", 1, sf_lgamma);
    rc |= register_scalar(db, "stat_tgamma", 1, sf_tgamma);
    rc |= register_scalar(db, "stat_beta_func", 2, sf_beta_func);
    rc |= register_scalar(db, "stat_lbeta", 2, sf_lbeta);
    rc |= register_scalar(db, "stat_erf", 1, sf_erf);
    rc |= register_scalar(db, "stat_erfc", 1, sf_erfc);

    // Basic statistics (scalar)
    rc |= register_scalar(db, "stat_logarithmic_mean", 2, sf_logarithmic_mean);

    // Effect size corrections/conversions
    rc |= register_scalar(db, "stat_hedges_j", 1, sf_hedges_j);
    rc |= register_scalar(db, "stat_t_to_r", 2, sf_t_to_r);
    rc |= register_scalar(db, "stat_d_to_r", 1, sf_d_to_r);
    rc |= register_scalar(db, "stat_r_to_d", 1, sf_r_to_d);
    rc |= register_scalar(db, "stat_eta_squared_ef", 2, sf_eta_squared_ef);
    rc |= register_scalar(db, "stat_partial_eta_sq", 3, sf_partial_eta_sq);
    rc |= register_scalar(db, "stat_omega_squared_ef", 4, sf_omega_squared_ef);
    rc |= register_scalar(db, "stat_cohens_h", 2, sf_cohens_h);

    // Effect size interpretation
    rc |= register_scalar(db, "stat_interpret_d", 1, sf_interpret_d);
    rc |= register_scalar(db, "stat_interpret_r", 1, sf_interpret_r);
    rc |= register_scalar(db, "stat_interpret_eta2", 1, sf_interpret_eta2);

    // Power analysis
    rc |= register_scalar(db, "stat_power_t1", 3, sf_power_t1);
    rc |= register_scalar(db, "stat_n_t1", 3, sf_n_t1);
    rc |= register_scalar(db, "stat_power_t2", 4, sf_power_t2);
    rc |= register_scalar(db, "stat_n_t2", 3, sf_n_t2);
    rc |= register_scalar(db, "stat_power_prop", 4, sf_power_prop);
    rc |= register_scalar(db, "stat_n_prop", 4, sf_n_prop);

    // Margin of error / sample size
    rc |= register_scalar(db, "stat_moe_prop", -1, sf_moe_prop);
    rc |= register_scalar(db, "stat_moe_prop_worst", -1, sf_moe_prop_worst);
    rc |= register_scalar(db, "stat_n_moe_prop", -1, sf_n_moe_prop);
    rc |= register_scalar(db, "stat_n_moe_mean", -1, sf_n_moe_mean);

    return rc == SQLITE_OK ? SQLITE_OK : SQLITE_ERROR;
}
