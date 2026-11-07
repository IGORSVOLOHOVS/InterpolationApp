# Manual

## The core

Every interpolator takes the control points and a step count, and answers `std::expected<std::vector<Point>, DomainError>`:

```cpp
#include "Infrastructure/Interpolation/LinearInterpolator.h"

Infrastructure::Interpolation::LinearInterpolator linear;
auto result = linear.interpolate(points, 100);
if (!result) { /* result.error().message says why */ }
```

`Application::UseCases::InterpolationService` wraps any type that meets the `Interpolator` concept. `PointGenerationService` makes sample data: `generateRandomPoints(count, {.minX = 0, .maxX = 100, .minY = 0, .maxY = 100})` and `generateSineWave(count, minX, maxX)`.

## The tests and benchmarks

`.\recipe.ps1 run` builds and runs the test binary, and hands it any further arguments:

| command | what it does |
| --- | --- |
| `.\recipe.ps1 run` | every test case but the benchmarks |
| `.\recipe.ps1 run --list-test-cases` | names the test cases |
| `.\recipe.ps1 run --no-skip --test-case=Benchmarks` | times all ten methods and writes `benchmark_results_optimized.html` |

`ctest` never runs the benchmarks: they make no assertion and take far longer than the tests.

## The window

With Qt built as `docs/install.md` says, the window lets you pick a method and a point count, draws the control points and the curve, and shows how long the method took. No gate builds or checks it yet.
