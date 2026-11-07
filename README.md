# interpolation-app

Ten interpolation methods in modern C++23, and a Qt6 desktop window that plots and times them side by side. The algorithms are header-only and depend on nothing but the standard library; only the window needs Qt.

## Methods

1. **Linear**: a straight line between neighbours.
2. **Nearest Neighbor**: the closest point's value.
3. **Step (zero-order)**: holds a value until the next point.
4. **Lagrange polynomial**: one global polynomial, O(N^2).
5. **Cubic spline**: smooth piecewise cubics.
6. **Akima spline**: a spline that resists outliers.
7. **Cosine**: a smooth trigonometric transition.
8. **Sinc (Lanczos)**: signal reconstruction.
9. **Moving average**: a smoothing filter over a linear resample.
10. **Catmull-Rom spline**: a local spline through the control points.

Errors travel as `std::expected`: the core holds no exception and no lock, and `.\recipe.ps1 test` fails if one appears.

## Layout

```text
src/
  Domain/          value objects (Point, Coord) and the Interpolator concept
  Infrastructure/  the algorithms
  Application/     the services that run them
  Presentation/    the Qt window
  Tests/           doctest cases and nanobench benchmarks
```

## Build and test

`.\recipe.ps1 test` builds the core, runs `ctest`, `clang-tidy` and `clang-format`, and searches `src/Domain`, `src/Application` and `src/Infrastructure` for locks and for `throw`, `try` and `catch`. It needs mingw g++, cmake, ninja and LLVM, and no Qt: it configures with `-DINTERPOLATION_APP_WINDOW=OFF`. `docs/install.md` has the details, and `docs/manual.md` covers the benchmarks and the window.

Doxygen builds the API pages from `Doxyfile`. MIT licensed.

## Lab requirements

This repository complies with [revision 4.0.0](http://127.0.0.1:3000/igors-lab/software-requirements/src/tag/v4.0.0/BEST_REQUIREMENTS.md) of the lab requirements, except R-02 (#8), R-09 (#9), R-12 (#10), R-13 (#11), R-14 (#12), R-17 (#13), R-41 (#14), R-42 (#15).
