cmake_minimum_required(VERSION 3.28)
project(InterpolationApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# clang-tidy reads this: without it the FetchContent headers are not found.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# The window is the only part that needs Qt. recipe.ps1 switches it off, so the
# computational core builds and tests on a machine with no Qt.
option(INTERPOLATION_APP_WINDOW "Build the Qt window" ON)
option(BUILD_TESTING "Build tests" ON)

function(warn_as_errors target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

if(INTERPOLATION_APP_WINDOW)
    set(CMAKE_AUTOMOC ON)
    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Charts)

    add_executable(InterpolationApp
        src/Presentation/Main.cpp
        src/Presentation/MainWindow.cpp
    )
    target_link_libraries(InterpolationApp PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Charts
    )
    target_include_directories(InterpolationApp PRIVATE src)
    warn_as_errors(InterpolationApp)
endif()

if(BUILD_TESTING)
    enable_testing()

    # An archive and its hash, not a git clone: nothing here can ask for a credential on the runner.
    include(FetchContent)
    FetchContent_Declare(
        doctest
        URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.12.tar.gz
        URL_HASH SHA256=73381c7aa4dee704bd935609668cf41880ea7f19fa0504a200e13b74999c2d70
        SYSTEM
    )
    FetchContent_Declare(
        nanobench
        URL https://github.com/martinus/nanobench/archive/refs/tags/v4.3.11.tar.gz
        URL_HASH SHA256=53a5a913fa695c23546661bf2cd22b299e10a3e994d9ed97daf89b5cada0da70
        SYSTEM
    )
    FetchContent_MakeAvailable(doctest nanobench)

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
    warn_as_errors(tests_run)

    add_test(NAME interpolation_tests COMMAND tests_run)
    # The manual list is judged where every other file is judged (section 9).
    add_test(
        NAME manual_list
        COMMAND python -m pytest tests/test_manual_list.py -q -p no:cacheprovider
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endif()
