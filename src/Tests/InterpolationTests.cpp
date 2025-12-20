#include <doctest/doctest.h>
#include <nanobench.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>
#include <cstdlib>

#include "Domain/Model/Point.h"
#include "Infrastructure/Interpolation/LinearInterpolator.h"
#include "Infrastructure/Interpolation/AdditionalInterpolators.h"

using namespace Domain::Model;
using namespace Infrastructure::Interpolation;

// Helper to create points
std::vector<Point> createPoints(std::initializer_list<std::pair<double, double>> list) {
    std::vector<Point> points;
    for (auto [x, y] : list) {
        points.push_back({Coord{x}, Coord{y}});
    }
    return points;
}

TEST_CASE("Linear Interpolation") {
    LinearInterpolator interp;
    auto points = createPoints({{0, 0}, {10, 10}});
    auto result = interp.interpolate(points, 20); // 20 steps
    
    CHECK(result.has_value());
    CHECK(result->size() == 20);
    CHECK(result->front().y.value == doctest::Approx(0.0));
    CHECK(result->back().y.value == doctest::Approx(10.0));
    // Midpoint check (roughly)
    CHECK(result->at(10).y.value == doctest::Approx(5.0).epsilon(0.5));
}

/**
 * @file InterpolationTests.cpp
 * @brief Unit tests and Benchmarks for Interpolation Logic.
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
TEST_CASE("Nearest Neighbor Interpolation") {
    NearestNeighborInterpolator interp;
    auto points = createPoints({{0, 0}, {10, 100}});
    auto result = interp.interpolate(points, 10);
    
    CHECK(result.has_value());
    CHECK(result->front().y.value == 0.0);
    CHECK(result->back().y.value == 100.0);
}

TEST_CASE("Lagrange Interpolation") {
    LagrangeInterpolator interp;
    auto points = createPoints({{0, 0}, {5, 25}, {10, 100}}); // y=x^2
    auto result = interp.interpolate(points, 11); // 0, 1, ..., 10
    
    CHECK(result.has_value());
    // Check known point
    double x = 2.0; 
    // Find point close to x=2
    bool found = false;
    for(const auto& p : result.value()) {
        if(std::abs(p.x.value - 2.0) < 0.001) {
            CHECK(p.y.value == doctest::Approx(4.0));
            found = true;
        }
    }
}

TEST_CASE("Step Interpolation") {
    StepInterpolator interp;
    auto points = createPoints({{0, 0}, {5, 50}, {10, 100}});
    auto result = interp.interpolate(points, 20);
    
    CHECK(result.has_value());
    // At x=2.5, it should be 0 (left value)
    for(const auto& p : result.value()) {
        if(std::abs(p.x.value - 2.5) < 0.1) {
           CHECK(p.y.value == 0.0);
        }
        if(std::abs(p.x.value - 7.5) < 0.1) {
           CHECK(p.y.value == 50.0);
        }
    }
}

TEST_CASE("Catmull-Rom Spline") {
    CatmullRomInterpolator interp;
    auto points = createPoints({{-200, -200}, {0, 0}, {10, 10}, {200, 200}}); // Need > 4 points logic or phantom points handled?
    // The implementation requires 4 points minimum.
    auto result = interp.interpolate(points, 20);
    CHECK(result.has_value());
}


TEST_CASE("Cubic Spline Interpolation") {
    CubicSplineInterpolator interp;
    auto points = createPoints({{0,0},{1,1},{2,0},{3,1}});
    auto result = interp.interpolate(points, 20);
    CHECK(result.has_value());
}

TEST_CASE("Akima Spline Interpolation") {
    AkimaSplineInterpolator interp;
    auto points = createPoints({{0,0},{1,0},{2,1},{3,1},{4,0},{5,0}});
    auto result = interp.interpolate(points, 20);
    CHECK(result.has_value());
}

TEST_CASE("Cosine Interpolation") {
    CosineInterpolator interp;
    auto points = createPoints({{0,0},{1,1},{2,0}});
    auto result = interp.interpolate(points, 20);
    CHECK(result.has_value());
}

TEST_CASE("Sinc Interpolation") {
    SincInterpolator interp;
    auto points = createPoints({{0,0},{1,1},{2,0},{3,-1},{4,0}});
    auto result = interp.interpolate(points, 20);
    CHECK(result.has_value());
}

TEST_CASE("Moving Average Smoother") {
    MovingAverageInterpolator interp(3);
    auto points = createPoints({{0,0},{1,10},{2,0},{3,10},{4,0}});
    // Linear resample to 5 pts -> 0, 10, 0, 10, 0. 
    // Moving Avg window 3 at index 2 (val 0) -> (10+0+10)/3 = 6.66
    auto result = interp.interpolate(points, 5); 
    CHECK(result.has_value());
    if(result && result->size() == 5) {
        CHECK(result->at(2).y.value == doctest::Approx(6.666).epsilon(0.1));
    }
}

TEST_CASE("Benchmarks") {
    ankerl::nanobench::Bench bench;
    bench.minEpochIterations(20); // Quick run

    auto densePoints = createPoints({{0,0}});
    for(int i=1; i<100; ++i) densePoints.push_back({Coord{(double)i}, Coord{std::sin(i/10.0)}});

    bench.run("Linear", [&] {
        auto res = LinearInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Nearest", [&] {
        auto res = NearestNeighborInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Step", [&] {
        auto res = StepInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Lagrange (10 pts)", [&] {
        std::vector<Point> small(densePoints.begin(), densePoints.begin() + 10);
        auto res = LagrangeInterpolator().interpolate(small, 1000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Cubic Spline", [&] {
        auto res = CubicSplineInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Akima Spline", [&] {
        auto res = AkimaSplineInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Cosine", [&] {
        auto res = CosineInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Sinc", [&] {
        auto res = SincInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Moving Avg", [&] {
        auto res = MovingAverageInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });
    bench.run("Catmull-Rom", [&] {
        auto res = CatmullRomInterpolator().interpolate(densePoints, 100000);
        ankerl::nanobench::doNotOptimizeAway(res);
    });

    // Generate HTML graph
    std::ofstream out("benchmark_results_optimized.html");
    bench.render(ankerl::nanobench::templates::htmlBoxplot(), out);
    out.close();

    // Automatic open
    #ifdef __linux__
        std::system("google-chrome benchmark_results_optimized.html &");
    #elif _WIN32
        std::system("start google-chrome benchmark_results_optimized.html");
    #endif
}
