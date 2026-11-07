#ifndef INTERPOLATION_APP_DOMAIN_INTERPOLATOR_H
#define INTERPOLATION_APP_DOMAIN_INTERPOLATOR_H

#include <concepts>
#include <expected>
#include <string>
#include <vector>

#include "Domain/Model/Point.h"

namespace Domain::Ports {

using Domain::Model::Point;

/**
 * @brief Domain-specific error type.
 * @details Used to propagate errors from lower layers without exception overhead.
 */
struct DomainError {
    /// Error description message.
    std::string message;
};

/**
 * @brief Concept defining the contract for any Interpolation strategy.
 * @tparam T The type implementing the strategy.
 *
 * @details A compliant strategy must provide an `interpolate` method that accepts
 * a vector of Points and a step count, returning a `std::expected` containing either
 * the interpolated points or a `DomainError`.
 */
template <typename T>
concept Interpolator = requires(T t, const std::vector<Point>& points, int steps) {
    { t.interpolate(points, steps) } -> std::same_as<std::expected<std::vector<Point>, DomainError>>;
};

}  // namespace Domain::Ports

#endif  // INTERPOLATION_APP_DOMAIN_INTERPOLATOR_H
