# =============================================================================
# statcpp CMake configuration
#
# This file configures the statcpp header-only statistics library for the project.
# statcpp is a C++17 header-only library providing comprehensive statistical
# functions: descriptive statistics, hypothesis testing, regression, ANOVA,
# distributions, resampling, clustering, and more (524 functions, 31 modules).
#
# Download directory: ${CMAKE_CURRENT_SOURCE_DIR}/download/statcpp
# Install directory:  ${CMAKE_CURRENT_SOURCE_DIR}/download/statcpp/statcpp-install
#
# - If statcpp-install/include/statcpp/statcpp.hpp already exists,
#   skip download (reuse cache).
# - Otherwise, download the repository archive from GitHub and install headers.
#
# Note: statcpp is header-only. No compilation or linking is needed.
#
# License: MIT License (this cmake file)
# Note: statcpp library itself is licensed under MIT License.
# =============================================================================

include_guard(GLOBAL)

message(STATUS "===============================================================")
message(STATUS "statcpp configuration:")

# Path to download/install directories
set(STATCPP_DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/download/statcpp)
set(STATCPP_INSTALL_DIR ${STATCPP_DOWNLOAD_DIR}/statcpp-install)
set(STATCPP_VERSION "0.1.0")
set(STATCPP_BRANCH "main")
set(STATCPP_URL "https://github.com/mitsuruk/statcpp/archive/refs/heads/${STATCPP_BRANCH}.tar.gz")

message(STATUS "STATCPP_INSTALL_DIR = ${STATCPP_INSTALL_DIR}")

# =============================================================================
# statcpp: Download and Install (cached in download/ directory)
# =============================================================================
if(EXISTS ${STATCPP_INSTALL_DIR}/include/statcpp/statcpp.hpp)
    message(STATUS "statcpp already installed: ${STATCPP_INSTALL_DIR}/include/statcpp/statcpp.hpp")
else()
    # Create install directory structure
    file(MAKE_DIRECTORY ${STATCPP_INSTALL_DIR}/include)

    # Check if archive is already cached in download/
    set(STATCPP_CACHED_ARCHIVE ${STATCPP_DOWNLOAD_DIR}/statcpp-${STATCPP_BRANCH}.tar.gz)

    if(EXISTS ${STATCPP_CACHED_ARCHIVE})
        message(STATUS "statcpp archive already cached: ${STATCPP_CACHED_ARCHIVE}")
    else()
        file(MAKE_DIRECTORY ${STATCPP_DOWNLOAD_DIR})
        message(STATUS "Downloading statcpp (${STATCPP_BRANCH}) from GitHub ...")
        file(DOWNLOAD
            ${STATCPP_URL}
            ${STATCPP_CACHED_ARCHIVE}
            SHOW_PROGRESS
            TIMEOUT 120
            INACTIVITY_TIMEOUT 30
            STATUS DOWNLOAD_STATUS
        )
        list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
        if(NOT DOWNLOAD_RESULT EQUAL 0)
            list(GET DOWNLOAD_STATUS 1 DOWNLOAD_ERROR)
            file(REMOVE ${STATCPP_CACHED_ARCHIVE})
            message(FATAL_ERROR
                "statcpp download failed: ${DOWNLOAD_ERROR}\n"
                "You can manually download and place the file:\n"
                "  curl -L -o download/statcpp/statcpp-${STATCPP_BRANCH}.tar.gz ${STATCPP_URL}\n"
                "Then re-run cmake."
            )
        endif()
    endif()

    # Extract archive to a temporary directory
    set(STATCPP_EXTRACT_DIR ${STATCPP_DOWNLOAD_DIR}/_extract)
    file(REMOVE_RECURSE ${STATCPP_EXTRACT_DIR})
    file(MAKE_DIRECTORY ${STATCPP_EXTRACT_DIR})
    file(ARCHIVE_EXTRACT
        INPUT ${STATCPP_CACHED_ARCHIVE}
        DESTINATION ${STATCPP_EXTRACT_DIR}
    )

    # Find the extracted directory (statcpp-main/ or similar)
    file(GLOB STATCPP_EXTRACTED_DIRS "${STATCPP_EXTRACT_DIR}/statcpp-*")
    list(LENGTH STATCPP_EXTRACTED_DIRS STATCPP_DIRS_COUNT)
    if(STATCPP_DIRS_COUNT EQUAL 0)
        message(FATAL_ERROR "statcpp archive extraction failed: no directory found")
    endif()
    list(GET STATCPP_EXTRACTED_DIRS 0 STATCPP_EXTRACTED_DIR)

    # Copy headers to install directory (include/statcpp/)
    if(EXISTS ${STATCPP_EXTRACTED_DIR}/include/statcpp)
        file(COPY ${STATCPP_EXTRACTED_DIR}/include/statcpp
            DESTINATION ${STATCPP_INSTALL_DIR}/include
        )
    else()
        message(FATAL_ERROR "statcpp headers not found in extracted archive")
    endif()

    # Clean up extraction directory
    file(REMOVE_RECURSE ${STATCPP_EXTRACT_DIR})

    # Verify installation
    if(NOT EXISTS ${STATCPP_INSTALL_DIR}/include/statcpp/statcpp.hpp)
        message(FATAL_ERROR "statcpp installation failed")
    endif()

    message(STATUS "statcpp ${STATCPP_VERSION} installed successfully")
endif()

# =============================================================================
# statcpp Library Configuration
# =============================================================================
# Header-only: just add include directory, no linking needed
target_include_directories(${PROJECT_NAME} PRIVATE
    ${STATCPP_INSTALL_DIR}/include
)

message(STATUS "statcpp headers added to ${PROJECT_NAME}")
message(STATUS "===============================================================")
