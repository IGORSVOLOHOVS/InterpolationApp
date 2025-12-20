#pragma once

#include <vector>
#include <algorithm>
#include <cmath>
#include <expected>
#include <numbers>
#include "Domain/Model/Point.h"
#include "Domain/Ports/Interpolator.h"

namespace Infrastructure::Interpolation {

using Domain::Model::Point;
using Domain::Model::Coord;
using Domain::Ports::DomainError;

/**
 * @brief Helper for sorting points by X coordinate.
 * @param points Input points.
 * @return std::vector<Point> Sorted points.
 */
inline std::vector<Point> getSortedPoints(const std::vector<Point>& points) {
    std::vector<Point> sorted = points;
    std::ranges::sort(sorted, [](const Point& a, const Point& b) { return a.x < b.x; });
    return sorted;
}

// 2. Nearest Neighbor
/**
 * @brief Implements Nearest Neighbor Interpolation.
 * @details Assigns the value of the nearest data point to the interpolated point.
 */
class NearestNeighborInterpolator {
public:
    /**
     * @brief Performs nearest neighbor interpolation.
     * @param points Input control points.
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
        if (points.empty()) return std::unexpected(DomainError{"Points cannot be empty."});
        auto sortedPoints = getSortedPoints(points);
        double minX = sortedPoints.front().x.value;
        double maxX = sortedPoints.back().x.value;
        double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;

        std::vector<Point> result;
        result.reserve(steps);

        for (int i = 0; i < steps; ++i) {
            double cx = minX + i * stepSize;
            auto it = std::ranges::min_element(sortedPoints, [cx](const Point& a, const Point& b) {
                return std::abs(a.x.value - cx) < std::abs(b.x.value - cx);
            });
            result.push_back(Point{Coord{cx}, it->y});
        }
        return result;
    }
};

// 3. Step (Zero-Order Hold) - Left neighbor
/**
 * @brief Implements Step (Zero-Order Hold) Interpolation.
 * @details Holds the value of the previous point until a new point is reached (Left Neighbor).
 */
class StepInterpolator {
public:
    /**
     * @brief Performs step interpolation.
     * @param points Input control points.
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
        if (points.empty()) return std::unexpected(DomainError{"Points cannot be empty."});
        auto sortedPoints = getSortedPoints(points);
        double minX = sortedPoints.front().x.value;
        double maxX = sortedPoints.back().x.value;
        double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;

        std::vector<Point> result;
        result.reserve(steps);

        for (int i = 0; i < steps; ++i) {
            double cx = minX + i * stepSize;
            auto it = std::ranges::upper_bound(sortedPoints, cx, {}, [](const Point& p) { return p.x.value; });
            const Point& p = (it == sortedPoints.begin()) ? sortedPoints.front() : *std::prev(it);
            result.push_back(Point{Coord{cx}, p.y});
        }
        return result;
    }
};


// 4. Lagrange Polynomial
/**
 * @brief Implements Lagrange Polynomial Interpolation.
 * @details Fits a polynomial of degree N-1 through N points.
 * @warning Computational complexity is high O(N^2) per point. Prone to Runge's phenomenon for large N.
 */
class LagrangeInterpolator {
public:
    /**
     * @brief Performs Lagrange interpolation.
     * @param points Input control points (at least 2).
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
        if (points.size() < 2) return std::unexpected(DomainError{"Need at least 2 points."});
        // Lagrange is O(N^2) per point, careful with large N
        auto sortedPoints = getSortedPoints(points);
        double minX = sortedPoints.front().x.value;
        double maxX = sortedPoints.back().x.value;
        double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;

        std::vector<Point> result;
        result.reserve(steps);

        for (int i = 0; i < steps; ++i) {
            double cx = minX + i * stepSize;
            double cy = 0;
            for (size_t j = 0; j < sortedPoints.size(); ++j) {
                double term = sortedPoints[j].y.value;
                for (size_t k = 0; k < sortedPoints.size(); ++k) {
                    if (j != k) {
                        term *= (cx - sortedPoints[k].x.value) / (sortedPoints[j].x.value - sortedPoints[k].x.value);
                    }
                }
                cy += term;
            }
            result.push_back(Point{Coord{cx}, Coord{cy}});
        }
        return result;
    }
};

// 5. Cubic Spline (Simplified Natural Spline)
/**
 * @brief Implements Cubic Spline Interpolation (Natural).
 * @details Ensures continuous first and second derivatives for a smooth curve.
 */
class CubicSplineInterpolator {
    // Boilerplate helper for spline coeff calculation omitted for brevity in single-file representation
    // Implementing a basic version
public:
    /**
     * @brief Performs cubic spline interpolation.
     * @param points Input control points (at least 3).
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
         if (points.size() < 3) return std::unexpected(DomainError{"Need at least 3 points for Cubic Spline."});
         
         auto P = getSortedPoints(points);
         size_t n = P.size() - 1;
         std::vector<double> a(n+1), b(n), d(n), h(n), alpha(n), c(n+1), l(n+1), mu(n+1), z(n+1);

         for(size_t i=0; i<=n; ++i) a[i] = P[i].y.value;
         for(size_t i=0; i<n; ++i) h[i] = P[i+1].x.value - P[i].x.value;
         
         for(size_t i=1; i<n; ++i) 
             alpha[i] = (3.0/h[i])*(a[i+1]-a[i]) - (3.0/h[i-1])*(a[i]-a[i-1]);

         l[0] = 1.0; mu[0] = 0.0; z[0] = 0.0;
         for(size_t i=1; i<n; ++i) {
             l[i] = 2.0*(P[i+1].x.value - P[i-1].x.value) - h[i-1]*mu[i-1];
             mu[i] = h[i]/l[i];
             z[i] = (alpha[i] - h[i-1]*z[i-1])/l[i];
         }
         l[n] = 1.0; z[n] = 0.0; c[n] = 0.0;

         for(int j=n-1; j>=0; --j) {
             c[j] = z[j] - mu[j]*c[j+1];
             b[j] = (a[j+1]-a[j])/h[j] - h[j]*(c[j+1] + 2.0*c[j])/3.0;
             d[j] = (c[j+1]-c[j])/(3.0*h[j]);
         }

         double minX = P.front().x.value;
         double maxX = P.back().x.value;
         double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;
         std::vector<Point> result;
         result.reserve(steps);

         for(int i=0; i<steps; ++i) {
             double x = minX + i * stepSize;
             // find segment
             size_t j = 0;
             for(size_t k=0; k<n; ++k) {
                 if (x >= P[k].x.value && x <= P[k+1].x.value) {
                     j = k;
                     break;
                 }
                 // Handle rounding/last point
                 if (k == n-1 && x >= P[k+1].x.value) j = k; 
             }
             double dx = x - P[j].x.value;
             double y = a[j] + b[j]*dx + c[j]*dx*dx + d[j]*dx*dx*dx;
             result.push_back(Point{Coord{x}, Coord{y}});
         }

         return result;
    }
};

// 6. Akima Spline (Simplified)
/**
 * @brief Implements Akima Spline Interpolation.
 * @details A special spline stable to outliers and with less oscillation than cubic splines.
 */
class AkimaSplineInterpolator {
public:
     /**
     * @brief Performs Akima spline interpolation.
     * @param points Input control points (at least 5).
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
     [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
         if (points.size() < 5) return std::unexpected(DomainError{"Need at least 5 points for Akima Spline."});
         // Placeholder implementation: falling back to linear for prototype or implementing full algo requires more lines.
         // Let's implement full Akima for quality.
         auto P = getSortedPoints(points);
         size_t n = P.size();
         std::vector<double> m(n-1);
         for(size_t i=0; i<n-1; ++i) 
            m[i] = (P[i+1].y.value - P[i].y.value) / (P[i+1].x.value - P[i].x.value);
         
         std::vector<double> s(n); // slopes
         // Special handling for boundaries usually involves extrapolation, here we simplify
         // Using 0 for boundaries implies flatness, or mirror. 
         // Standard Akima uses extra estimated points.
         
         // Robust approximation for internal points
         for(size_t i=2; i<n-2; ++i) {
             double m1 = m[i-2], m2 = m[i-1], m3 = m[i], m4 = m[i+1];
             double num = std::abs(m4 - m3) * m2 + std::abs(m2 - m1) * m3;
             double den = std::abs(m4 - m3) + std::abs(m2 - m1);
             s[i] = (den == 0) ? 0 : num / den;
         }
         // Fill boundaries (simplified)
         s[0] = m[0]; s[1] = (m[0]+m[1])/2; 
         s[n-2] = (m[n-3]+m[n-2])/2; s[n-1] = m[n-2];

         double minX = P.front().x.value;
         double maxX = P.back().x.value;
         double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;
         std::vector<Point> result;

         for(int i=0; i<steps; ++i) {
             double x = minX + i * stepSize;
             // Segment search
             size_t k = 0;
             while(k < n-2 && x > P[k+1].x.value) k++;
             
             double h = P[k+1].x.value - P[k].x.value;
             double dx = x - P[k].x.value;
             
             double c0 = P[k].y.value;
             double c1 = s[k];
             double c2 = (3*m[k] - 2*s[k] - s[k+1]) / h;
             double c3 = (s[k] + s[k+1] - 2*m[k]) / (h*h);
             
             double y = c0 + c1*dx + c2*dx*dx + c3*dx*dx*dx;
             result.push_back(Point{Coord{x}, Coord{y}});
         }
         return result;
     }
};

// 7. Cosine Interpolation (Between points)
/**
 * @brief Implements Cosine Interpolation.
 * @details Provides a smooth transition between points using a cosine segment.
 */
class CosineInterpolator {
public:
    /**
     * @brief Performs cosine interpolation.
     * @param points Input control points (at least 2).
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
        if (points.size() < 2) return std::unexpected(DomainError{"Need at least 2 points."});
        auto P = getSortedPoints(points);
        double minX = P.front().x.value;
        double maxX = P.back().x.value;
        double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;
        
        std::vector<Point> result;
        for(int i=0; i<steps; ++i) {
             double x = minX + i * stepSize;
             auto it = std::ranges::upper_bound(P, x, {}, [](const Point& p) { return p.x.value; });
             if (it == P.begin()) { result.push_back({Coord{x}, P.front().y}); continue; }
             if (it == P.end()) { result.push_back({Coord{x}, P.back().y}); continue; }
             
             const Point& p1 = *std::prev(it);
             const Point& p2 = *it;
             double mu = (x - p1.x.value) / (p2.x.value - p1.x.value);
             double mu2 = (1 - std::cos(mu * std::numbers::pi)) / 2;
             double y = (p1.y.value * (1 - mu2) + p2.y.value * mu2);
             result.push_back(Point{Coord{x}, Coord{y}});
        }
        return result;
    }
};

// 8. Sinc Interpolation (Whittaker-Shannon reconstruction - naive sum)
/**
 * @brief Implements Sinc (Whittaker-Shannon) Interpolation.
 * @details Ideal for band-limited signal reconstruction.
 */
class SincInterpolator {
public:
    /**
     * @brief Performs Sinc interpolation.
     * @param points Input control points (non-empty).
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
         if (points.empty()) return std::unexpected(DomainError{"Points cannot be empty."});
         // Requires uniform sampling ideally, but we try best effort.
         // Assuming normalized X for sinc window usually? Or direct sum.
         // Sum_k y_k * sinc((x - x_k)/T). Let's assume T is avg spacing.
         
         auto P = getSortedPoints(points);
         if (P.size() < 2) return P; // No spacing
         double avgSpacing = (P.back().x.value - P.front().x.value) / (P.size() - 1);
         if (avgSpacing == 0) avgSpacing = 1.0;

         double minX = P.front().x.value;
         double maxX = P.back().x.value;
         double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;
         
         std::vector<Point> result;
         for(int i=0; i<steps; ++i) {
             double x = minX + i * stepSize;
             double y = 0;
             for(const auto& p : P) {
                 double v = (x - p.x.value) / avgSpacing;
                 double weight = (v == 0) ? 1.0 : (std::sin(std::numbers::pi * v) / (std::numbers::pi * v));
                 y += p.y.value * weight;
             }
             result.push_back(Point{Coord{x}, Coord{y}});
         }
         return result;
    }
};

// 9. Moving Average (Smoothing, not strictly interpolation but can function as such/regressor)
/**
 * @brief Implements Moving Average Smoothing.
 * @details Acts as a low-pass filter to smooth out noise.
 */
class MovingAverageInterpolator {
    int windowSize_ = 3;
public:
    /**
     * @brief Constructs a MovingAverageInterpolator.
     * @param w Window size (default 3).
     */
    MovingAverageInterpolator(int w = 3) : windowSize_(w) {}
    
    /**
     * @brief Performs moving average smoothing (after linear resampling).
     * @param points Input control points (at least 2).
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
         if (points.size() < 2) return std::unexpected(DomainError{"Need at least 2 points."});
         
         // Validating window
         if (windowSize_ < 1) return std::unexpected(DomainError{"Window size must be >= 1"});

         // 1. Linearly resample first
         LinearInterpolator lin;
         auto res = lin.interpolate(points, steps);
         if (!res) return res;
         
         auto& data = res.value();
         std::vector<Point> smoothed = data;
         
         // 2. Apply Window
         int half = windowSize_ / 2;
         for(size_t i = 0; i < data.size(); ++i) {
             double sum = 0;
             int count = 0;
             for (int k = -half; k <= half; ++k) {
                 int idx = static_cast<int>(i) + k;
                 if (idx >= 0 && idx < static_cast<int>(data.size())) {
                     sum += data[idx].y.value;
                     count++;
                 }
             }
             if (count > 0) smoothed[i].y.value = sum / count;
         }
         return smoothed;
    }
};

// 10. Catmull-Rom Spline
/**
 * @brief Implements Catmull-Rom Spline Interpolation.
 * @details A local interpolating spline that passes through the control points.
 */
class CatmullRomInterpolator {
public:
    /**
     * @brief Performs Catmull-Rom spline interpolation.
     * @param points Input control points (at least 4).
     * @param steps Total number of points to generate.
     * @return Resulting points or error.
     */
    [[nodiscard]] std::expected<std::vector<Point>, DomainError> interpolate(const std::vector<Point>& points, int steps) const {
        if (points.size() < 4) return std::unexpected(DomainError{"Catmull-Rom needs at least 4 points (or phantom points)."});
        auto P = getSortedPoints(points);
        


        double minX = P.front().x.value;
        double maxX = P.back().x.value;
        double stepSize = (steps > 1) ? (maxX - minX) / (steps - 1) : 0;
        
        std::vector<Point> result;
        // Simple Uniform Catmull-Rom for strictly uniform X is easier, but here X might vary.
        // Assuming uniform Catmull-Rom for simplicity on X-Y curve segments
        // We need 4 points p0, p1, p2, p3 to interpolate between p1 and p2.
        
        for(int i=0; i<steps; ++i) {
             double x = minX + i * stepSize;
             // find segment p1->p2
             // Need index k such that P[k] <= x < P[k+1]
             auto it = std::ranges::upper_bound(P, x, {}, [](const Point& p) { return p.x.value; });
             size_t k = (it == P.begin()) ? 0 : std::distance(P.begin(), it) - 1;
             if (k >= P.size() - 1) k = P.size() - 2;

             // Indices
             size_t i0 = (k > 0) ? k - 1 : 0;
             size_t i1 = k;
             size_t i2 = (k + 1 < P.size()) ? k + 1 : k;
             size_t i3 = (k + 2 < P.size()) ? k + 2 : k; // Clamp end

             const Point& p0 = P[i0];
             const Point& p1 = P[i1];
             const Point& p2 = P[i2];
             const Point& p3 = P[i3];

             // Standard Catmull-Rom formula with t in [0,1]
             double t = (p2.x.value - p1.x.value == 0) ? 0 : (x - p1.x.value) / (p2.x.value - p1.x.value);
             
             double t2 = t * t;
             double t3 = t * t * t;

             double f0 = -0.5 * t3 + t2 - 0.5 * t;
             double f1 = 1.5 * t3 - 2.5 * t2 + 1.0;
             double f2 = -1.5 * t3 + 2.0 * t2 + 0.5 * t;
             double f3 = 0.5 * t3 - 0.5 * t2;

             double y = p0.y.value * f0 + p1.y.value * f1 + p2.y.value * f2 + p3.y.value * f3;
             result.push_back(Point{Coord{x}, Coord{y}});
        }
        return result;
    }
};

} // namespace Infrastructure::Interpolation
