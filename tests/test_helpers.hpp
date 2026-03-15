/**
 * @file test_helpers.hpp
 * @brief 統計関数テスト用の共通ヘルパーとフィクスチャ
 *
 * Google Test フィクスチャとして,インメモリDB作成・拡張ロード・
 * テストデータ投入を共通化し,各テストファイルから利用する.
 */
#pragma once

#include <gtest/gtest.h>
#include <sqlite3.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

// =====================================================================
// 拡張ライブラリのパス(CMakeから注入)
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
// ヘルパー関数(フリー関数)
// =====================================================================

/**
 * @brief DDL/DML を実行する. エラー時は FAIL() で即座にテスト失敗
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
 * @brief SELECT の結果を double で取得する
 * @return 結果値. NULL の場合は NaN を返す
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
 * @brief SELECT の結果をテキスト(JSON等)で取得する
 * @return 結果文字列. NULL の場合は空文字列
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
 * @brief SELECT の結果を int64_t で取得する
 * @return 結果値. NULL の場合は 0 を返す
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
 * @brief SELECT の結果が NULL かどうかを判定する
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
 * @brief SELECT の複数行結果を vector<double> で取得する(ウィンドウ関数用)
 *        NULL 行は NaN として格納
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
 * @brief JSON文字列から json_extract() で double 値を取得する
 */
inline double json_double(sqlite3* db, const std::string& json,
                          const char* path) {
    std::string sql = "SELECT json_extract('" + json + "', '" +
                      std::string(path) + "')";
    return query_double(db, sql.c_str());
}

/**
 * @brief JSON文字列から json_extract() でテキスト値を取得する
 */
inline std::string json_text(sqlite3* db, const std::string& json,
                             const char* path) {
    std::string sql = "SELECT json_extract('" + json + "', '" +
                      std::string(path) + "')";
    return query_text(db, sql.c_str());
}

// =====================================================================
// テストフィクスチャ
// =====================================================================

/**
 * @brief 統計関数テスト用の基本フィクスチャ
 *
 * SetUp() でインメモリDB作成,ext_funcs拡張ロード,テストデータ投入を行い,
 * TearDown() でDB close する.
 */
class StatFuncTest : public ::testing::Test {
protected:
    sqlite3* db_ = nullptr;

    void SetUp() override {
        // インメモリDB作成
        int rc = sqlite3_open(":memory:", &db_);
        ASSERT_EQ(rc, SQLITE_OK) << "Cannot open database: "
                                 << sqlite3_errmsg(db_);

        // 拡張ロード有効化
        rc = sqlite3_enable_load_extension(db_, 1);
        ASSERT_EQ(rc, SQLITE_OK) << "enable_load_extension failed: "
                                 << sqlite3_errmsg(db_);

        // ext_funcs 拡張ロード
        char* load_errmsg = nullptr;
        rc = sqlite3_load_extension(db_, EXT_FUNCS_PATH,
                                    "sqlite3_ext_funcs_init", &load_errmsg);
        if (rc != SQLITE_OK) {
            std::string msg = load_errmsg ? load_errmsg : "unknown";
            sqlite3_free(load_errmsg);
            FAIL() << "Load extension failed: " << msg
                   << "\n  Path: " << EXT_FUNCS_PATH;
        }

        // テストデータ投入
        CreateTestData();
    }

    void TearDown() override {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

private:
    /// @brief 全テストデータテーブルを作成する
    void CreateTestData() {
        // data: 基本統計用 (val: 1-10, mean=5.5, median=5.5)
        exec_sql(db_, "CREATE TABLE data (id INTEGER PRIMARY KEY, val REAL)");
        for (int i = 1; i <= 10; ++i) {
            std::string sql = "INSERT INTO data (val) VALUES (" +
                              std::to_string(i) + ")";
            exec_sql(db_, sql.c_str());
        }

        // data2: 最頻値用 (mode=3)
        exec_sql(db_, "CREATE TABLE data2 (val REAL)");
        exec_sql(db_,
            "INSERT INTO data2 VALUES (1),(2),(2),(3),(3),(3),(4),(4),(5)");

        // pos_data: 幾何平均用 (2,4,8 → geometric_mean=4.0)
        exec_sql(db_, "CREATE TABLE pos_data (val REAL)");
        exec_sql(db_, "INSERT INTO pos_data VALUES (2),(4),(8)");

        // nulldata: NULL処理用 (1,NULL,3,NULL,5)
        exec_sql(db_, "CREATE TABLE nulldata (val REAL)");
        exec_sql(db_,
            "INSERT INTO nulldata VALUES (1),(NULL),(3),(NULL),(5)");

        // empty_data: 空テーブル(エッジケース用)
        exec_sql(db_, "CREATE TABLE empty_data (val REAL)");

        // grouped: GROUP BY テスト用
        exec_sql(db_, "CREATE TABLE grouped (grp TEXT, val REAL)");
        exec_sql(db_,
            "INSERT INTO grouped VALUES "
            "('A',1),('A',2),('A',3),('B',10),('B',20),('B',30)");

        // xy_data: 相関・回帰用 (10行)
        exec_sql(db_, "CREATE TABLE xy_data (x REAL, y REAL)");
        exec_sql(db_,
            "INSERT INTO xy_data VALUES "
            "(1,2),(2,4),(3,5),(4,4),(5,5),(6,7),(7,8),(8,6),(9,11),(10,8)");

        // wt_data: 加重統計用 (5行)
        exec_sql(db_, "CREATE TABLE wt_data (val REAL, wt REAL)");
        exec_sql(db_,
            "INSERT INTO wt_data VALUES "
            "(10,1),(20,2),(30,3),(40,2),(50,1)");

        // pred_data: 予測精度メトリクス用
        exec_sql(db_, "CREATE TABLE pred_data (actual REAL, predicted REAL)");
        exec_sql(db_,
            "INSERT INTO pred_data VALUES "
            "(3,2.5),(5,5.2),(7,6.8),(9,9.1),(11,10.5)");

        // ts_data: ウィンドウ関数用 (10行, NULL at id=3,8)
        exec_sql(db_, "CREATE TABLE ts_data (id INTEGER PRIMARY KEY, val REAL)");
        exec_sql(db_,
            "INSERT INTO ts_data VALUES "
            "(1,10),(2,20),(3,NULL),(4,40),(5,50),"
            "(6,30),(7,70),(8,NULL),(9,90),(10,100)");

        // cat_data: カテゴリカルエンコーディング用
        exec_sql(db_,
            "CREATE TABLE cat_data (id INTEGER PRIMARY KEY, val REAL)");
        exec_sql(db_,
            "INSERT INTO cat_data VALUES "
            "(1,100),(2,200),(3,100),(4,300),(5,200),(6,100)");

        // surv_data: 生存分析用
        exec_sql(db_, "CREATE TABLE surv_data(time REAL, event INT)");
        exec_sql(db_,
            "INSERT INTO surv_data VALUES "
            "(1,1),(2,0),(3,1),(4,1),(5,0),(6,1),(7,1),(8,0),(9,1),(10,1)");

        // grp_data: 2標本検定用
        exec_sql(db_, "CREATE TABLE grp_data(val REAL, grp INT)");
        exec_sql(db_,
            "INSERT INTO grp_data VALUES "
            "(10,0),(12,0),(14,0),(16,0),(18,0),"
            "(20,1),(22,1),(24,1),(26,1),(28,1)");

        // surv2: ログランク検定用 (2群)
        exec_sql(db_, "CREATE TABLE surv2(time REAL, event INT, grp INT)");
        exec_sql(db_,
            "INSERT INTO surv2 VALUES "
            "(1,1,0),(3,0,0),(5,1,0),(7,1,0),(9,0,0),"
            "(2,1,1),(4,1,1),(6,0,1),(8,1,1),(10,1,1)");
    }
};
