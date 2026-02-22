#include <iostream>
#include <string>
#include <cstdlib>
#include <sqlite3.h>

// Helper to get the extension library path
static std::string get_extension_path() {
#ifdef EXT_FUNCS_PATH
    return EXT_FUNCS_PATH;
#else
#if defined(__APPLE__)
    return "./ext_funcs.dylib";
#elif defined(_WIN32)
    return "./ext_funcs.dll";
#else
    return "./ext_funcs.so";
#endif
#endif
}

// Callback to print SELECT results
static int print_callback(void* /*unused*/, int ncols, char** values, char** names) {
    std::cout << "  ";
    for (int i = 0; i < ncols; i++) {
        if (i > 0) std::cout << " | ";
        std::cout << names[i] << "=" << (values[i] ? values[i] : "NULL");
    }
    std::cout << "\n";
    return 0;
}

// Helper to execute SQL and display results
static bool exec_sql(sqlite3* db, const char* sql, const char* label = nullptr) {
    if (label) {
        std::cout << label << "\n";
        std::cout << "  SQL: " << sql << "\n";
    }
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql, print_callback, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::cerr << "  [ERROR] " << errmsg << "\n";
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}

int main() {
    std::cout << "=== SQLite3 Statistical Functions Test (249 functions) ===\n";
    std::cout << "SQLite3 version: " << sqlite3_libversion() << "\n\n";

    // Open in-memory database
    sqlite3* db = nullptr;
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }
    std::cout << "[OK] Database opened (in-memory)\n";

    // Enable and load extension
    rc = sqlite3_enable_load_extension(db, 1);
    if (rc != SQLITE_OK) {
        std::cerr << "[FAIL] sqlite3_enable_load_extension: " << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return 1;
    }

    std::string ext_path = get_extension_path();
    char* load_errmsg = nullptr;
    rc = sqlite3_load_extension(db, ext_path.c_str(), "sqlite3_ext_funcs_init", &load_errmsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[FAIL] Load extension: " << (load_errmsg ? load_errmsg : "unknown") << "\n";
        sqlite3_free(load_errmsg);
        sqlite3_close(db);
        return 1;
    }
    std::cout << "[OK] Extension loaded: " << ext_path << "\n";

    // ======================================================
    // Prepare test data
    // ======================================================
    std::cout << "\n--- Prepare test data ---\n";

    exec_sql(db, "CREATE TABLE data (id INTEGER PRIMARY KEY, val REAL);", nullptr);

    // Test dataset: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    // Well-known values for verification:
    //   mean = 5.5, median = 5.5, mode = 1 (all unique, returns smallest)
    //   range = 9, population_variance = 8.25, sample_variance = 9.1667
    const char* sql_insert = "INSERT INTO data (val) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, sql_insert, -1, &stmt, nullptr);
    for (int i = 1; i <= 10; i++) {
        sqlite3_bind_double(stmt, 1, static_cast<double>(i));
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
    std::cout << "[OK] Inserted 10 rows (1..10) into 'data'\n";

    // Additional table with repeated values for mode test
    exec_sql(db, "CREATE TABLE data2 (val REAL);", nullptr);
    exec_sql(db, "INSERT INTO data2 VALUES (1),(2),(2),(3),(3),(3),(4),(4),(5);", nullptr);
    std::cout << "[OK] Inserted 9 rows into 'data2' (mode=3)\n";

    // Table with positive values for geometric_mean / geometric_stddev
    exec_sql(db, "CREATE TABLE pos_data (val REAL);", nullptr);
    exec_sql(db, "INSERT INTO pos_data VALUES (2),(4),(8);", nullptr);
    std::cout << "[OK] Inserted 3 rows into 'pos_data'\n";

    // Table with NULL values to test NULL handling
    exec_sql(db, "CREATE TABLE nulldata (val REAL);", nullptr);
    exec_sql(db, "INSERT INTO nulldata VALUES (1),(NULL),(3),(NULL),(5);", nullptr);
    std::cout << "[OK] Inserted 5 rows (2 NULLs) into 'nulldata'\n";

    // ======================================================
    // Test Basic Aggregates Functions: Basic Statistics
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Basic Aggregates: Basic Statistics\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_mean(val) AS result FROM data;",
        "\n[1] stat_mean — expected: 5.5");

    exec_sql(db,
        "SELECT stat_median(val) AS result FROM data;",
        "\n[2] stat_median — expected: 5.5");

    exec_sql(db,
        "SELECT stat_mode(val) AS result FROM data2;",
        "\n[3] stat_mode — expected: 3.0");

    exec_sql(db,
        "SELECT stat_geometric_mean(val) AS result FROM pos_data;",
        "\n[4] stat_geometric_mean(2,4,8) — expected: 4.0");

    exec_sql(db,
        "SELECT stat_harmonic_mean(val) AS result FROM pos_data;",
        "\n[5] stat_harmonic_mean(2,4,8) — expected: ~3.4286");

    // ======================================================
    // Test Basic Aggregates Functions: Dispersion / Spread
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Basic Aggregates: Dispersion / Spread\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_range(val) AS result FROM data;",
        "\n[6] stat_range — expected: 9.0");

    exec_sql(db,
        "SELECT stat_var(val) AS result FROM data;",
        "\n[7] stat_var (ddof=0) — expected: 8.25");

    exec_sql(db,
        "SELECT stat_population_variance(val) AS result FROM data;",
        "\n[8] stat_population_variance — expected: 8.25");

    exec_sql(db,
        "SELECT stat_sample_variance(val) AS result FROM data;",
        "\n[9] stat_sample_variance — expected: ~9.1667");

    exec_sql(db,
        "SELECT stat_stdev(val) AS result FROM data;",
        "\n[10] stat_stdev (ddof=0) — expected: ~2.8723");

    exec_sql(db,
        "SELECT stat_population_stddev(val) AS result FROM data;",
        "\n[11] stat_population_stddev — expected: ~2.8723");

    exec_sql(db,
        "SELECT stat_sample_stddev(val) AS result FROM data;",
        "\n[12] stat_sample_stddev — expected: ~3.0277");

    exec_sql(db,
        "SELECT stat_cv(val) AS result FROM data;",
        "\n[13] stat_cv — expected: ~0.5505");

    exec_sql(db,
        "SELECT stat_iqr(val) AS result FROM data;",
        "\n[14] stat_iqr — expected: 4.5");

    exec_sql(db,
        "SELECT stat_mad_mean(val) AS result FROM data;",
        "\n[15] stat_mad_mean — expected: 2.5");

    exec_sql(db,
        "SELECT stat_geometric_stddev(val) AS result FROM pos_data;",
        "\n[16] stat_geometric_stddev(2,4,8)");

    // ======================================================
    // Test Basic Aggregates Functions: Shape of Distribution
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Basic Aggregates: Shape of Distribution\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_population_skewness(val) AS result FROM data;",
        "\n[17] stat_population_skewness — expected: 0.0 (symmetric)");

    exec_sql(db,
        "SELECT stat_skewness(val) AS result FROM data;",
        "\n[18] stat_skewness (sample) — expected: 0.0 (symmetric)");

    exec_sql(db,
        "SELECT stat_population_kurtosis(val) AS result FROM data;",
        "\n[19] stat_population_kurtosis — expected: ~-1.2242 (excess)");

    exec_sql(db,
        "SELECT stat_kurtosis(val) AS result FROM data;",
        "\n[20] stat_kurtosis (sample) — expected: ~-1.2000");

    // ======================================================
    // Test Basic Aggregates Functions: Estimation
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Basic Aggregates: Estimation\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_se(val) AS result FROM data;",
        "\n[21] stat_se — expected: ~0.9574");

    // ======================================================
    // Test Basic Aggregates Functions: Robust Statistics
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Basic Aggregates: Robust Statistics\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_mad(val) AS result FROM data;",
        "\n[22] stat_mad — expected: 2.5");

    exec_sql(db,
        "SELECT stat_mad_scaled(val) AS result FROM data;",
        "\n[23] stat_mad_scaled — expected: ~3.7065");

    exec_sql(db,
        "SELECT stat_hodges_lehmann(val) AS result FROM data;",
        "\n[24] stat_hodges_lehmann — expected: 5.5");

    // ======================================================
    // Test NULL handling
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  NULL handling tests\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_mean(val) AS mean_with_nulls FROM nulldata;",
        "\n[25] stat_mean with NULLs — expected: 3.0 (NULLs ignored)");

    exec_sql(db,
        "SELECT stat_median(val) AS median_with_nulls FROM nulldata;",
        "\n[26] stat_median with NULLs — expected: 3.0");

    // All-NULL case
    exec_sql(db,
        "SELECT stat_mean(NULL) AS all_null;",
        "\n[27] stat_mean(NULL) — expected: NULL");

    // Empty result set
    exec_sql(db,
        "SELECT stat_mean(val) AS empty_result FROM data WHERE val > 100;",
        "\n[28] stat_mean on empty result — expected: NULL");

    // ======================================================
    // Test with GROUP BY
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  GROUP BY test\n";
    std::cout << "========================================\n";

    exec_sql(db, "CREATE TABLE grouped (grp TEXT, val REAL);", nullptr);
    exec_sql(db,
        "INSERT INTO grouped VALUES "
        "('A',1),('A',2),('A',3),"
        "('B',10),('B',20),('B',30);",
        nullptr);

    exec_sql(db,
        "SELECT grp, stat_mean(val) AS mean, stat_median(val) AS median, "
        "stat_sample_stddev(val) AS stddev "
        "FROM grouped GROUP BY grp;",
        "\n[29] GROUP BY — A: mean=2, median=2, stddev=1 / B: mean=20, median=20, stddev=10");

    // ======================================================
    // Test Parameterized Aggregates Functions: Basic Statistics (parameterized)
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Basic Statistics (param)\n";
    std::cout << "========================================\n";

    // trimmed_mean(1..10, 0.1) trims 10% from each end → removes 1 and 10 → mean(2..9) = 5.5
    exec_sql(db,
        "SELECT stat_trimmed_mean(val, 0.1) AS result FROM data;",
        "\n[30] stat_trimmed_mean(0.1) — expected: 5.5");

    // trimmed_mean with 20% trim → removes 1,2 and 9,10 → mean(3..8) = 5.5
    exec_sql(db,
        "SELECT stat_trimmed_mean(val, 0.2) AS result FROM data;",
        "\n[31] stat_trimmed_mean(0.2) — expected: 5.5");

    // ======================================================
    // Test Parameterized Aggregates Functions: Order Statistics (parameterized)
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Order Statistics (param)\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_quartile(val) AS result FROM data;",
        "\n[32] stat_quartile — expected: JSON {q1, q2, q3}");

    exec_sql(db,
        "SELECT stat_percentile(val, 0.5) AS result FROM data;",
        "\n[33] stat_percentile(0.5) — expected: 5.5 (median)");

    exec_sql(db,
        "SELECT stat_percentile(val, 0.25) AS result FROM data;",
        "\n[34] stat_percentile(0.25) — expected: Q1");

    exec_sql(db,
        "SELECT stat_percentile(val, 0.75) AS result FROM data;",
        "\n[35] stat_percentile(0.75) — expected: Q3");

    // ======================================================
    // Test Parameterized Aggregates Functions: Parametric Tests
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Parametric Tests\n";
    std::cout << "========================================\n";

    // z-test: H0: mu=5.5, sigma=3.0 (data mean IS 5.5, so p should be ~1.0)
    exec_sql(db,
        "SELECT stat_z_test(val, 5.5, 3.0) AS result FROM data;",
        "\n[36] stat_z_test(mu0=5.5, sigma=3.0) — expected: p~1.0 (fail to reject)");

    // z-test: H0: mu=0, sigma=3.0 (data mean=5.5, should reject)
    exec_sql(db,
        "SELECT stat_z_test(val, 0.0, 3.0) AS result FROM data;",
        "\n[37] stat_z_test(mu0=0, sigma=3.0) — expected: small p (reject)");

    // t-test: H0: mu=5.5 (data mean IS 5.5)
    exec_sql(db,
        "SELECT stat_t_test(val, 5.5) AS result FROM data;",
        "\n[38] stat_t_test(mu0=5.5) — expected: p~1.0");

    // t-test: H0: mu=0
    exec_sql(db,
        "SELECT stat_t_test(val, 0.0) AS result FROM data;",
        "\n[39] stat_t_test(mu0=0) — expected: small p (reject)");

    // chi-square goodness-of-fit (uniform): data2 has {1,2,2,3,3,3,4,4,5}
    exec_sql(db,
        "SELECT stat_chisq_gof_uniform(val) AS result FROM data2;",
        "\n[40] stat_chisq_gof_uniform — expected: JSON {statistic, p_value, df}");

    // ======================================================
    // Test Parameterized Aggregates Functions: Nonparametric Tests
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Nonparametric Tests\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_shapiro_wilk(val) AS result FROM data;",
        "\n[41] stat_shapiro_wilk — expected: JSON (uniform data, high p)");

    exec_sql(db,
        "SELECT stat_ks_test(val) AS result FROM data;",
        "\n[42] stat_ks_test — expected: JSON (KS test for normality)");

    // Wilcoxon: H0: median=5.5 (data median IS 5.5)
    exec_sql(db,
        "SELECT stat_wilcoxon(val, 5.5) AS result FROM data;",
        "\n[43] stat_wilcoxon(mu0=5.5) — expected: large p");

    // Wilcoxon: H0: median=0
    exec_sql(db,
        "SELECT stat_wilcoxon(val, 0.0) AS result FROM data;",
        "\n[44] stat_wilcoxon(mu0=0) — expected: small p (reject)");

    // ======================================================
    // Test Parameterized Aggregates Functions: Estimation
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Estimation\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_ci_mean(val, 0.95) AS result FROM data;",
        "\n[45] stat_ci_mean(0.95) — expected: JSON {lower, upper, point_estimate=5.5}");

    exec_sql(db,
        "SELECT stat_ci_mean_z(val, 3.0, 0.95) AS result FROM data;",
        "\n[46] stat_ci_mean_z(sigma=3.0, 0.95) — expected: JSON CI");

    exec_sql(db,
        "SELECT stat_ci_var(val, 0.95) AS result FROM data;",
        "\n[47] stat_ci_var(0.95) — expected: JSON CI for variance");

    exec_sql(db,
        "SELECT stat_moe_mean(val, 0.95) AS result FROM data;",
        "\n[48] stat_moe_mean(0.95) — expected: ~2.16 (margin of error)");

    // ======================================================
    // Test Parameterized Aggregates Functions: Effect Size
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Effect Size\n";
    std::cout << "========================================\n";

    // Cohen's d: (mean - mu0) / sample_stddev = (5.5 - 0) / 3.0277 ≈ 1.816
    exec_sql(db,
        "SELECT stat_cohens_d(val, 0.0) AS result FROM data;",
        "\n[49] stat_cohens_d(mu0=0) — expected: ~1.816");

    exec_sql(db,
        "SELECT stat_hedges_g(val, 0.0) AS result FROM data;",
        "\n[50] stat_hedges_g(mu0=0) — expected: ~1.716 (bias-corrected)");

    // ======================================================
    // Test Parameterized Aggregates Functions: Time Series
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Time Series\n";
    std::cout << "========================================\n";

    // Autocorrelation lag=0 should be 1.0
    exec_sql(db,
        "SELECT stat_acf_lag(val, 0) AS result FROM data;",
        "\n[51] stat_acf_lag(lag=0) — expected: 1.0");

    exec_sql(db,
        "SELECT stat_acf_lag(val, 1) AS result FROM data;",
        "\n[52] stat_acf_lag(lag=1) — expected: high positive autocorrelation");

    // ======================================================
    // Test Parameterized Aggregates Functions: Robust Statistics (parameterized)
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Robust Statistics (param)\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_biweight_midvar(val, 9.0) AS result FROM data;",
        "\n[53] stat_biweight_midvar(c=9.0) — expected: robust variance estimate");

    // ======================================================
    // Test Parameterized Aggregates Functions: Resampling
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: Resampling\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_bootstrap_mean(val, 500) AS result FROM data;",
        "\n[54] stat_bootstrap_mean(500) — expected: JSON {estimate~5.5, ...}");

    exec_sql(db,
        "SELECT stat_bootstrap_median(val, 500) AS result FROM data;",
        "\n[55] stat_bootstrap_median(500) — expected: JSON {estimate~5.5, ...}");

    exec_sql(db,
        "SELECT stat_bootstrap_stddev(val, 500) AS result FROM data;",
        "\n[56] stat_bootstrap_stddev(500) — expected: JSON {estimate~3.03, ...}");

    // ======================================================
    // Test Parameterized Aggregates: NULL and edge cases
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: NULL / edge cases\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_percentile(val, 0.5) AS result FROM nulldata;",
        "\n[57] stat_percentile with NULLs — expected: 3.0 (NULLs ignored)");

    exec_sql(db,
        "SELECT stat_t_test(val, 3.0) AS result FROM nulldata;",
        "\n[58] stat_t_test with NULLs — expected: JSON (uses 3 non-NULL values)");

    exec_sql(db,
        "SELECT stat_ci_mean(val, 0.95) AS result FROM nulldata;",
        "\n[59] stat_ci_mean with NULLs — expected: JSON CI around 3.0");

    // Empty result
    exec_sql(db,
        "SELECT stat_percentile(val, 0.5) AS result FROM data WHERE val > 100;",
        "\n[60] stat_percentile on empty result — expected: NULL");

    // ======================================================
    // Test Parameterized Aggregates: GROUP BY
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Parameterized Aggregates: GROUP BY\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT grp, stat_percentile(val, 0.5) AS median, "
        "stat_t_test(val, 0.0) AS t_test "
        "FROM grouped GROUP BY grp;",
        "\n[61] GROUP BY — percentile + t_test per group");

    // ======================================================
    // Prepare test data for Two-Column Aggregates (two-column)
    // ======================================================
    std::cout << "\n--- Prepare two-column test data ---\n";

    exec_sql(db, "CREATE TABLE xy_data (x REAL, y REAL);", nullptr);
    exec_sql(db,
        "INSERT INTO xy_data VALUES "
        "(1,2),(2,4),(3,5),(4,4),(5,5),(6,7),(7,8),(8,6),(9,11),(10,8);",
        nullptr);
    std::cout << "[OK] Inserted 10 rows into 'xy_data' (x,y pairs)\n";

    // Weighted data: values and weights
    exec_sql(db, "CREATE TABLE wt_data (val REAL, wt REAL);", nullptr);
    exec_sql(db,
        "INSERT INTO wt_data VALUES "
        "(10,1),(20,2),(30,3),(40,2),(50,1);",
        nullptr);
    std::cout << "[OK] Inserted 5 rows into 'wt_data' (val, weight)\n";

    // Actual vs predicted for error metrics
    exec_sql(db, "CREATE TABLE pred_data (actual REAL, predicted REAL);", nullptr);
    exec_sql(db,
        "INSERT INTO pred_data VALUES "
        "(3,2.5),(5,5.2),(7,6.8),(9,9.1),(11,10.5);",
        nullptr);
    std::cout << "[OK] Inserted 5 rows into 'pred_data'\n";

    // ======================================================
    // Test Two-Column Aggregates: Correlation / Covariance
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Two-Column Aggregates: Correlation / Covariance\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_population_covariance(x, y) AS result FROM xy_data;",
        "\n[62] stat_population_covariance");

    exec_sql(db,
        "SELECT stat_covariance(x, y) AS result FROM xy_data;",
        "\n[63] stat_covariance (sample)");

    exec_sql(db,
        "SELECT stat_pearson_r(x, y) AS result FROM xy_data;",
        "\n[64] stat_pearson_r — expected: positive correlation");

    exec_sql(db,
        "SELECT stat_spearman_r(x, y) AS result FROM xy_data;",
        "\n[65] stat_spearman_r");

    exec_sql(db,
        "SELECT stat_kendall_tau(x, y) AS result FROM xy_data;",
        "\n[66] stat_kendall_tau");

    exec_sql(db,
        "SELECT stat_weighted_covariance(val, wt) AS result FROM wt_data;",
        "\n[67] stat_weighted_covariance");

    // ======================================================
    // Test Two-Column Aggregates: Weighted Statistics
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Two-Column Aggregates: Weighted Statistics\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_weighted_mean(val, wt) AS result FROM wt_data;",
        "\n[68] stat_weighted_mean — expected: 30.0 (symmetric weights)");

    exec_sql(db,
        "SELECT stat_weighted_harmonic_mean(val, wt) AS result FROM wt_data;",
        "\n[69] stat_weighted_harmonic_mean");

    exec_sql(db,
        "SELECT stat_weighted_variance(val, wt) AS result FROM wt_data;",
        "\n[70] stat_weighted_variance");

    exec_sql(db,
        "SELECT stat_weighted_stddev(val, wt) AS result FROM wt_data;",
        "\n[71] stat_weighted_stddev");

    exec_sql(db,
        "SELECT stat_weighted_median(val, wt) AS result FROM wt_data;",
        "\n[72] stat_weighted_median — expected: 30.0");

    exec_sql(db,
        "SELECT stat_weighted_percentile(val, wt, 0.5) AS result FROM wt_data;",
        "\n[73] stat_weighted_percentile(0.5) — expected: ~30.0");

    // ======================================================
    // Test Two-Column Aggregates: Linear Regression
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Two-Column Aggregates: Linear Regression\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_simple_regression(x, y) AS result FROM xy_data;",
        "\n[74] stat_simple_regression — expected: JSON with intercept, slope, r_squared");

    exec_sql(db,
        "SELECT stat_r_squared(actual, predicted) AS result FROM pred_data;",
        "\n[75] stat_r_squared — expected: high R^2 (~0.99)");

    exec_sql(db,
        "SELECT stat_adjusted_r_squared(actual, predicted) AS result FROM pred_data;",
        "\n[76] stat_adjusted_r_squared");

    // ======================================================
    // Test Two-Column Aggregates: Paired / Two-sample Tests
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Two-Column Aggregates: Paired / Two-sample Tests\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_t_test_paired(x, y) AS result FROM xy_data;",
        "\n[77] stat_t_test_paired — expected: JSON {statistic, p_value, df}");

    exec_sql(db,
        "SELECT stat_chisq_gof(actual, predicted) AS result FROM pred_data;",
        "\n[78] stat_chisq_gof — expected: JSON {statistic, p_value, df}");

    // ======================================================
    // Test Two-Column Aggregates: Error Metrics
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Two-Column Aggregates: Error Metrics\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_mae(actual, predicted) AS result FROM pred_data;",
        "\n[79] stat_mae — expected: ~0.34");

    exec_sql(db,
        "SELECT stat_mse(actual, predicted) AS result FROM pred_data;",
        "\n[80] stat_mse — expected: ~0.134");

    exec_sql(db,
        "SELECT stat_rmse(actual, predicted) AS result FROM pred_data;",
        "\n[81] stat_rmse — expected: ~0.366");

    exec_sql(db,
        "SELECT stat_mape(actual, predicted) AS result FROM pred_data;",
        "\n[82] stat_mape — expected: small value");

    // ======================================================
    // Test Two-Column Aggregates: Distance Metrics
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Two-Column Aggregates: Distance Metrics\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT stat_euclidean_dist(x, y) AS result FROM xy_data;",
        "\n[83] stat_euclidean_dist");

    exec_sql(db,
        "SELECT stat_manhattan_dist(x, y) AS result FROM xy_data;",
        "\n[84] stat_manhattan_dist");

    exec_sql(db,
        "SELECT stat_cosine_sim(x, y) AS result FROM xy_data;",
        "\n[85] stat_cosine_sim — expected: close to 1.0");

    exec_sql(db,
        "SELECT stat_cosine_dist(x, y) AS result FROM xy_data;",
        "\n[86] stat_cosine_dist — expected: close to 0.0");

    exec_sql(db,
        "SELECT stat_minkowski_dist(x, y, 3.0) AS result FROM xy_data;",
        "\n[87] stat_minkowski_dist(p=3)");

    exec_sql(db,
        "SELECT stat_chebyshev_dist(x, y) AS result FROM xy_data;",
        "\n[88] stat_chebyshev_dist");

    // ======================================================
    // Prepare test data for Window Functions (window functions)
    // ======================================================
    std::cout << "\n--- Prepare window function test data ---\n";

    exec_sql(db, "CREATE TABLE ts_data (id INTEGER PRIMARY KEY, val REAL);", nullptr);
    exec_sql(db,
        "INSERT INTO ts_data VALUES "
        "(1,10),(2,20),(3,NULL),(4,40),(5,50),(6,30),(7,70),(8,NULL),(9,90),(10,100);",
        nullptr);
    std::cout << "[OK] Inserted 10 rows into 'ts_data' (with NULLs at id=3,8)\n";

    // ======================================================
    // Test Window Functions: Rolling Statistics
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Rolling Statistics\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT id, val, stat_rolling_mean(val, 3) AS rolling_mean "
        "FROM ts_data;",
        "\n[89] stat_rolling_mean(window=3)");

    exec_sql(db,
        "SELECT id, val, stat_rolling_std(val, 3) AS rolling_std "
        "FROM ts_data;",
        "\n[90] stat_rolling_std(window=3)");

    exec_sql(db,
        "SELECT id, val, stat_rolling_min(val, 3) AS rolling_min "
        "FROM ts_data;",
        "\n[91] stat_rolling_min(window=3)");

    exec_sql(db,
        "SELECT id, val, stat_rolling_max(val, 3) AS rolling_max "
        "FROM ts_data;",
        "\n[92] stat_rolling_max(window=3)");

    exec_sql(db,
        "SELECT id, val, stat_rolling_sum(val, 3) AS rolling_sum "
        "FROM ts_data;",
        "\n[93] stat_rolling_sum(window=3)");

    // ======================================================
    // Test Window Functions: Moving Averages
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Moving Averages\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT id, val, stat_moving_avg(val, 3) AS moving_avg "
        "FROM ts_data;",
        "\n[94] stat_moving_avg(window=3)");

    exec_sql(db,
        "SELECT id, val, stat_ema(val, 5) AS ema "
        "FROM ts_data;",
        "\n[95] stat_ema(span=5) — alpha=2/(5+1)=0.333");

    // ======================================================
    // Test Window Functions: Rank
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Rank\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT id, val, stat_rank(val) AS rank "
        "FROM ts_data;",
        "\n[96] stat_rank — NULLs → NaN");

    // ======================================================
    // Test Window Functions: Fill NA
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Fill NA\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT id, val, stat_fillna_mean(val) AS filled "
        "FROM ts_data;",
        "\n[97] stat_fillna_mean — NULLs replaced with mean of non-null values");

    exec_sql(db,
        "SELECT id, val, stat_fillna_median(val) AS filled "
        "FROM ts_data;",
        "\n[98] stat_fillna_median");

    exec_sql(db,
        "SELECT id, val, stat_fillna_ffill(val) AS filled "
        "FROM ts_data;",
        "\n[99] stat_fillna_ffill — forward fill");

    exec_sql(db,
        "SELECT id, val, stat_fillna_bfill(val) AS filled "
        "FROM ts_data;",
        "\n[100] stat_fillna_bfill — backward fill");

    exec_sql(db,
        "SELECT id, val, stat_fillna_interp(val) AS filled "
        "FROM ts_data;",
        "\n[101] stat_fillna_interp — linear interpolation");

    // ======================================================
    // Test Window Functions: Encoding
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Encoding\n";
    std::cout << "========================================\n";

    exec_sql(db, "CREATE TABLE cat_data (id INTEGER PRIMARY KEY, val REAL);", nullptr);
    exec_sql(db,
        "INSERT INTO cat_data VALUES (1,100),(2,200),(3,100),(4,300),(5,200),(6,100);",
        nullptr);
    std::cout << "[OK] Inserted 6 rows into 'cat_data'\n";

    exec_sql(db,
        "SELECT id, val, stat_label_encode(val) AS encoded "
        "FROM cat_data;",
        "\n[102] stat_label_encode — unique values → 0,1,2,...");

    exec_sql(db,
        "SELECT id, val, stat_bin_width(val, 3) AS bin "
        "FROM data;",
        "\n[103] stat_bin_width(n_bins=3) — equal-width binning");

    exec_sql(db,
        "SELECT id, val, stat_bin_freq(val, 3) AS bin "
        "FROM data;",
        "\n[104] stat_bin_freq(n_bins=3) — equal-frequency binning");

    // ======================================================
    // Test Window Functions: Time Series
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Time Series\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT id, val, stat_lag(val, 1) AS lagged "
        "FROM ts_data;",
        "\n[105] stat_lag(k=1) — shift by 1");

    exec_sql(db,
        "SELECT id, val, stat_diff(val, 1) AS diffed "
        "FROM ts_data;",
        "\n[106] stat_diff(order=1) — first difference");

    exec_sql(db,
        "SELECT id, val, stat_seasonal_diff(val, 3) AS seasonal "
        "FROM ts_data;",
        "\n[107] stat_seasonal_diff(period=3)");

    // ======================================================
    // Test Window Functions: Outlier Detection
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Outlier Detection\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT id, val, stat_outliers_iqr(val) AS is_outlier "
        "FROM ts_data;",
        "\n[108] stat_outliers_iqr — 1.0=outlier, 0.0=normal");

    exec_sql(db,
        "SELECT id, val, stat_outliers_zscore(val) AS is_outlier "
        "FROM ts_data;",
        "\n[109] stat_outliers_zscore");

    exec_sql(db,
        "SELECT id, val, stat_outliers_mzscore(val) AS is_outlier "
        "FROM ts_data;",
        "\n[110] stat_outliers_mzscore");

    // ======================================================
    // Test Window Functions: Robust (winsorize)
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Window Functions: Robust (winsorize)\n";
    std::cout << "========================================\n";

    exec_sql(db,
        "SELECT id, val, stat_winsorize(val, 10) AS winsorized "
        "FROM ts_data;",
        "\n[111] stat_winsorize(percentile=10) — clip at 10th/90th percentile");

    // ======================================================
    // Test Complex Aggregates: Complex Aggregates / Two-Sample Tests
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Complex Aggregates: Complex Aggregates\n";
    std::cout << "========================================\n";

    // Create survival data for kaplan_meier/nelson_aalen
    exec_sql(db, "CREATE TABLE surv_data(time REAL, event INT);", nullptr);
    exec_sql(db, "INSERT INTO surv_data VALUES(1,1),(2,0),(3,1),(4,1),(5,0),(6,1),(7,1),(8,0),(9,1),(10,1);", nullptr);

    // Create two-group data for two-sample tests
    exec_sql(db, "CREATE TABLE grp_data(val REAL, grp INT);", nullptr);
    exec_sql(db, "INSERT INTO grp_data VALUES(10,0),(12,0),(14,0),(16,0),(18,0),(20,1),(22,1),(24,1),(26,1),(28,1);", nullptr);

    // Create logrank data (time, event, group)
    exec_sql(db, "CREATE TABLE surv2(time REAL, event INT, grp INT);", nullptr);
    exec_sql(db, "INSERT INTO surv2 VALUES(1,1,0),(3,0,0),(5,1,0),(7,1,0),(9,0,0),(2,1,1),(4,1,1),(6,0,1),(8,1,1),(10,1,1);", nullptr);

    exec_sql(db,
        "SELECT stat_modes(val) FROM data;",
        "\n[112] stat_modes — all modes as JSON array");

    exec_sql(db,
        "SELECT stat_five_number_summary(val) FROM data;",
        "\n[113] stat_five_number_summary — {min,q1,median,q3,max}");

    exec_sql(db,
        "SELECT stat_frequency_table(val) FROM data2;",
        "\n[114] stat_frequency_table — full freq table as JSON");

    exec_sql(db,
        "SELECT stat_frequency_count(val) FROM data2;",
        "\n[115] stat_frequency_count — {value:count}");

    exec_sql(db,
        "SELECT stat_relative_frequency(val) FROM data2;",
        "\n[116] stat_relative_frequency");

    exec_sql(db,
        "SELECT stat_cumulative_frequency(val) FROM data2;",
        "\n[117] stat_cumulative_frequency");

    exec_sql(db,
        "SELECT stat_cumulative_relative_frequency(val) FROM data2;",
        "\n[118] stat_cumulative_relative_frequency");

    // Two-sample tests using grp_data split by group
    exec_sql(db,
        "SELECT stat_t_test2(val, grp) FROM "
        "(SELECT val, CAST(grp AS REAL) as grp FROM grp_data WHERE grp=0 "
        "UNION ALL SELECT val, CAST(grp AS REAL) FROM grp_data WHERE grp=1);",
        "\n[119] stat_t_test2 — two-sample t-test (pooled)");

    exec_sql(db,
        "SELECT stat_t_test_welch(val, grp) FROM "
        "(SELECT val, CAST(grp AS REAL) as grp FROM grp_data WHERE grp=0 "
        "UNION ALL SELECT val, CAST(grp AS REAL) FROM grp_data WHERE grp=1);",
        "\n[120] stat_t_test_welch — Welch t-test");

    exec_sql(db,
        "SELECT stat_f_test(val, grp) FROM "
        "(SELECT val, CAST(grp AS REAL) as grp FROM grp_data WHERE grp=0 "
        "UNION ALL SELECT val, CAST(grp AS REAL) FROM grp_data WHERE grp=1);",
        "\n[121] stat_f_test — F-test for variance ratio");

    exec_sql(db,
        "SELECT stat_mann_whitney(val, grp) FROM "
        "(SELECT val, CAST(grp AS REAL) as grp FROM grp_data WHERE grp=0 "
        "UNION ALL SELECT val, CAST(grp AS REAL) FROM grp_data WHERE grp=1);",
        "\n[122] stat_mann_whitney — Mann-Whitney U test");

    exec_sql(db,
        "SELECT stat_chisq_independence(x, y) FROM cat_data;",
        "\n[123] stat_chisq_independence — chi-square independence test");

    exec_sql(db,
        "SELECT stat_anova1(val, CAST(grp AS REAL)) FROM grp_data;",
        "\n[124] stat_anova1 — one-way ANOVA");

    exec_sql(db,
        "SELECT stat_contingency_table(x, y) FROM cat_data;",
        "\n[125] stat_contingency_table — 2D contingency table");

    exec_sql(db,
        "SELECT stat_cohens_d2(val, grp) FROM "
        "(SELECT val, CAST(grp AS REAL) as grp FROM grp_data WHERE grp=0 "
        "UNION ALL SELECT val, CAST(grp AS REAL) FROM grp_data WHERE grp=1);",
        "\n[126] stat_cohens_d2 — Cohen's d (two-sample)");

    exec_sql(db,
        "SELECT stat_hedges_g2(val, grp) FROM "
        "(SELECT val, CAST(grp AS REAL) as grp FROM grp_data WHERE grp=0 "
        "UNION ALL SELECT val, CAST(grp AS REAL) FROM grp_data WHERE grp=1);",
        "\n[127] stat_hedges_g2 — Hedges' g (two-sample)");

    exec_sql(db,
        "SELECT stat_glass_delta(val, grp) FROM "
        "(SELECT val, CAST(grp AS REAL) as grp FROM grp_data WHERE grp=0 "
        "UNION ALL SELECT val, CAST(grp AS REAL) FROM grp_data WHERE grp=1);",
        "\n[128] stat_glass_delta — Glass's Delta");

    exec_sql(db,
        "SELECT stat_ci_mean_diff(x, y) FROM xy_data;",
        "\n[129] stat_ci_mean_diff — CI for mean difference");

    exec_sql(db,
        "SELECT stat_ci_mean_diff_welch(x, y) FROM xy_data;",
        "\n[130] stat_ci_mean_diff_welch — CI for mean difference (Welch)");

    exec_sql(db,
        "SELECT stat_kaplan_meier(time, CAST(event AS REAL)) FROM surv_data;",
        "\n[131] stat_kaplan_meier — survival curve");

    exec_sql(db,
        "SELECT stat_logrank(time, event, grp) FROM surv2;",
        "\n[132] stat_logrank — log-rank test (3 columns)");

    exec_sql(db,
        "SELECT stat_nelson_aalen(time, CAST(event AS REAL)) FROM surv_data;",
        "\n[133] stat_nelson_aalen — cumulative hazard");

    exec_sql(db,
        "SELECT stat_bootstrap(val, 500) FROM data;",
        "\n[134] stat_bootstrap — generic bootstrap (mean)");

    exec_sql(db,
        "SELECT stat_bootstrap_bca(val, 500) FROM data;",
        "\n[135] stat_bootstrap_bca — BCa bootstrap");

    exec_sql(db,
        "SELECT stat_bootstrap_sample(val) FROM data;",
        "\n[136] stat_bootstrap_sample — single bootstrap sample");

    exec_sql(db,
        "SELECT stat_permutation_test2(x, y) FROM xy_data;",
        "\n[137] stat_permutation_test2 — two-sample permutation");

    exec_sql(db,
        "SELECT stat_permutation_paired(x, y) FROM xy_data;",
        "\n[138] stat_permutation_paired — paired permutation");

    exec_sql(db,
        "SELECT stat_permutation_corr(x, y) FROM xy_data;",
        "\n[139] stat_permutation_corr — correlation permutation");

    exec_sql(db,
        "SELECT stat_acf(val, 5) FROM data;",
        "\n[140] stat_acf(max_lag=5) — autocorrelation function");

    exec_sql(db,
        "SELECT stat_pacf(val, 5) FROM data;",
        "\n[141] stat_pacf(max_lag=5) — partial ACF");

    exec_sql(db,
        "SELECT stat_sample_replace(val, 5) FROM data;",
        "\n[142] stat_sample_replace — with replacement");

    exec_sql(db,
        "SELECT stat_sample(val, 5) FROM data;",
        "\n[143] stat_sample — without replacement");

    // ======================================================
    // Test Scalar — Tests/Helpers: Scalar Distribution Functions
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Scalar — Tests/Helpers: Scalar Functions\n";
    std::cout << "========================================\n";

    exec_sql(db, "SELECT stat_normal_pdf(0.0);",
        "\n[144] stat_normal_pdf(0) — standard normal PDF at 0");
    exec_sql(db, "SELECT stat_normal_cdf(1.96);",
        "\n[145] stat_normal_cdf(1.96) — ~0.975");
    exec_sql(db, "SELECT stat_normal_quantile(0.975);",
        "\n[146] stat_normal_quantile(0.975) — ~1.96");
    exec_sql(db, "SELECT stat_normal_rand();",
        "\n[147] stat_normal_rand — standard normal random");

    exec_sql(db, "SELECT stat_chisq_pdf(5.0, 3);",
        "\n[148] stat_chisq_pdf(5, df=3)");
    exec_sql(db, "SELECT stat_chisq_cdf(7.815, 3);",
        "\n[149] stat_chisq_cdf(7.815, df=3) — ~0.95");
    exec_sql(db, "SELECT stat_chisq_quantile(0.95, 3);",
        "\n[150] stat_chisq_quantile(0.95, df=3) — ~7.815");
    exec_sql(db, "SELECT stat_chisq_rand(5);",
        "\n[151] stat_chisq_rand(df=5)");

    exec_sql(db, "SELECT stat_t_pdf(0.0, 10);",
        "\n[152] stat_t_pdf(0, df=10)");
    exec_sql(db, "SELECT stat_t_cdf(2.228, 10);",
        "\n[153] stat_t_cdf(2.228, df=10) — ~0.975");
    exec_sql(db, "SELECT stat_t_quantile(0.975, 10);",
        "\n[154] stat_t_quantile(0.975, df=10) — ~2.228");
    exec_sql(db, "SELECT stat_t_rand(10);",
        "\n[155] stat_t_rand(df=10)");

    exec_sql(db, "SELECT stat_f_pdf(1.0, 5, 10);",
        "\n[156] stat_f_pdf(1, df1=5, df2=10)");
    exec_sql(db, "SELECT stat_f_cdf(3.33, 5, 10);",
        "\n[157] stat_f_cdf(3.33, 5, 10)");
    exec_sql(db, "SELECT stat_f_quantile(0.95, 5, 10);",
        "\n[158] stat_f_quantile(0.95, 5, 10)");
    exec_sql(db, "SELECT stat_f_rand(5, 10);",
        "\n[159] stat_f_rand(5, 10)");

    // Special functions
    exec_sql(db, "SELECT stat_betainc(2, 3, 0.5);",
        "\n[160] stat_betainc(2, 3, 0.5)");
    exec_sql(db, "SELECT stat_betaincinv(2, 3, 0.6875);",
        "\n[161] stat_betaincinv(2, 3, 0.6875) — ~0.5");
    exec_sql(db, "SELECT stat_norm_cdf(1.96);",
        "\n[162] stat_norm_cdf(1.96) — standard normal ~0.975");
    exec_sql(db, "SELECT stat_norm_quantile(0.975);",
        "\n[163] stat_norm_quantile(0.975) — ~1.96");
    exec_sql(db, "SELECT stat_gammainc_lower(2, 1);",
        "\n[164] stat_gammainc_lower(2, 1)");
    exec_sql(db, "SELECT stat_gammainc_upper(2, 1);",
        "\n[165] stat_gammainc_upper(2, 1)");
    exec_sql(db, "SELECT stat_gammainc_lower_inv(2, 0.264);",
        "\n[166] stat_gammainc_lower_inv(2, 0.264) — ~1.0");

    // Proportion tests
    exec_sql(db, "SELECT stat_z_test_prop(30, 100, 0.25);",
        "\n[167] stat_z_test_prop(30/100, p0=0.25)");
    exec_sql(db, "SELECT stat_z_test_prop2(30, 100, 40, 100);",
        "\n[168] stat_z_test_prop2(30/100 vs 40/100)");

    // Multiple testing
    exec_sql(db, "SELECT stat_bonferroni(0.03, 5);",
        "\n[169] stat_bonferroni(p=0.03, n=5) — min(0.15, 1)");
    exec_sql(db, "SELECT stat_bh_correction(0.03, 2, 5);",
        "\n[170] stat_bh_correction(p=0.03, rank=2, total=5)");
    exec_sql(db, "SELECT stat_holm_correction(0.03, 2, 5);",
        "\n[171] stat_holm_correction(p=0.03, rank=2, total=5)");

    // Categorical
    exec_sql(db, "SELECT stat_fisher_exact(10, 5, 3, 12);",
        "\n[172] stat_fisher_exact(10,5,3,12)");
    exec_sql(db, "SELECT stat_odds_ratio(10, 5, 3, 12);",
        "\n[173] stat_odds_ratio(10,5,3,12)");
    exec_sql(db, "SELECT stat_relative_risk(10, 5, 3, 12);",
        "\n[174] stat_relative_risk(10,5,3,12)");
    exec_sql(db, "SELECT stat_risk_difference(10, 5, 3, 12);",
        "\n[175] stat_risk_difference(10,5,3,12)");
    exec_sql(db, "SELECT stat_nnt(10, 5, 3, 12);",
        "\n[176] stat_nnt(10,5,3,12)");

    // Proportion CIs
    exec_sql(db, "SELECT stat_ci_prop(30, 100);",
        "\n[177] stat_ci_prop(30/100) — Wald CI");
    exec_sql(db, "SELECT stat_ci_prop_wilson(30, 100);",
        "\n[178] stat_ci_prop_wilson(30/100) — Wilson CI");
    exec_sql(db, "SELECT stat_ci_prop_diff(30, 100, 40, 100);",
        "\n[179] stat_ci_prop_diff(30/100 vs 40/100)");

    // Model selection
    exec_sql(db, "SELECT stat_aic(-100.0, 3);",
        "\n[180] stat_aic(ll=-100, k=3)");
    exec_sql(db, "SELECT stat_aicc(-100.0, 50, 3);",
        "\n[181] stat_aicc(ll=-100, n=50, k=3)");
    exec_sql(db, "SELECT stat_bic(-100.0, 50, 3);",
        "\n[182] stat_bic(ll=-100, n=50, k=3)");

    // Box-Cox
    exec_sql(db, "SELECT stat_boxcox(5.0, 0.5);",
        "\n[183] stat_boxcox(5, lambda=0.5)");

    // ======================================================
    // Test Scalar — Distributions/Transforms: More Scalar Functions
    // ======================================================
    std::cout << "\n========================================\n";
    std::cout << "  Scalar — Distributions/Transforms: Distribution & Utility Functions\n";
    std::cout << "========================================\n";

    // Continuous distributions
    exec_sql(db, "SELECT stat_uniform_pdf(0.5);",
        "\n[184] stat_uniform_pdf(0.5) — 1.0 for [0,1]");
    exec_sql(db, "SELECT stat_uniform_cdf(0.5);",
        "\n[185] stat_uniform_cdf(0.5) — 0.5");
    exec_sql(db, "SELECT stat_uniform_quantile(0.5);",
        "\n[186] stat_uniform_quantile(0.5) — 0.5");
    exec_sql(db, "SELECT stat_uniform_rand();",
        "\n[187] stat_uniform_rand");

    exec_sql(db, "SELECT stat_exponential_pdf(1.0);",
        "\n[188] stat_exponential_pdf(1)");
    exec_sql(db, "SELECT stat_exponential_cdf(1.0);",
        "\n[189] stat_exponential_cdf(1) — ~0.632");
    exec_sql(db, "SELECT stat_exponential_quantile(0.5);",
        "\n[190] stat_exponential_quantile(0.5) — ~0.693");
    exec_sql(db, "SELECT stat_exponential_rand();",
        "\n[191] stat_exponential_rand");

    exec_sql(db, "SELECT stat_gamma_pdf(2.0, 2.0, 1.0);",
        "\n[192] stat_gamma_pdf(2, shape=2, rate=1)");
    exec_sql(db, "SELECT stat_gamma_cdf(2.0, 2.0, 1.0);",
        "\n[193] stat_gamma_cdf(2, 2, 1)");
    exec_sql(db, "SELECT stat_gamma_quantile(0.5, 2.0, 1.0);",
        "\n[194] stat_gamma_quantile(0.5, 2, 1)");
    exec_sql(db, "SELECT stat_gamma_rand(2.0, 1.0);",
        "\n[195] stat_gamma_rand(2, 1)");

    exec_sql(db, "SELECT stat_beta_pdf(0.5, 2.0, 5.0);",
        "\n[196] stat_beta_pdf(0.5, 2, 5)");
    exec_sql(db, "SELECT stat_beta_cdf(0.5, 2.0, 5.0);",
        "\n[197] stat_beta_cdf(0.5, 2, 5)");
    exec_sql(db, "SELECT stat_beta_quantile(0.5, 2.0, 5.0);",
        "\n[198] stat_beta_quantile(0.5, 2, 5)");
    exec_sql(db, "SELECT stat_beta_rand(2.0, 5.0);",
        "\n[199] stat_beta_rand(2, 5)");

    exec_sql(db, "SELECT stat_lognormal_pdf(1.0);",
        "\n[200] stat_lognormal_pdf(1)");
    exec_sql(db, "SELECT stat_lognormal_cdf(1.0);",
        "\n[201] stat_lognormal_cdf(1) — ~0.5");
    exec_sql(db, "SELECT stat_lognormal_quantile(0.5);",
        "\n[202] stat_lognormal_quantile(0.5) — ~1.0");
    exec_sql(db, "SELECT stat_lognormal_rand();",
        "\n[203] stat_lognormal_rand");

    exec_sql(db, "SELECT stat_weibull_pdf(1.0, 1.5, 1.0);",
        "\n[204] stat_weibull_pdf(1, shape=1.5, scale=1)");
    exec_sql(db, "SELECT stat_weibull_cdf(1.0, 1.5, 1.0);",
        "\n[205] stat_weibull_cdf(1, 1.5, 1)");
    exec_sql(db, "SELECT stat_weibull_quantile(0.5, 1.5, 1.0);",
        "\n[206] stat_weibull_quantile(0.5, 1.5, 1)");
    exec_sql(db, "SELECT stat_weibull_rand(1.5, 1.0);",
        "\n[207] stat_weibull_rand(1.5, 1)");

    // Discrete distributions
    exec_sql(db, "SELECT stat_binomial_pmf(3, 10, 0.5);",
        "\n[208] stat_binomial_pmf(3, 10, 0.5)");
    exec_sql(db, "SELECT stat_binomial_cdf(5, 10, 0.5);",
        "\n[209] stat_binomial_cdf(5, 10, 0.5) — ~0.623");
    exec_sql(db, "SELECT stat_binomial_quantile(0.5, 10, 0.5);",
        "\n[210] stat_binomial_quantile(0.5, 10, 0.5) — 5");
    exec_sql(db, "SELECT stat_binomial_rand(10, 0.5);",
        "\n[211] stat_binomial_rand(10, 0.5)");

    exec_sql(db, "SELECT stat_poisson_pmf(3, 2.5);",
        "\n[212] stat_poisson_pmf(3, lambda=2.5)");
    exec_sql(db, "SELECT stat_poisson_cdf(3, 2.5);",
        "\n[213] stat_poisson_cdf(3, 2.5)");
    exec_sql(db, "SELECT stat_poisson_quantile(0.5, 2.5);",
        "\n[214] stat_poisson_quantile(0.5, 2.5)");
    exec_sql(db, "SELECT stat_poisson_rand(2.5);",
        "\n[215] stat_poisson_rand(2.5)");

    exec_sql(db, "SELECT stat_geometric_pmf(3, 0.3);",
        "\n[216] stat_geometric_pmf(3, 0.3)");
    exec_sql(db, "SELECT stat_geometric_cdf(3, 0.3);",
        "\n[217] stat_geometric_cdf(3, 0.3)");
    exec_sql(db, "SELECT stat_geometric_quantile(0.5, 0.3);",
        "\n[218] stat_geometric_quantile(0.5, 0.3)");
    exec_sql(db, "SELECT stat_geometric_rand(0.3);",
        "\n[219] stat_geometric_rand(0.3)");

    exec_sql(db, "SELECT stat_nbinom_pmf(3, 5, 0.5);",
        "\n[220] stat_nbinom_pmf(3, r=5, p=0.5)");
    exec_sql(db, "SELECT stat_nbinom_cdf(5, 5, 0.5);",
        "\n[221] stat_nbinom_cdf(5, 5, 0.5)");
    exec_sql(db, "SELECT stat_nbinom_quantile(0.5, 5, 0.5);",
        "\n[222] stat_nbinom_quantile(0.5, 5, 0.5)");
    exec_sql(db, "SELECT stat_nbinom_rand(5, 0.5);",
        "\n[223] stat_nbinom_rand(5, 0.5)");

    exec_sql(db, "SELECT stat_hypergeom_pmf(3, 20, 8, 5);",
        "\n[224] stat_hypergeom_pmf(3, N=20, K=8, n=5)");
    exec_sql(db, "SELECT stat_hypergeom_cdf(3, 20, 8, 5);",
        "\n[225] stat_hypergeom_cdf(3, 20, 8, 5)");
    exec_sql(db, "SELECT stat_hypergeom_quantile(0.5, 20, 8, 5);",
        "\n[226] stat_hypergeom_quantile(0.5, 20, 8, 5)");
    exec_sql(db, "SELECT stat_hypergeom_rand(20, 8, 5);",
        "\n[227] stat_hypergeom_rand(20, 8, 5)");

    exec_sql(db, "SELECT stat_bernoulli_pmf(1, 0.7);",
        "\n[228] stat_bernoulli_pmf(1, 0.7) — 0.7");
    exec_sql(db, "SELECT stat_bernoulli_cdf(0, 0.7);",
        "\n[229] stat_bernoulli_cdf(0, 0.7) — 0.3");
    exec_sql(db, "SELECT stat_bernoulli_quantile(0.5, 0.7);",
        "\n[230] stat_bernoulli_quantile(0.5, 0.7)");
    exec_sql(db, "SELECT stat_bernoulli_rand(0.7);",
        "\n[231] stat_bernoulli_rand(0.7)");

    exec_sql(db, "SELECT stat_duniform_pmf(3, 1, 6);",
        "\n[232] stat_duniform_pmf(3, 1, 6) — 1/6");
    exec_sql(db, "SELECT stat_duniform_cdf(3, 1, 6);",
        "\n[233] stat_duniform_cdf(3, 1, 6) — 0.5");
    exec_sql(db, "SELECT stat_duniform_quantile(0.5, 1, 6);",
        "\n[234] stat_duniform_quantile(0.5, 1, 6) — 3");
    exec_sql(db, "SELECT stat_duniform_rand(1, 6);",
        "\n[235] stat_duniform_rand(1, 6)");

    // Combinatorics
    exec_sql(db, "SELECT stat_binomial_coef(10, 3);",
        "\n[236] stat_binomial_coef(10, 3) — 120");
    exec_sql(db, "SELECT stat_log_binomial_coef(10, 3);",
        "\n[237] stat_log_binomial_coef(10, 3)");
    exec_sql(db, "SELECT stat_log_factorial(10);",
        "\n[238] stat_log_factorial(10) — ~15.104");

    // Special functions
    exec_sql(db, "SELECT stat_lgamma(5);",
        "\n[239] stat_lgamma(5) — ln(4!) = ~3.178");
    exec_sql(db, "SELECT stat_tgamma(5);",
        "\n[240] stat_tgamma(5) — 4! = 24");
    exec_sql(db, "SELECT stat_beta_func(2, 3);",
        "\n[241] stat_beta_func(2, 3)");
    exec_sql(db, "SELECT stat_lbeta(2, 3);",
        "\n[242] stat_lbeta(2, 3)");
    exec_sql(db, "SELECT stat_erf(1.0);",
        "\n[243] stat_erf(1) — ~0.843");
    exec_sql(db, "SELECT stat_erfc(1.0);",
        "\n[244] stat_erfc(1) — ~0.157");

    // Scalar statistics
    exec_sql(db, "SELECT stat_logarithmic_mean(2, 8);",
        "\n[245] stat_logarithmic_mean(2, 8)");

    // Effect size corrections/conversions
    exec_sql(db, "SELECT stat_hedges_j(20);",
        "\n[246] stat_hedges_j(df=20) — correction factor");
    exec_sql(db, "SELECT stat_t_to_r(2.5, 30);",
        "\n[247] stat_t_to_r(t=2.5, df=30)");
    exec_sql(db, "SELECT stat_d_to_r(0.8);",
        "\n[248] stat_d_to_r(d=0.8)");
    exec_sql(db, "SELECT stat_r_to_d(0.5);",
        "\n[249] stat_r_to_d(r=0.5)");
    exec_sql(db, "SELECT stat_eta_squared_ef(50.0, 200.0);",
        "\n[250] stat_eta_squared_ef(ss_eff=50, ss_total=200) — 0.25");
    exec_sql(db, "SELECT stat_partial_eta_sq(5.0, 2, 30);",
        "\n[251] stat_partial_eta_sq(F=5, df1=2, df2=30)");
    exec_sql(db, "SELECT stat_omega_squared_ef(50.0, 200.0, 5.0, 2);",
        "\n[252] stat_omega_squared_ef(ss_eff, ss_total, ms_err, df_eff)");
    exec_sql(db, "SELECT stat_cohens_h(0.6, 0.4);",
        "\n[253] stat_cohens_h(p1=0.6, p2=0.4)");

    // Interpretation
    exec_sql(db, "SELECT stat_interpret_d(0.8);",
        "\n[254] stat_interpret_d(0.8) — 'large'");
    exec_sql(db, "SELECT stat_interpret_r(0.3);",
        "\n[255] stat_interpret_r(0.3) — 'medium'");
    exec_sql(db, "SELECT stat_interpret_eta2(0.14);",
        "\n[256] stat_interpret_eta2(0.14) — 'large'");

    // Power analysis
    exec_sql(db, "SELECT stat_power_t1(0.5, 30, 0.05);",
        "\n[257] stat_power_t1(es=0.5, n=30, alpha=0.05)");
    exec_sql(db, "SELECT stat_n_t1(0.5, 0.8, 0.05);",
        "\n[258] stat_n_t1(es=0.5, power=0.8, alpha=0.05)");
    exec_sql(db, "SELECT stat_power_t2(0.5, 30, 30, 0.05);",
        "\n[259] stat_power_t2(es=0.5, n1=30, n2=30, alpha=0.05)");
    exec_sql(db, "SELECT stat_n_t2(0.5, 0.8, 0.05);",
        "\n[260] stat_n_t2(es=0.5, power=0.8, alpha=0.05)");
    exec_sql(db, "SELECT stat_power_prop(0.3, 0.5, 100, 0.05);",
        "\n[261] stat_power_prop(p1=0.3, p2=0.5, n=100, alpha=0.05)");
    exec_sql(db, "SELECT stat_n_prop(0.3, 0.5, 0.8, 0.05);",
        "\n[262] stat_n_prop(p1=0.3, p2=0.5, power=0.8, alpha=0.05)");

    // Margin of error / sample size
    exec_sql(db, "SELECT stat_moe_prop(30, 100);",
        "\n[263] stat_moe_prop(30/100)");
    exec_sql(db, "SELECT stat_moe_prop_worst(100);",
        "\n[264] stat_moe_prop_worst(n=100) — worst case MoE");
    exec_sql(db, "SELECT stat_n_moe_prop(0.05);",
        "\n[265] stat_n_moe_prop(moe=0.05) — sample size for 5% MoE");
    exec_sql(db, "SELECT stat_n_moe_mean(1.0, 5.0);",
        "\n[266] stat_n_moe_mean(moe=1, sigma=5)");

    // Cleanup
    sqlite3_close(db);
    std::cout << "\n[OK] All tests completed (266 tests).\n";

    return 0;
}
