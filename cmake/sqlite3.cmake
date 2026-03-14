# ============================================================
# CMake build configuration
#
# Project: [SQLite3: download amalgamation from official site and build static library]
# Author: mitsuruk
# Date:    2025/11/26
# License: Public Domain (same as SQLite)
# Note: SQLite itself is in the public domain. See https://sqlite.org/copyright.html
# ============================================================

include_guard(GLOBAL)

# If you have already downloaded SQLite3 amalgamation files in ${CMAKE_SOURCE_DIR}/download/sqlite3
# Cmake compile process will use the cached files instead of downloading again.
# At first time build, it will download from official site and cache them.
# If you want to force re-download, please delete the cache directory.

# Set local cache directory
set(SQLITE3_CACHE_DIR "${CMAKE_SOURCE_DIR}/download/sqlite3")

# Check if cache exists
if(EXISTS "${SQLITE3_CACHE_DIR}/sqlite3.c" AND
   EXISTS "${SQLITE3_CACHE_DIR}/sqlite3.h" AND
   EXISTS "${SQLITE3_CACHE_DIR}/sqlite3ext.h")
  message(STATUS "Using cached SQLite3 from: ${SQLITE3_CACHE_DIR}")
  set(SQLITE3_SOURCE_DIR "${SQLITE3_CACHE_DIR}")
else()
  # Download if cache does not exist
  message(STATUS "SQLite3 cache not found, downloading...")

  include(FetchContent)

  # Attempt to automatically fetch from the latest download page
  # Fallback to a known stable version if it fails
  set(SQLITE_DOWNLOAD_PAGE "https://sqlite.org/download.html")

  message(STATUS "Fetching SQLite download page to find latest version...")
  file(DOWNLOAD
    ${SQLITE_DOWNLOAD_PAGE}
    ${CMAKE_BINARY_DIR}/sqlite_download.html
    STATUS DOWNLOAD_STATUS
  )

  list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
  if(NOT STATUS_CODE EQUAL 0)
    message(FATAL_ERROR "Could not download SQLite download page")
  endif()

  file(READ ${CMAKE_BINARY_DIR}/sqlite_download.html _sqlite_html)
  # Match relative path format URL (e.g., 2025/sqlite-autoconf-3510000.tar.gz)
  string(REGEX MATCH "([0-9]+)/sqlite-autoconf-([0-9]+)\\.tar\\.gz" _match "${_sqlite_html}")

  if(NOT _match)
    message(FATAL_ERROR "Could not parse latest SQLite version from download page")
  endif()

  set(SQLITE_YEAR "${CMAKE_MATCH_1}")
  set(SQLITE_VERSION_NUMBER "${CMAKE_MATCH_2}")
  message(STATUS "Detected SQLite version: ${SQLITE_VERSION_NUMBER} (year: ${SQLITE_YEAR})")

  # Download SQLite amalgamation
  set(SQLITE_URL "https://sqlite.org/${SQLITE_YEAR}/sqlite-autoconf-${SQLITE_VERSION_NUMBER}.tar.gz")
  message(STATUS "Downloading SQLite from: ${SQLITE_URL}")

  FetchContent_Declare(
    sqlite3_download
    URL ${SQLITE_URL}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  )

  FetchContent_MakeAvailable(sqlite3_download)

  # Copy downloaded source to cache directory
  set(SQLITE3_DOWNLOAD_DIR "${sqlite3_download_SOURCE_DIR}")
  message(STATUS "Caching SQLite3 to: ${SQLITE3_CACHE_DIR}")

  file(MAKE_DIRECTORY "${SQLITE3_CACHE_DIR}")
  file(COPY "${SQLITE3_DOWNLOAD_DIR}/sqlite3.c" DESTINATION "${SQLITE3_CACHE_DIR}")
  file(COPY "${SQLITE3_DOWNLOAD_DIR}/sqlite3.h" DESTINATION "${SQLITE3_CACHE_DIR}")
  file(COPY "${SQLITE3_DOWNLOAD_DIR}/sqlite3ext.h" DESTINATION "${SQLITE3_CACHE_DIR}")

  # Also save version information
  file(WRITE "${SQLITE3_CACHE_DIR}/VERSION.txt" "${SQLITE_VERSION_NUMBER}")

  set(SQLITE3_SOURCE_DIR "${SQLITE3_CACHE_DIR}")
  message(STATUS "SQLite3 cached successfully")
endif()

# Copy header files to a separate directory to avoid conflicts between
# SQLite's version file and the C++ standard library's <version>
set(SQLITE3_INCLUDE_DIR "${CMAKE_BINARY_DIR}/sqlite3_include")
file(MAKE_DIRECTORY "${SQLITE3_INCLUDE_DIR}")
file(COPY "${SQLITE3_SOURCE_DIR}/sqlite3.h" DESTINATION "${SQLITE3_INCLUDE_DIR}")
file(COPY "${SQLITE3_SOURCE_DIR}/sqlite3ext.h" DESTINATION "${SQLITE3_INCLUDE_DIR}")

# SQLite3 static library (using amalgamation)
add_library(sqlite3 STATIC
    "${SQLITE3_SOURCE_DIR}/sqlite3.c"
)

# The sqlite3 library itself uses the source directory as a private include
target_include_directories(sqlite3 PRIVATE
    ${SQLITE3_SOURCE_DIR}
)

# External users use the copied header directory
target_include_directories(sqlite3 PUBLIC
    ${SQLITE3_INCLUDE_DIR}
)

target_compile_definitions(sqlite3 PUBLIC
    SQLITE_ENABLE_FTS5                      # Full-text search engine version 5
    SQLITE_ENABLE_MATH_FUNCTIONS            # Math functions: sin(), log(), sqrt(), etc.
    SQLITE_ENABLE_STAT4                     # Enhanced query planner statistics
    SQLITE_ENABLE_COLUMN_METADATA           # Column metadata API support
    SQLITE_DQS=0                            # Disable double-quoted string literals
    SQLITE_DEFAULT_MEMSTATUS=0              # Disable memory usage tracking for speed
    SQLITE_DEFAULT_WAL_SYNCHRONOUS=1        # Set WAL sync level to NORMAL
    SQLITE_LIKE_DOESNT_MATCH_BLOBS          # LIKE operator skips BLOB values
    SQLITE_OMIT_DEPRECATED                  # Omit deprecated APIs
    SQLITE_MAX_EXPR_DEPTH=0                 # Remove expression depth limit
    SQLITE_THREADSAFE=2                     # Thread-safe mode: 1=serialized, 2=multi-thread, 0=off
    # SQLITE_ENABLE_FTS3                    # Full-text search engine version 3
    # SQLITE_ENABLE_FTS4                    # Full-text search engine version 4
    # SQLITE_ENABLE_RTREE                   # R-Tree index for spatial queries
    # SQLITE_ENABLE_GEOPOLY                 # Geospatial extension based on R-Tree
    # SQLITE_ENABLE_JSON1                   # JSON functions (built-in since 3.38.0)
    # SQLITE_ENABLE_LOAD_EXTENSION          # Allow loading extensions at runtime
    # SQLITE_ENABLE_UPDATE_DELETE_LIMIT     # Allow LIMIT clause in UPDATE/DELETE
    # SQLITE_ENABLE_DBSTAT_VTAB             # dbstat virtual table for DB analysis
    # SQLITE_ENABLE_STMTVTAB                # sqlite_stmt virtual table for diagnostics
    # SQLITE_ENABLE_EXPLAIN_COMMENTS        # Add comments to EXPLAIN output
    # SQLITE_OMIT_DECLTYPE                  # Omit sqlite3_column_decltype() for smaller build
)

target_compile_options(sqlite3 PRIVATE
    -O2
)

# Link to the main target
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3)
