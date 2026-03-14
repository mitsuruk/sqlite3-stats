# =============================================================================
# CMake build configuration
#
# Project: [CMake Template Project]
# Author: [mitsuruk]
# Date:    2026/02/12
# License: MIT License
# See LICENSE.md for details.
# =============================================================================
# Minimum CMake version
cmake_minimum_required(VERSION 3.20)

# CMake version diagnostics
message(STATUS "===============================================================")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "===============================================================")
# Current local time
if(APPLE OR UNIX)
    execute_process(
        COMMAND date "+%Y-%m-%d %H:%M:%S"
        OUTPUT_VARIABLE LOCAL_TIME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()
message(STATUS "Local time: ${LOCAL_TIME}")


# Export compile_commands.json (clangd, etc.)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project name
# If this CMakeLists.txt is included via add_subdirectory(), use the directory
# name as the project name (and typically as the executable name). If it is
# configured as the top-level project, use "a.out" as the project name.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(a.out VERSION 0.0.1)
else()
    get_filename_component(DIR_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    project(${DIR_NAME}.out VERSION 0.0.1)
endif()

# ----------------------------
# Directories (diagnostics)
# ----------------------------
message(STATUS "===============================================================")
message(STATUS "Directory Information:")
message(STATUS "CMAKE_SOURCE_DIR         = ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR         = ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR = ${CMAKE_CURRENT_BINARY_DIR}")


# Source root
if(NOT GTEST)
    # Source root
    if(EXISTS ${PROJECT_SOURCE_DIR}/src)
        set(LOCAL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
    else()
        set(LOCAL_SOURCE_DIR ${PROJECT_SOURCE_DIR})
    endif()
else()
    # GoogleTest source root
    set(LOCAL_SOURCE_DIR ${PROJECT_SOURCE_DIR}/test)
endif()

# Sources
file(GLOB SRC
    CONFIGURE_DEPENDS
    "${LOCAL_SOURCE_DIR}/main.cpp"
)
# Exclude extension source files from the main executable
list(FILTER SRC EXCLUDE REGEX "ext_funcs\\.cpp$")

# Warn if no source files found
if(NOT SRC)
    message(WARNING "No source files found in ${LOCAL_SOURCE_DIR}")
    message(WARNING "Please add at least one .cpp/.c/.m/.mm file")
endif()

# Target
add_executable(${PROJECT_NAME} ${SRC})

# ----------------------------
# Compiler info (diagnostics)
# ----------------------------
message(STATUS "===============================================================")
message(STATUS "C/C++ Compiler Information:")
message(STATUS "CMAKE_CXX_COMPILER_ID      = ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "CMAKE_CXX_COMPILER         = ${CMAKE_CXX_COMPILER}")
message(STATUS "CMAKE_C_COMPILER           = ${CMAKE_C_COMPILER}")


# ----------------------------
# Output directories
# ----------------------------
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    set(LOCAL_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
else()
    set(LOCAL_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

message(STATUS "===============================================================")
message(STATUS "PROJECT_BINARY_DIR = ${PROJECT_BINARY_DIR}")


if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
    set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# ----------------------------
# Binary path helper
# ----------------------------
if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(BINARY_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
else()
    set(BINARY_OUTPUT_DIR ${CMAKE_BINARY_DIR})
endif()
message(STATUS "BINARY_OUTPUT_DIR = ${BINARY_OUTPUT_DIR}")

# ----------------------------
# C/C++ Compiler settings
# ----------------------------
# Language standards (C17, C++17)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_17 cxx_std_17)

# Project-wide compile-time constants
# This is an example of how to set project-wide properties.
# Project-wide means globally accessible within the translation unit.
target_compile_definitions(${PROJECT_NAME} PRIVATE
    PROJECT_NAME="${PROJECT_NAME}"
    PROJECT_VERSION="${PROJECT_VERSION}"
    ONE_=1 # This is a numeric constant example.
    TWO_=2 # This is another numeric constant example.
    THREE_=3 # This is yet another numeric constant example.
)

# Example: setting properties for a specific source file
# Use this when you need different compile definitions for specific files.
# For project-wide definitions, use target_compile_definitions() instead..
# The following example sets three compile-time constants for main.cpp:
#
# // Example usage in main.cpp:
# std::cout << "MAIN_FILE_ = " << MAIN_FILE_ << std::endl;
# std::cout << "MSG1 = " << MSG1 << std::endl;
# std::cout << "MSG2 = " << MSG2 << std::endl;
#
set_source_files_properties(
    ${LOCAL_SOURCE_DIR}/main.cpp
    PROPERTIES COMPILE_DEFINITIONS
    # Define three constants for main.cpp
    # Unfortunately, we have to use escaped quotes for string values.
    # And we have to put all definitions in a single line.
    "MAIN_FILE_=1;MSG1=\"MSG1\";MSG2=\"Hello\""
)

# ----------------------------
# Build type
# ----------------------------
if(DEBUG)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Build-type flags
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:-O2 -Wall -funroll-loops>
    $<$<CONFIG:Debug>:-g -O0 -Wall>
)

# Add compile definitions to a target.
# These definitions will be added to the compile command line for every source file in the target.
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:DEBUG_BUILD>
    $<$<CONFIG:Release>:NDEBUG>
)

message(STATUS "===============================================================")
message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

# Encoding options
target_compile_options(${PROJECT_NAME} PRIVATE -finput-charset=UTF-8 -fexec-charset=UTF-8)

# ----------------------------
# Sanitizers (opt-in: -DSANI=true)
# ----------------------------
if(SANI)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        message(STATUS "===============================================================")
        message(WARNING "Sanitizers are only supported with Apple Clang.")
    endif()
    message(STATUS "===============================================================")
    message(STATUS "Sanitizers are enabled.")

    # Compile options
    target_compile_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
        -g
        -O1
    )

    # Link options
    target_link_options(${PROJECT_NAME} PRIVATE
        -fsanitize=address
        -fsanitize=undefined
    )
endif()

# ----------------------------
# Post-build diagnostics
# (Some commands are Apple-specific)
# ----------------------------
if(APPLE)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND "date" "-R" # Get current date
        COMMAND "pwd" # Print working directory
        COMMAND "lipo" "-archs" "${BINARY_OUTPUT_DIR}/${PROJECT_NAME}" # List architectures
        COMMAND "otool" "-L" "${BINARY_OUTPUT_DIR}/${PROJECT_NAME}" # List linked libraries
        COMMENT "${PROJECT_NAME} information"
    )
endif()

# ----------------------------
# Post-build diagnostics
# (Some commands are clangd specific)
# ----------------------------
# copy compile_commands.json back to source tree
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    COMMENT "Copying compile_commands.json to source directory"
)

# remove clangd cache files.
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${PROJECT_SOURCE_DIR}/.cache"
)


# ----------------------------
# Includes
# ----------------------------
if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/src/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src/include)
endif()
if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
endif()

# Common include paths (if present)
if(IS_DIRECTORY /usr/local/include)
    target_include_directories(${PROJECT_NAME} PRIVATE /usr/local/include)
endif()
if(IS_DIRECTORY $ENV{HOME}/include)
    target_include_directories(${PROJECT_NAME} PRIVATE $ENV{HOME}/include)
endif()
if(IS_DIRECTORY ../include)
    target_include_directories(${PROJECT_NAME} PRIVATE ../include)
endif()

# ----------------------------
# Optional integration hooks
# ----------------------------

# Apple-specific Includes and Settings
if(APPLE)
    # Apple metal and Homebrew-specific options
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/apple.cmake)
        message(STATUS "===============================================================")
        message(STATUS "apple.cmake found.")
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/apple.cmake)
    endif()
    # setting information for framework builds for Apple platforms
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/framework.cmake)
        message(STATUS "===============================================================")
        message(STATUS "framework.cmake found.")
        include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/framework.cmake)
    endif()
endif()

# ----------------------------
# Utility: copy_files() helper
# ----------------------------
# Arguments:
#   TARGET_NAME  : unique name for the custom target
#   S_DIR        : source directory
#   FILE_EXT     : extension (e.g. json, txt)
#   TAG_DIR      : destination directory
# ----------------------------
# NOTE: This function uses "cmake -E copy_if_different" which follows symlinks
#       and copies the actual file content, not the symlink itself.
#       If you need to preserve symlinks, use file(CREATE_LINK ... SYMBOLIC) instead.
# ----------------------------
# Example usage (kept commented)
#    copy_files(copy_json_files  "${CMAKE_CURRENT_SOURCE_DIR}/json"  "json" "${BINARY_OUTPUT_DIR}")
#    add_dependencies(${PROJECT_NAME} copy_json_files)
#    copy_files(copy_json2_files "${CMAKE_CURRENT_SOURCE_DIR}/json2" "json" "${BINARY_OUTPUT_DIR}")
#    add_dependencies(${PROJECT_NAME} copy_json2_files)
function(copy_files TARGET_NAME S_DIR FILE_EXT TAG_DIR)
    file(GLOB TARGET_FILES "${S_DIR}/*.${FILE_EXT}")

    if(NOT TARGET_FILES)
        message(WARNING "No .${FILE_EXT} files found in ${S_DIR}")
        return()
    endif()

    set(COPIED_TARGET_FILES "")

    foreach(TARGET_FILE ${TARGET_FILES})
        get_filename_component(TARGET_FILENAME ${TARGET_FILE} NAME)
        set(OUTPUT_FILE "${TAG_DIR}/${TARGET_FILENAME}")

        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${TARGET_FILE} ${OUTPUT_FILE}
            DEPENDS ${TARGET_FILE}
            COMMENT "Copy ${TARGET_FILENAME} -> ${TAG_DIR}"
        )
        list(APPEND COPIED_TARGET_FILES ${OUTPUT_FILE})
    endforeach()

    add_custom_target(${TARGET_NAME} ALL
        DEPENDS ${COPIED_TARGET_FILES}
        COMMENT "Ensure .${FILE_EXT} files are copied to ${TAG_DIR}"
    )
endfunction()

# ----------------------------
# PkgConfig helper (kept API, trimmed comments)
# ----------------------------
# Example usage (kept commented)
#    find_pkg_config(${PROJECT_NAME} PRIVATE cairo)
#    find_pkg_config(${PROJECT_NAME} PRIVATE gtk+-3.0)
function(find_pkg_config target)
    find_package(PkgConfig REQUIRED)
    if(NOT PkgConfig_FOUND)
        message(FATAL_ERROR "PkgConfig is required to use find_pkg_config() helper.")
    endif()

    # Scope defaults to PRIVATE unless explicitly set
    set(scope "PRIVATE")
    set(package "")

    list(GET ARGV 1 first_arg)
    if(first_arg STREQUAL "PRIVATE" OR first_arg STREQUAL "PUBLIC" OR first_arg STREQUAL "INTERFACE")
        set(scope "${first_arg}")
        list(REMOVE_AT ARGV 1)
    endif()

    list(GET ARGV 1 package)
    if(NOT package)
        message(FATAL_ERROR "Package name is required after the scope")
    endif()

    string(TOUPPER ${package} PACKAGE_NAME)
    pkg_check_modules(${PACKAGE_NAME} REQUIRED ${package})

    target_include_directories(${target} ${scope} ${${PACKAGE_NAME}_INCLUDE_DIRS})
    target_link_libraries(${target} ${scope} ${${PACKAGE_NAME}_LIBRARIES})

    # Expose for callers
    set(${package}_INCLUDE_DIRS ${${PACKAGE_NAME}_INCLUDE_DIRS} PARENT_SCOPE)
    set(${package}_LIBRARY_DIRS ${${PACKAGE_NAME}_LIBRARY_DIRS} PARENT_SCOPE)
    set(${package}_LIBRARIES ${${PACKAGE_NAME}_LIBRARIES} PARENT_SCOPE)

    # Diagnostics
    message(STATUS "===============================================================")
    message(STATUS "${package} include dirs : ${${PACKAGE_NAME}_INCLUDE_DIRS}")
    message(STATUS "${package} library dirs : ${${PACKAGE_NAME}_LIBRARY_DIRS}")
    message(STATUS "${package} libraries    : ${${PACKAGE_NAME}_LIBRARIES}")
    message(STATUS "Link scope             : ${scope}")
endfunction()

# ----------------------------
# Graphviz generation
# ----------------------------
# if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
#     add_custom_target(generate_graphviz_${PROJECT_NAME}
#         COMMAND ${CMAKE_COMMAND} -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_BINARY_DIR}
#                 --graphviz=${CMAKE_BINARY_DIR}/graphviz/${PROJECT_NAME}.dot
#         COMMENT "Generate Graphviz dependency graph"
#     )
#     if(TARGET ${PROJECT_NAME})
#         add_dependencies(${PROJECT_NAME} generate_graphviz_${PROJECT_NAME})
#     endif()
# endif()

# ----------------------------
# GoogleTest (opt-in: -DGTEST=true)
# ----------------------------
if(GTEST)
    find_package(GTest REQUIRED)

    # Enable testing for this project
    enable_testing()

    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE
        # Examples (kept as a reference)
        # GTest::gtest
        # GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "GTEST")

    # Register the test executable with CTest
    add_test(
        NAME ${PROJECT_NAME}_tests
        COMMAND ${PROJECT_NAME}
        WORKING_DIRECTORY ${BINARY_OUTPUT_DIR}
    )

    # Optional: Set test properties
    set_tests_properties(${PROJECT_NAME}_tests PROPERTIES
        TIMEOUT 60  # 60 seconds timeout
        ENVIRONMENT "GTEST_COLOR=1"  # Enable colored output
    )

    #[[
        If you want to use GoogleTest, run:
          cmake -DGTEST=true ..
          cmake --build .
          ctest  # or ctest --verbose

        You can also run the test executable directly:
          ./${PROJECT_NAME}

        You can also add this to main.cpp:

        #include <gtest/gtest.h>
        #include <gmock/gmock.h>
        int main(int argc, char **argv) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    #]]
endif()

# setting information for install rules
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/install.cmake)
#     message(STATUS "===============================================================")
#     message(STATUS "install.cmake found.")
#     include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/install.cmake)
# endif()

# setting information for boost integration (if boost.cmake is present)
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/boost.cmake)
#     message(STATUS "===============================================================")
#     message(STATUS "boost.cmake found.")
#     include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/boost.cmake)
# endif()

# setting external code generators (Flex/Bison/gRPC/ANTLR)
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CodeGenerators.cmake)
#     message(STATUS "===============================================================")
#     message(STATUS "CodeGenerators.cmake found.")
#     include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CodeGenerators.cmake)
# endif()

# setting information for sqlite3 integration (if sqlite3.cmake is present)
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sqlite3.cmake)
#     message(STATUS "===============================================================")
#     message(STATUS "sqlite3.cmake found.")
#     include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/sqlite3.cmake)
# endif()

# setting information for dlib integration (if dlib.cmake is present)
# if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/dlib.cmake)
#     message(STATUS "===============================================================")
#     message(STATUS "dlib.cmake found.")
#     include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dlib.cmake)
# endif()


# ----------------------------
# Homebrew helper (kept API, comments trimmed)
# ----------------------------
# link_latest_package(PACKAGE_NAME LIB_FILES)
#   PACKAGE_NAME: Cellar dir name (e.g., libomp, boost, opencv)
#   LIB_FILES   : one or more libs (e.g., libomp.dylib or libboost_system.dylib;libboost_filesystem.dylib)
#   NOTE       : For cases where the library is installed via Homebrew but
#                CMake's find_package()/pkg-config cannot provide usable
#                configuration, this helper manually wires include/lib paths
#                and links the specified libraries.
if(APPLE)
    function(link_latest_package PACKAGE_NAME LIB_FILES)
        execute_process(COMMAND brew --prefix OUTPUT_VARIABLE BREW_DIR ERROR_QUIET)
        string(STRIP "${BREW_DIR}" BREW_DIR)
        set(PKG_BASE_DIR "${BREW_DIR}/Cellar/${PACKAGE_NAME}")
        message(STATUS "Link latest package: ${PACKAGE_NAME} from ${PKG_BASE_DIR}")

        file(GLOB VERSION_DIRS "${PKG_BASE_DIR}/*/")
        set(LATEST_DIR "")
        set(LATEST_VERSION "0.0.0")

        foreach(DIR ${VERSION_DIRS})
            get_filename_component(DIR_NAME ${DIR} NAME)
            if(DIR_NAME VERSION_GREATER LATEST_VERSION)
                set(LATEST_VERSION ${DIR_NAME})
                set(LATEST_DIR ${DIR})
            endif()
        endforeach()

        if(NOT LATEST_DIR)
            message(FATAL_ERROR "No valid version directory in ${PKG_BASE_DIR}")
        endif()

        message(STATUS "Using ${PACKAGE_NAME} version: ${LATEST_VERSION}")

        target_include_directories(${PROJECT_NAME} PRIVATE ${LATEST_DIR}/include/)
        target_link_directories(${PROJECT_NAME} PRIVATE ${LATEST_DIR}/lib/)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB_FILES})
    endfunction()
endif()


message(STATUS "===============================================================")
message(STATUS "CMake configuration completed.")
message(STATUS "That's all folks!")
message(STATUS "===============================================================")

# ----------------------------
# End
# ----------------------------

# That's all folks! Enjoy coding!
include("./cmake/statcpp.cmake")
include("./cmake/sqlite3.cmake")

target_compile_definitions(sqlite3 PUBLIC
    SQLITE_ENABLE_LOAD_EXTENSION
)

# ----------------------------
# SQLite3 Loadable Extension (shared library)
# ----------------------------
# Build ext_funcs.cpp as a shared library (C++ required for PicoSHA2)
# that can be loaded at runtime via sqlite3_load_extension()
add_library(ext_funcs SHARED ${PROJECT_SOURCE_DIR}/src/ext_funcs.cpp)

# statcpp requires C++17 (std::invoke, etc.)
target_compile_features(ext_funcs PRIVATE cxx_std_17)

# The extension library references sqlite3 headers and src/include (picosha2.h)
target_link_libraries(ext_funcs PRIVATE sqlite3)
target_include_directories(ext_funcs PRIVATE
    ${PROJECT_SOURCE_DIR}/src/include
    ${STATCPP_INSTALL_DIR}/include
)

# macOS: remove the "lib" prefix and use .dylib extension
# Linux: remove the "lib" prefix and use .so extension
set_target_properties(ext_funcs PROPERTIES
    PREFIX ""
    OUTPUT_NAME "ext_funcs"
)

# Place the extension library in the same directory as the executable
set_target_properties(ext_funcs PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)

# Pass the extension library path to the main executable at compile time
target_compile_definitions(${PROJECT_NAME} PRIVATE
    EXT_FUNCS_PATH="${BINARY_OUTPUT_DIR}/ext_funcs$<IF:$<PLATFORM_ID:Darwin>,.dylib,$<IF:$<PLATFORM_ID:Windows>,.dll,.so>>"
)

# Build the extension library before the main executable
add_dependencies(${PROJECT_NAME} ext_funcs)
