# sqlite3.cmake Reference

## Overview

`sqlite3.cmake` is a CMake configuration file that automatically downloads the SQLite3 database library from the official site and builds it as a static library. A caching mechanism skips re-downloading on subsequent builds.

## File Information

| Item | Details |
|------|---------|
| Project | SQLite3 amalgamation download and build |
| Author | mitsuruk |
| Created | 2025/11/26 |
| License | Public Domain (same as SQLite) |

---

## Include Guard

```cmake
include_guard(GLOBAL)
```

This file uses `include_guard(GLOBAL)` to ensure it is only executed once, even if included multiple times.

**Why it's needed:**

- Prevents duplicate definition errors from `add_library(sqlite3 ...)`
- Avoids duplicate execution of the download process
- Prevents duplicate linking in `target_link_libraries`

---

## Processing Flow

```
1. Check cache
   ├─ Exists → Use cache
   └─ Does not exist → Proceed to step 2
2. Detect latest version from the official site
3. Download SQLite amalgamation
4. Save to cache directory
5. Copy header files to a separate directory (to avoid conflicts)
6. Build static library
7. Link to the main project
```

---

## Feature Details

### 1. Caching Mechanism

```cmake
set(SQLITE3_CACHE_DIR "${CMAKE_SOURCE_DIR}/download/sqlite3")

if(EXISTS "${SQLITE3_CACHE_DIR}/sqlite3.c" AND
   EXISTS "${SQLITE3_CACHE_DIR}/sqlite3.h" AND
   EXISTS "${SQLITE3_CACHE_DIR}/sqlite3ext.h")
  message(STATUS "Using cached SQLite3 from: ${SQLITE3_CACHE_DIR}")
  set(SQLITE3_SOURCE_DIR "${SQLITE3_CACHE_DIR}")
else()
  # Download process
endif()
```

If the required files are present in the cache directory, the download is skipped.

**Cached Files:**

| File | Description |
|------|-------------|
| `sqlite3.c` | SQLite main source (amalgamation) |
| `sqlite3.h` | Public header |
| `sqlite3ext.h` | Extension header |
| `VERSION.txt` | Version information (auto-generated) |

---

### 2. Automatic Detection of the Latest Version

```cmake
set(SQLITE_DOWNLOAD_PAGE "https://sqlite.org/download.html")

file(DOWNLOAD
    ${SQLITE_DOWNLOAD_PAGE}
    ${CMAKE_BINARY_DIR}/sqlite_download.html
    STATUS DOWNLOAD_STATUS
)

# Extract download URL from HTML
string(REGEX MATCH "([0-9]+)/sqlite-autoconf-([0-9]+)\\.tar\\.gz"
    _match "${_sqlite_html}")
```

Fetches the official SQLite download page and extracts the URL of the latest autoconf package using a regular expression.

---

### 3. Download and Cache

```cmake
set(SQLITE_URL "https://sqlite.org/${SQLITE_YEAR}/sqlite-autoconf-${SQLITE_VERSION_NUMBER}.tar.gz")

FetchContent_Declare(
    sqlite3_download
    URL ${SQLITE_URL}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(sqlite3_download)
```

Uses the `FetchContent` module to download and extract the archive.

---

### 4. Header File Separation (Conflict Avoidance)

```cmake
set(SQLITE3_INCLUDE_DIR "${CMAKE_BINARY_DIR}/sqlite3_include")
file(MAKE_DIRECTORY "${SQLITE3_INCLUDE_DIR}")
file(COPY "${SQLITE3_SOURCE_DIR}/sqlite3.h" DESTINATION "${SQLITE3_INCLUDE_DIR}")
file(COPY "${SQLITE3_SOURCE_DIR}/sqlite3ext.h" DESTINATION "${SQLITE3_INCLUDE_DIR}")
```

**Important**: The SQLite distribution includes a `VERSION` file that can conflict with the C++ standard library `<version>` header. To avoid this, header files are copied to a separate directory.

---

### 5. Building the Static Library

```cmake
add_library(sqlite3 STATIC
    "${SQLITE3_SOURCE_DIR}/sqlite3.c"
)
```

Creates a static library from the SQLite amalgamation (single source file).

---

### 6. Setting Include Directories

```cmake
# The sqlite3 library itself uses the source directory (PRIVATE)
target_include_directories(sqlite3 PRIVATE
    ${SQLITE3_SOURCE_DIR}
)

# External users use the copied headers (PUBLIC)
target_include_directories(sqlite3 PUBLIC
    ${SQLITE3_INCLUDE_DIR}
)
```

---

### 7. Compile Settings

```cmake
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
)

target_compile_options(sqlite3 PRIVATE
    -O2
)
```

| Definition | Description |
| ---------- | ----------- |
| `SQLITE_ENABLE_FTS5` | Enables the full-text search engine (FTS5) |
| `SQLITE_ENABLE_MATH_FUNCTIONS` | Enables math functions such as `sin()`, `log()`, `sqrt()` |
| `SQLITE_ENABLE_STAT4` | Enables enhanced query planner statistics for better performance with large datasets |
| `SQLITE_ENABLE_COLUMN_METADATA` | Enables column metadata API (`sqlite3_column_table_name()`, etc.) |
| `SQLITE_DQS=0` | Disables double-quoted string literals (recommended for safety) |
| `SQLITE_DEFAULT_MEMSTATUS=0` | Disables memory usage tracking for slightly faster performance |
| `SQLITE_DEFAULT_WAL_SYNCHRONOUS=1` | Sets WAL mode sync level to NORMAL for better write performance |
| `SQLITE_LIKE_DOESNT_MATCH_BLOBS` | LIKE operator skips BLOB values for faster text searches |
| `SQLITE_OMIT_DEPRECATED` | Omits deprecated APIs for a smaller build |
| `SQLITE_MAX_EXPR_DEPTH=0` | Removes expression depth limit for complex queries |
| `SQLITE_THREADSAFE=2` | Multi-thread mode (each connection must be used in a single thread) |

| Option | Description |
|--------|-------------|
| `-O2` | Optimization level 2 |

**Adding extra definitions from the caller side:**

You can add additional compile definitions from your `CMakeLists.txt` after including `sqlite3.cmake`. The `target_compile_definitions` command is cumulative, so definitions are appended without modifying the original file.

```cmake
include(cmake/sqlite3.cmake)

# Add extra compile definitions to sqlite3 from the caller side
target_compile_definitions(sqlite3 PUBLIC
    SQLITE_ENABLE_LOAD_EXTENSION
)
```

---

### 8. Linking to the Main Project

```cmake
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3)
```

---

## Directory Structure

### After First Build

```
project/
├── download/
│   └── sqlite3/                    # Cache directory
│       ├── sqlite3.c
│       ├── sqlite3.h
│       ├── sqlite3ext.h
│       └── VERSION.txt
├── build/
│   ├── sqlite3_include/            # Headers for conflict avoidance
│   │   ├── sqlite3.h
│   │   └── sqlite3ext.h
│   └── libsqlite3.a                # Built static library
└── CMakeLists.txt
```

---

## Forcing a Re-download

To clear the cache and force a re-download:

```bash
rm -rf download/sqlite3
```

The latest version will be downloaded the next time CMake is configured.

---

## Thread-Safe Modes

Behavior differences based on the `SQLITE_THREADSAFE` value:

| Value | Mode | Description |
|-------|------|-------------|
| `0` | Single-thread | No mutexes (fastest) |
| `1` | Serialized | All operations are serialized (safest) |
| `2` | Multi-thread | Each connection can be used in a separate thread |

---

## Usage

Include from the main `CMakeLists.txt`:

```cmake
include(cmake/sqlite3.cmake)
```

Usage in code:

```cpp
#include <sqlite3.h>

sqlite3* db;
sqlite3_open(":memory:", &db);
// ...
sqlite3_close(db);
```

---

## Dependencies

| Dependency | Required/Optional | Description |
|------------|-------------------|-------------|
| `${PROJECT_NAME}` | Required | Main project target name |
| `FetchContent` | Required | Standard module included in CMake 3.11+ |
| Internet connection | Required (first time only) | Needed for downloading |

---

## Notes

1. **Network dependency**: An internet connection is required for the first build.

2. **Version pinning**: If you want to use a specific version, modify the script to directly specify `SQLITE_URL`.

3. **VERSION file conflict**: This script copies header files to a separate directory to avoid conflicts with the `<version>` header.

4. **Compile options**: `-O2` is for GCC/Clang. Adjustments may be needed for other compilers.

5. **FTS5**: If full-text search is not needed, you can remove `SQLITE_ENABLE_FTS5`.

6. **Proxy environments**: In corporate networks where a proxy is required, set the CMake environment variables (`HTTP_PROXY`/`HTTPS_PROXY`).
