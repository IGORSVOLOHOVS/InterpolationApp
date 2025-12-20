#pragma once

#include <vector>
#include <algorithm>
#include <expected>
#include "Domain/Model/Point.h"
#include "Domain/Ports/Interpolator.h"

namespace Infrastructure::Interpolation {

using Domain::Model::Point;
using Domain::Model::Coord;
using Domain::Ports::DomainError;

/**
 * @brief Implements Linear Interpolation strategy.
 * @details Connects points with straight lines. Simple and efficient but not smooth.
 */
class LinearInterpolator {
public:
    /**
     * @brief Performs linear interpolation on the given points.
     * 
     * @param points Input control points (must be at least 2).
     * @param steps Total number of points to generate.
     * @return std::expected<std::vector<Point>, DomainError> Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
        if (points.size() < 2) {
            return std::unexpected(DomainError{"At least two points are required for linear interpolation."});
        }
        if (steps <= 0) {
             return std::unexpected(DomainError{"Steps must be positive."});
        }

        std::vector<Point> result;
        // Total points in result includes the start and end of each segment
        // Simple approach: generating 'steps' points between each pair?
        // Or resampling the whole range into 'steps' total points?
        // User request: "applied one of 10 methods... graphic points after".
        // Usually interpolation typically means resampling.
        // Let's assume we want to generate 'steps' points uniformly distributed over the X range.

        auto minX = std::ranges::min_element(points, [](const Point& a, const Point& b) { return a.x < b.x; })->x.value;
        auto maxX = std::ranges::max_element(points, [](const Point& a, const Point& b) { return a.x < b.x; })->x.value;
        
        double stepSize = (maxX - minX) / (steps - 1);
        
        // Ensure points are sorted by X for correct interpolation
        std::vector<Point> sortedPoints = points;
        std::ranges::sort(sortedPoints, [](const Point& a, const Point& b) { return a.x < b.x; });

        for (int i = 0; i < steps; ++i) {
            double currentX = minX + i * stepSize;
            
            // Find segment [p1, p2]
            auto it = std::ranges::lower_bound(sortedPoints, currentX, {}, [](const Point& p) { return p.x.value; });
            
            if (it == sortedPoints.begin()) {
                result.push_back(*it);
            } else if (it == sortedPoints.end()) {
                result.push_back(sortedPoints.back());
            } else {
                const Point& p2 = *it;
                const Point& p1 = *std::prev(it);
                
                double t = (currentX - p1.x.value) / (p2.x.value - p1.x.value);
                double y = p1.y.value + t * (p2.y.value - p1.y.value);
                
                result.push_back(Point{Coord{currentX}, Coord{y}});
            }
        }

        return result;
    }
};

static_assert(Domain::Ports::Interpolator<LinearInterpolator>);

} // namespace Infrastructure::Interpolation
