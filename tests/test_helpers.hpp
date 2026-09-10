/**
 * @file test_helpers.hpp
 * @brief Shared helpers and fixture for the statistical function tests
 *
 * Provides a Google Test fixture that centralises creating the in-memory database,
 * loading the extension and inserting the test data, for use by every test file.
 */
#pragma once

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// =====================================================================
// Path to the extension library (injected by CMake)
// =====================================================================
#ifndef EXT_FUNCS_PATH
#if defined(__APPLE__)
#define EXT_FUNCS_PATH "./ext_funcs.dylib"
#elif defined(_WIN32)
#define EXT_FUNCS_PATH "./ext_funcs.dll"
#else
#define EXT_FUNCS_PATH "./ext_funcs.so"
#endif
#endif

// =====================================================================
// Helper functions (free functions)
// =====================================================================

/**
 * @brief Run DDL/DML. Fails the test immediately via FAIL() on error
 */
inline void exec_sql(sqlite3* db, const char* sql) {
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        std::string msg = errmsg ? errmsg : "unknown error";
        sqlite3_free(errmsg);
        FAIL() << "exec_sql failed: " << msg << "\n  SQL: " << sql;
    }
}

/**
 * @brief Fetch a SELECT result as a double
 * @return The value, or NaN if it is NULL
 */
inline double query_double(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    EXPECT_EQ(rc, SQLITE_OK) << "prepare failed: " << sqlite3_errmsg(db)
                              << "\n  SQL: " << sql;
    rc = sqlite3_step(stmt);
    double result = std::numeric_limits<double>::quiet_NaN();
    if (rc == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
        result = sqlite3_column_double(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return result;
}

/**
 * @brief Fetch a SELECT result as text (JSON and the like)
 * @return The string, or empty if it is NULL
 */
inline std::string query_text(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    EXPECT_EQ(rc, SQLITE_OK) << "prepare failed: " << sqlite3_errmsg(db)
                              << "\n  SQL: " << sql;
    rc = sqlite3_step(stmt);
    std::string result;
    if (rc == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
        const auto* text = reinterpret_cast<const char*>(
            sqlite3_column_text(stmt, 0));
        if (text) result = text;
    }
    sqlite3_finalize(stmt);
    return result;
}

/**
 * @brief Fetch a SELECT result as an int64_t
 * @return The value, or 0 if it is NULL
 */
inline int64_t query_int(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    EXPECT_EQ(rc, SQLITE_OK) << "prepare failed: " << sqlite3_errmsg(db)
                              << "\n  SQL: " << sql;
    rc = sqlite3_step(stmt);
    int64_t result = 0;
    if (rc == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
        result = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return result;
}

/**
 * @brief Determine whether a SELECT result is NULL
 */
inline bool query_is_null(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    EXPECT_EQ(rc, SQLITE_OK) << "prepare failed: " << sqlite3_errmsg(db)
                              << "\n  SQL: " << sql;
    rc = sqlite3_step(stmt);
    bool is_null = true;
    if (rc == SQLITE_ROW) {
        is_null = (sqlite3_column_type(stmt, 0) == SQLITE_NULL);
    }
    sqlite3_finalize(stmt);
    return is_null;
}

/**
 * @brief Assert that a SELECT fails and return the error message
 *
 * statcpp throws std::invalid_argument when an argument is out of range. If the
 * extension fails to catch it, the exception crosses SQLite's C ABI boundary and
 * reaches std::terminate, taking the test process down with it. That this helper
 * can return a value at all is itself evidence the guard is working.
 *
 * @param db Target database connection
 * @param sql SQL to execute
 * @return The error message if it failed, or an empty string on success
 */
inline std::string query_error(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string msg = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        return msg;
    }
    rc = sqlite3_step(stmt);
    std::string msg;
    if (rc != SQLITE_ROW && rc != SQLITE_DONE) {
        msg = sqlite3_errmsg(db);
    }
    sqlite3_finalize(stmt);
    return msg;
}

/**
 * @brief Fetch a multi-row SELECT result as a vector<double> (for window functions)
 *        NULL rows are stored as NaN
 */
inline std::vector<double> query_doubles(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    EXPECT_EQ(rc, SQLITE_OK) << "prepare failed: " << sqlite3_errmsg(db)
                              << "\n  SQL: " << sql;
    std::vector<double> results;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
            results.push_back(std::numeric_limits<double>::quiet_NaN());
        } else {
            results.push_back(sqlite3_column_double(stmt, 0));
        }
    }
    sqlite3_finalize(stmt);
    return results;
}

/**
 * @brief Extract a double from a JSON string via json_extract()
 */
inline double json_double(sqlite3* db, const std::string& json,
                          const char* path) {
    std::string sql = "SELECT json_extract('" + json + "', '" +
                      std::string(path) + "')";
    return query_double(db, sql.c_str());
}

/**
 * @brief Extract text from a JSON string via json_extract()
 */
inline std::string json_text(sqlite3* db, const std::string& json,
                             const char* path) {
    std::string sql = "SELECT json_extract('" + json + "', '" +
                      std::string(path) + "')";
    return query_text(db, sql.c_str());
}

// =====================================================================
// Test fixtures
// =====================================================================

/**
 * @brief Base fixture for the statistical function tests
 *
 * SetUp() creates the in-memory database, loads the ext_funcs extension and inserts
 * the test data; TearDown() closes the database.
 */
class StatFuncTest : public ::testing::Test {
protected:
    sqlite3* db_ = nullptr;

    void SetUp() override {
        // Create the in-memory database
        int rc = sqlite3_open(":memory:", &db_);
        ASSERT_EQ(rc, SQLITE_OK) << "Cannot open database: "
                                 << sqlite3_errmsg(db_);

        // Enable extension loading
        rc = sqlite3_enable_load_extension(db_, 1);
        ASSERT_EQ(rc, SQLITE_OK) << "enable_load_extension failed: "
                                 << sqlite3_errmsg(db_);

        // Load the ext_funcs extension
        char* load_errmsg = nullptr;
        rc = sqlite3_load_extension(db_, EXT_FUNCS_PATH,
                                    "sqlite3_ext_funcs_init", &load_errmsg);
        if (rc != SQLITE_OK) {
            std::string msg = load_errmsg ? load_errmsg : "unknown";
            sqlite3_free(load_errmsg);
            FAIL() << "Load extension failed: " << msg
                   << "\n  Path: " << EXT_FUNCS_PATH;
        }

        // Insert the test data
        CreateTestData();
    }

    void TearDown() override {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

private:
    /// @brief Create every test data table
    void CreateTestData() {
        // data: basic statistics (val: 1-10, mean=5.5, median=5.5)
        exec_sql(db_, "CREATE TABLE data (id INTEGER PRIMARY KEY, val REAL)");
        for (int i = 1; i <= 10; ++i) {
            std::string sql = "INSERT INTO data (val) VALUES (" +
                              std::to_string(i) + ")";
            exec_sql(db_, sql.c_str());
        }

        // data2: mode (mode=3)
        exec_sql(db_, "CREATE TABLE data2 (val REAL)");
        exec_sql(db_,
            "INSERT INTO data2 VALUES (1),(2),(2),(3),(3),(3),(4),(4),(5)");

        // pos_data: geometric mean (2,4,8 -> geometric_mean=4.0)
        exec_sql(db_, "CREATE TABLE pos_data (val REAL)");
        exec_sql(db_, "INSERT INTO pos_data VALUES (2),(4),(8)");

        // nulldata: NULL handling (1,NULL,3,NULL,5)
        exec_sql(db_, "CREATE TABLE nulldata (val REAL)");
        exec_sql(db_,
            "INSERT INTO nulldata VALUES (1),(NULL),(3),(NULL),(5)");

        // empty_data: empty table (edge cases)
        exec_sql(db_, "CREATE TABLE empty_data (val REAL)");

        // grouped: GROUP BY tests
        exec_sql(db_, "CREATE TABLE grouped (grp TEXT, val REAL)");
        exec_sql(db_,
            "INSERT INTO grouped VALUES "
            "('A',1),('A',2),('A',3),('B',10),('B',20),('B',30)");

        // xy_data: correlation and regression (10 rows)
        exec_sql(db_, "CREATE TABLE xy_data (x REAL, y REAL)");
        exec_sql(db_,
            "INSERT INTO xy_data VALUES "
            "(1,2),(2,4),(3,5),(4,4),(5,5),(6,7),(7,8),(8,6),(9,11),(10,8)");

        // wt_data: weighted statistics (5 rows)
        exec_sql(db_, "CREATE TABLE wt_data (val REAL, wt REAL)");
        exec_sql(db_,
            "INSERT INTO wt_data VALUES "
            "(10,1),(20,2),(30,3),(40,2),(50,1)");

        // pred_data: prediction accuracy metrics
        exec_sql(db_, "CREATE TABLE pred_data (actual REAL, predicted REAL)");
        exec_sql(db_,
            "INSERT INTO pred_data VALUES "
            "(3,2.5),(5,5.2),(7,6.8),(9,9.1),(11,10.5)");

        // ts_data: window functions (10 rows, NULL at id=3,8)
        exec_sql(db_, "CREATE TABLE ts_data (id INTEGER PRIMARY KEY, val REAL)");
        exec_sql(db_,
            "INSERT INTO ts_data VALUES "
            "(1,10),(2,20),(3,NULL),(4,40),(5,50),"
            "(6,30),(7,70),(8,NULL),(9,90),(10,100)");

        // cat_data: categorical encoding
        exec_sql(db_,
            "CREATE TABLE cat_data (id INTEGER PRIMARY KEY, val REAL)");
        exec_sql(db_,
            "INSERT INTO cat_data VALUES "
            "(1,100),(2,200),(3,100),(4,300),(5,200),(6,100)");

        // surv_data: survival analysis
        exec_sql(db_, "CREATE TABLE surv_data(time REAL, event INT)");
        exec_sql(db_,
            "INSERT INTO surv_data VALUES "
            "(1,1),(2,0),(3,1),(4,1),(5,0),(6,1),(7,1),(8,0),(9,1),(10,1)");

        // grp_data: two-sample tests
        exec_sql(db_, "CREATE TABLE grp_data(val REAL, grp INT)");
        exec_sql(db_,
            "INSERT INTO grp_data VALUES "
            "(10,0),(12,0),(14,0),(16,0),(18,0),"
            "(20,1),(22,1),(24,1),(26,1),(28,1)");

        // surv2: log-rank test (two groups)
        exec_sql(db_, "CREATE TABLE surv2(time REAL, event INT, grp INT)");
        exec_sql(db_,
            "INSERT INTO surv2 VALUES "
            "(1,1,0),(3,0,0),(5,1,0),(7,1,0),(9,0,0),"
            "(2,1,1),(4,1,1),(6,0,1),(8,1,1),(10,1,1)");
    }
};
