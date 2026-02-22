# statcpp.cmake Reference

## Overview

`statcpp.cmake` is a CMake configuration file that automatically downloads and configures the statcpp library.
It uses CMake's `file(DOWNLOAD)` to download the repository archive from GitHub, with caching in the `download/` directory to avoid redundant downloads.

statcpp is a modern, header-only C++17 statistics library. It provides 524 public functions across 31 modules, covering descriptive statistics, hypothesis testing, regression analysis, ANOVA, probability distributions, resampling, clustering, and more. All functions use an STL-style iterator-based API with projection support.

Since statcpp is header-only, no compilation or linking is required. Only the include path needs to be configured.

## File Information

| Item | Details |
|------|---------|
| Install Directory | `${CMAKE_CURRENT_SOURCE_DIR}/download/statcpp/statcpp-install` |
| Download URL | https://github.com/mitsuruk/statcpp/archive/refs/heads/main.tar.gz |
| Version | 0.1.0 |
| License | MIT License |

---

## Include Guard

```cmake
include_guard(GLOBAL)
```

This file uses `include_guard(GLOBAL)` to ensure it is only executed once, even if included multiple times.

**Why it's needed:**

- Prevents duplicate `file(DOWNLOAD)` invocations during configure
- Prevents duplicate `target_include_directories` calls

---

## Directory Structure

```
project/
├── cmake/
│   ├── statcpp.cmake         # This configuration file
│   ├── statcppCmake.md       # This document
│   └── statcppCmake-jp.md    # Japanese version
├── download/statcpp/
│   ├── statcpp-main.tar.gz   # Cached archive
│   └── statcpp-install/      # Installed headers
│       └── include/
│           └── statcpp/
│               ├── statcpp.hpp               # Master include
│               ├── basic_statistics.hpp       # Mean, median, mode, ...
│               ├── parametric_tests.hpp       # t-test, z-test, F-test, ...
│               ├── linear_regression.hpp      # Simple & multiple regression
│               ├── anova.hpp                  # One-way, two-way, repeated measures
│               ├── continuous_distributions.hpp
│               └── ... (31 header files total)
├── src/
│   └── main.cpp
├── build/
└── CMakeLists.txt
```

## Usage

### Adding to CMakeLists.txt

```cmake
include("./cmake/statcpp.cmake")
```

### Build

```bash
mkdir build && cd build
cmake ..
make
```

---

## Processing Flow

### 1. Setting the Directory Paths

```cmake
set(STATCPP_DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/download/statcpp)
set(STATCPP_INSTALL_DIR ${STATCPP_DOWNLOAD_DIR}/statcpp-install)
set(STATCPP_VERSION "0.1.0")
set(STATCPP_BRANCH "main")
set(STATCPP_URL "https://github.com/mitsuruk/statcpp/archive/refs/heads/${STATCPP_BRANCH}.tar.gz")
```

### 2. Cache Check and Conditional Download

```cmake
if(EXISTS ${STATCPP_INSTALL_DIR}/include/statcpp/statcpp.hpp)
    message(STATUS "statcpp already installed")
else()
    # Download and install ...
endif()
```

The cache logic works as follows:

| Condition | Action |
|-----------|--------|
| `statcpp-install/include/statcpp/statcpp.hpp` exists | Skip everything (use cached) |
| `statcpp-main.tar.gz` exists (install missing) | Skip download, extract and install |
| Nothing exists | Download from GitHub, extract and install |

### 3. Download (if needed)

```cmake
file(DOWNLOAD
    ${STATCPP_URL}
    ${STATCPP_CACHED_ARCHIVE}
    SHOW_PROGRESS
    TIMEOUT 120
    INACTIVITY_TIMEOUT 30
    STATUS DOWNLOAD_STATUS
)
```

- Downloads the repository archive (`.tar.gz`) from GitHub
- Contains all 31 header files in `include/statcpp/`

### 4. Extract and Install

```cmake
file(ARCHIVE_EXTRACT
    INPUT ${STATCPP_CACHED_ARCHIVE}
    DESTINATION ${STATCPP_EXTRACT_DIR}
)

file(COPY ${STATCPP_EXTRACTED_DIR}/include/statcpp
    DESTINATION ${STATCPP_INSTALL_DIR}/include
)
```

- Extracts the archive to a temporary directory
- Copies the `include/statcpp/` directory to the install location
- Cleans up the temporary extraction directory
- No compilation step is needed (header-only library)

### 5. Configuring Include Path

```cmake
target_include_directories(${PROJECT_NAME} PRIVATE
    ${STATCPP_INSTALL_DIR}/include
)
```

statcpp is header-only. No `add_library`, `target_link_libraries`, or static library creation is needed.

---

## statcpp Library

statcpp consists of 31 header files organized into modules:

### Header Files

| Category | Header | Description |
|----------|--------|-------------|
| **Descriptive** | `basic_statistics.hpp` | Mean, median, mode, trimmed mean, etc. |
| | `order_statistics.hpp` | Quartiles, percentiles, quantiles |
| | `dispersion_spread.hpp` | Variance, std deviation, range, IQR |
| | `shape_of_distribution.hpp` | Skewness, kurtosis |
| | `correlation_covariance.hpp` | Pearson, Spearman, Kendall correlation |
| | `frequency_distribution.hpp` | Histograms, frequency tables |
| **Probability** | `special_functions.hpp` | Gamma, beta, error functions |
| | `random_engine.hpp` | Random number generation |
| | `continuous_distributions.hpp` | Normal, t, chi-square, F, etc. |
| | `discrete_distributions.hpp` | Binomial, Poisson, etc. |
| **Inference** | `estimation.hpp` | Confidence intervals |
| | `parametric_tests.hpp` | t-test, z-test, F-test |
| | `nonparametric_tests.hpp` | Wilcoxon, Mann-Whitney |
| | `effect_size.hpp` | Cohen's d, Hedges' g |
| | `resampling.hpp` | Bootstrap, jackknife, permutation |
| | `power_analysis.hpp` | Sample size & power calculations |
| **Modeling** | `linear_regression.hpp` | Simple & multiple regression |
| | `anova.hpp` | One-way, two-way, repeated measures ANOVA |
| | `glm.hpp` | Logistic regression, GLMs |
| | `model_selection.hpp` | AIC, BIC, cross-validation |
| **Applied** | `clustering.hpp` | K-means, hierarchical clustering |
| | `distance_metrics.hpp` | Euclidean, Manhattan, cosine distance |
| | `categorical.hpp` | Categorical data analysis |
| | `data_wrangling.hpp` | Data manipulation utilities |
| | `missing_data.hpp` | Missing data handling |
| | `multivariate.hpp` | Multivariate analysis |
| | `robust.hpp` | Robust statistics |
| | `survival.hpp` | Survival analysis |
| | `time_series.hpp` | Time series analysis |
| **Utility** | `numerical_utils.hpp` | Numerical helpers |
| **Master** | `statcpp.hpp` | Includes all modules |

---

## Key Features of statcpp

| Feature | Description |
|---------|-------------|
| Header-only | No compilation or linking required |
| C++17 | Uses modern C++17 features |
| 524 Functions | Comprehensive coverage of statistical methods |
| STL-style API | Iterator-based interface (`begin`, `end`) |
| Projection Support | Direct struct member processing via projections |
| R-verified | 167 numerical checks validated against R 4.4.2 |
| 758 Unit Tests | Tested with Google Test framework |
| Cross-platform | macOS, Linux |

---

## Usage Examples in C++

### Include All Modules

```cpp
#include <statcpp/statcpp.hpp>
```

### Include Specific Modules

```cpp
#include <statcpp/basic_statistics.hpp>
#include <statcpp/parametric_tests.hpp>
```

### Basic Statistics

```cpp
#include <statcpp/basic_statistics.hpp>
#include <vector>
#include <iostream>

int main() {
    std::vector<double> data = {2.0, 4.0, 6.0, 8.0, 10.0};

    double m = statcpp::mean(data.begin(), data.end());
    double med = statcpp::median(data.begin(), data.end());

    std::cout << "Mean: " << m << "\n";    // 6.0
    std::cout << "Median: " << med << "\n"; // 6.0

    return 0;
}
```

### Hypothesis Testing

```cpp
#include <statcpp/parametric_tests.hpp>
#include <vector>
#include <iostream>

int main() {
    std::vector<double> group1 = {5.1, 4.9, 5.3, 5.0, 5.2};
    std::vector<double> group2 = {4.8, 4.6, 4.7, 4.5, 4.9};

    auto result = statcpp::t_test_two_sample(
        group1.begin(), group1.end(),
        group2.begin(), group2.end()
    );

    std::cout << "t-statistic: " << result.statistic << "\n";
    std::cout << "p-value: " << result.p_value << "\n";

    return 0;
}
```

### Linear Regression

```cpp
#include <statcpp/linear_regression.hpp>
#include <vector>
#include <iostream>

int main() {
    std::vector<double> x = {1, 2, 3, 4, 5};
    std::vector<double> y = {2.1, 3.9, 6.2, 7.8, 10.1};

    auto result = statcpp::simple_linear_regression(
        x.begin(), x.end(),
        y.begin(), y.end()
    );

    std::cout << "Intercept: " << result.intercept << "\n";
    std::cout << "Slope: " << result.slope << "\n";
    std::cout << "R-squared: " << result.r_squared << "\n";

    return 0;
}
```

### Projection Support

```cpp
#include <statcpp/basic_statistics.hpp>
#include <vector>
#include <iostream>

struct Product {
    std::string name;
    double price;
};

int main() {
    std::vector<Product> products = {
        {"Apple", 1.5}, {"Banana", 0.8}, {"Cherry", 3.0}
    };

    double avg = statcpp::mean(
        products.begin(), products.end(),
        [](const Product& p) { return p.price; }
    );

    std::cout << "Average price: " << avg << "\n"; // 1.76667

    return 0;
}
```

### Compilation

```bash
g++ -std=c++17 -I download/statcpp/statcpp-install/include example.cpp -o example
./example
```

---

## statcpp API Conventions

### Namespace

All functionality is in the `statcpp` namespace:

```cpp
double m = statcpp::mean(data.begin(), data.end());
auto result = statcpp::t_test_two_sample(g1.begin(), g1.end(), g2.begin(), g2.end());
```

### Iterator-based API

Most functions accept iterator pairs:

```cpp
statcpp::mean(first, last);                   // Basic form
statcpp::mean(first, last, projection);       // With projection
```

### Return Types

| Type | Used By |
|------|---------|
| `double` | Scalar statistics (mean, variance, etc.) |
| Struct results | Tests, regression (`.statistic`, `.p_value`, `.r_squared`, etc.) |

---

## Troubleshooting

### Download Fails

If GitHub is unreachable, you can manually download and place the archive:

```bash
curl -L -o download/statcpp/statcpp-main.tar.gz \
    https://github.com/mitsuruk/statcpp/archive/refs/heads/main.tar.gz
```

Then re-run `cmake ..` and the installation will proceed from the cached archive.

### Rebuild from Scratch

To force a fresh download and install:

```bash
rm -rf download/statcpp
cd build && cmake ..
```

### Header Not Found

Ensure the include directory is correctly configured:

```cmake
target_include_directories(${PROJECT_NAME} PRIVATE ${STATCPP_INSTALL_DIR}/include)
```

The headers should be included as:

```cpp
#include <statcpp/statcpp.hpp>            // OK (all modules)
#include <statcpp/basic_statistics.hpp>   // OK (specific module)
// #include "statcpp.hpp"                 // NG (wrong path)
```

### C++17 Required

statcpp requires C++17 or later. Ensure your CMakeLists.txt includes:

```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

---

## References

- [statcpp GitHub Repository](https://github.com/mitsuruk/statcpp)
- [CMake file(DOWNLOAD) Documentation](https://cmake.org/cmake/help/latest/command/file.html#download)
- [CMake file(ARCHIVE_EXTRACT) Documentation](https://cmake.org/cmake/help/latest/command/file.html#archive-extract)
