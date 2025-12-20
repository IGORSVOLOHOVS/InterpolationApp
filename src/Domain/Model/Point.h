#pragma once

#include <compare>
#include <concepts>

namespace Domain::Model {

/**
 * @brief Strong type wrapper for coordinate values.
 */
/**
 * @brief Strong type wrapper for coordinate values to prevent implicit conversions.
 */
struct Coord {
    /// The coordinate value.
    double value;
    
    /// Default spaceship operator for easy comparison.
    auto operator<=>(const Coord&) const = default;
};

/**
 * @brief Represents a 2D point in the Cartesian plane.
 * @details Uses Value Object semantics with zero-cost abstractions.
 */
struct Point {
    /// X coordinate.
    Coord x;
    /// Y coordinate.
    Coord y;
    
    /// Default spaceship operator for lexicographical comparison.
    auto operator<=>(const Point&) const = default;
};

} // namespace Domain::Model
