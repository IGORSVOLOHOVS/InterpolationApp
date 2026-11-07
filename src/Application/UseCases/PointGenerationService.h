#ifndef INTERPOLATION_APP_APPLICATION_POINT_GENERATION_SERVICE_H
#define INTERPOLATION_APP_APPLICATION_POINT_GENERATION_SERVICE_H

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>
#include <vector>

#include "Domain/Model/Point.h"

namespace Application::UseCases {

using Domain::Model::Coord;
using Domain::Model::Point;

/**
 * @brief The rectangle random points are drawn from.
 * @details One named argument: four adjacent doubles are easily swapped by mistake.
 */
struct PointBounds {
    /// Minimum X value.
    double minX;
    /// Maximum X value.
    double maxX;
    /// Minimum Y value.
    double minY;
    /// Maximum Y value.
    double maxY;
};

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
     * @param bounds The rectangle to draw from.
     * @return std::vector<Point> Vector of generated Sort points.
     */
    static std::vector<Point> generateRandomPoints(int count, const PointBounds& bounds) {
        if (count <= 0) return {};

        std::vector<Point> points;
        points.reserve(count);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> disX(bounds.minX, bounds.maxX);
        std::uniform_real_distribution<> disY(bounds.minY, bounds.maxY);

        for (int i = 0; i < count; ++i) {
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
        for (int i = 0; i < count; ++i) {
            double x = minX + i * step;
            points.push_back({Coord{x}, Coord{std::sin(x)}});
        }
        return points;
    }
};

}  // namespace Application::UseCases

#endif  // INTERPOLATION_APP_APPLICATION_POINT_GENERATION_SERVICE_H
