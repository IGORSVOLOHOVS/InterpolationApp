cmake_minimum_required(VERSION 3.28)
project(InterpolationApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)

# Dependencies
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts)

# Testing & Benchmarking setup
option(BUILD_TESTING "Build tests" ON)

if(BUILD_TESTING)
    enable_testing()
    
    # Download dependencies if not present (simplified for lab mode)
    # in a real scenario we might use FetchContent
    include(FetchContent)
    
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
    )
    FetchContent_MakeAvailable(doctest)

    FetchContent_Declare(
        nanobench
        GIT_REPOSITORY https://github.com/martinus/nanobench.git
        GIT_TAG v4.3.11
    )
    FetchContent_MakeAvailable(nanobench)
endif()

# Sources
add_executable(InterpolationApp
    src/Presentation/Main.cpp
    src/Presentation/MainWindow.cpp
    # Add other sources here as we create them
)

target_link_libraries(InterpolationApp PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Charts
)

target_include_directories(InterpolationApp PRIVATE src)

# Compiler Warnings
if(MSVC)
    target_compile_options(InterpolationApp PRIVATE /W4 /WX)
else()
    target_compile_options(InterpolationApp PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Test Runner Generation
if(BUILD_TESTING)
    set(GEN_RUNNER "${CMAKE_BINARY_DIR}/tests_run.cpp")
    file(WRITE ${GEN_RUNNER} "
        #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
        #include <doctest/doctest.h>
        #define ANKERL_NANOBENCH_IMPLEMENT
        #include <nanobench.h>
    ")
    
    add_executable(tests_run
        ${GEN_RUNNER}
        src/Tests/InterpolationTests.cpp
    )
    
    target_link_libraries(tests_run PRIVATE doctest::doctest nanobench::nanobench)
    target_include_directories(tests_run PRIVATE src)
    target_compile_features(tests_run PRIVATE cxx_std_23)
endif()
