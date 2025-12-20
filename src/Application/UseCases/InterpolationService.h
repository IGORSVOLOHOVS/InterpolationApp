#pragma once

#include <vector>
#include <expected>
#include "Domain/Model/Point.h"
#include "Domain/Ports/Interpolator.h"

namespace Application::UseCases {

using Domain::Model::Point;
using Domain::Ports::DomainError;

/**
 * @brief Application Service for orchestrating interpolation.
 * @tparam T The interpolation strategy conforming to the Interpolator concept.
 */
template <Domain::Ports::Interpolator T>
class InterpolationService {
public:
    /**
     * @brief Constructs the service with a specific strategy.
     * @param strategy The interpolation strategy instance.
     */
    explicit InterpolationService(T strategy) : strategy_(std::move(strategy)) {}

    /**
     * @brief Executes the interpolation strategy.
     * @param points Input control points.
     * @param steps Desired number of interpolated points.
     * @return Resulting points or domain error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> execute(const std::vector<Point>& points, int steps) const {
        return strategy_.interpolate(points, steps);
    }

private:
    T strategy_;
};

} // namespace Application::UseCases
