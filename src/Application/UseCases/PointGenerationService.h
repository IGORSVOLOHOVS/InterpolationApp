#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include <numbers>
#include "Domain/Model/Point.h"

namespace Application::UseCases {

using Domain::Model::Point;
using Domain::Model::Coord;

/**
 * @brief Service responsible for generating sample data points.
 */
class PointGenerationService {
public:
    /**
     * @brief Generates random 2D points within specified bounds.
     * @details Points are sorted by X coordinate to be suitable for interpolation.
     * 
     * @param count Number of points to generate.
     * @param minX Minimum X value.
     * @param maxX Maximum X value.
     * @param minY Minimum Y value.
     * @param maxY Maximum Y value.
     * @return std::vector<Point> Vector of generated Sort points.
     */
    static std::vector<Point> generateRandomPoints(int count, double minX, double maxX, double minY, double maxY) {
        if (count <= 0) return {};
        
        std::vector<Point> points;
        points.reserve(count);
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> disX(minX, maxX);
        std::uniform_real_distribution<> disY(minY, maxY);
        
        for(int i=0; i<count; ++i) {
            points.push_back({Coord{disX(gen)}, Coord{disY(gen)}});
        }
        
        // Usually we want sorted X for interpolation inputs
        std::ranges::sort(points, [](const Point& a, const Point& b) { return a.x < b.x; });
        
        return points;
    }

    /**
     * @brief Generates points following a sine wave pattern.
     * 
     * @param count Number of points.
     * @param minX Start X.
     * @param maxX End X.
     * @return std::vector<Point> Vector of points on a sine curve.
     */
    static std::vector<Point> generateSineWave(int count, double minX, double maxX) {
        if (count <= 0) return {};
        std::vector<Point> points;
        double step = (maxX - minX) / (count - 1);
        for(int i=0; i<count; ++i) {
            double x = minX + i*step;
            points.push_back({Coord{x}, Coord{std::sin(x)}});
        }
        return points;
    }
};

} // namespace Application::UseCases
