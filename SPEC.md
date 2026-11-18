# Software Requirements Specification — InterpolationApp

Reverse-engineered on 2026-11-18 from commit `5670e81`. Every requirement describes
behaviour present in the code at that commit; sources are `path:line`. Claims found only
in prose are listed in §3.8. Documentation hub: [`docs/README.md`](docs/README.md).

## 1. Purpose and scope

InterpolationApp generates random 2D points, resamples them with one of ten
interpolation or smoothing methods, and plots both in a Qt6 window together with the
duration of the call (`src/Presentation/MainWindow.cpp:141-199`).

In scope:

- Domain types and the strategy contract (`src/Domain/`).
- Ten header-only strategies (`src/Infrastructure/Interpolation/`).
- Point generation and the strategy runner (`src/Application/UseCases/`).
- The Qt6 window (`src/Presentation/`).
- The doctest/nanobench executable `tests_run` (`src/Tests/`, `CMakeLists.txt:63-81`).
- Doxygen configuration (`Doxyfile`).

Out of scope (no code for it): loading or saving point sets, user-entered points,
exporting results, any network or hardware access, packaging or installation.

## 2. Context

| Item | Description | Source |
| --- | --- | --- |
| Actor: user | Operates the dock controls of the window | `MainWindow.cpp:84-139` |
| Actor: developer | Builds with CMake, runs `tests_run`, runs Doxygen | `README.md:57-90` |
| Qt6 Core/Gui/Widgets/Charts | Window, controls, chart | `CMakeLists.txt:13` |
| doctest, nanobench | Test framework and benchmark library, fetched at configure time | `CMakeLists.txt:23-37` |
| Google Chrome | Started by the benchmark case to show the HTML report | `src/Tests/InterpolationTests.cpp:192-196` |
| Doxygen, Graphviz `dot` | API documentation with graphs | `Doxyfile:1-25` |
| Runtime | Desktop OS with Qt6; Linux and Windows branches exist in test code and MSVC/non-MSVC branches in CMake | `InterpolationTests.cpp:192-196`, `CMakeLists.txt:57-61` |

## 3. Functional requirements

### 3.1 Domain and contract

- **FR-1** The system shall represent a point as two `Coord` values `x` and `y`, each
  wrapping one `double`, with defaulted three-way comparison.
  Source: `src/Domain/Model/Point.h:14-34`. Verification: no test (used by all tests).
- **FR-2** The system shall define an `Interpolator` concept that requires
  `interpolate(const std::vector<Point>&, int steps)` returning
  `std::expected<std::vector<Point>, DomainError>`, where `DomainError` carries a message.
  Source: `src/Domain/Ports/Interpolator.h:17-33`. Verification: `static_assert` at
  `src/Infrastructure/Interpolation/LinearInterpolator.h:78` (Linear only); the other
  strategies are checked when `InterpolationService<T>` is instantiated
  (`MainWindow.cpp:169-178`).
- **FR-3** `InterpolationService<T>` shall forward `execute(points, steps)` to the
  wrapped strategy unchanged. Source: `src/Application/UseCases/InterpolationService.h:17-34`.
  Verification: no test.

### 3.2 Shared resampling rule

- **FR-4** Each strategy shall work on a copy of the input sorted by x and, for
  `steps > 1`, return `steps` points whose x values are evenly spaced from the minimum to
  the maximum input x inclusive. For `steps == 1` the strategies in
  `AdditionalInterpolators.h` use step size 0 (one point at the minimum x); Linear divides
  by `steps - 1` and returned the first input point in a scratch run.
  Source: `AdditionalInterpolators.h:22-26,46,80,116,183,249,291,337,427`;
  `LinearInterpolator.h:44-53`. Verification: `Linear Interpolation` (size 20).

### 3.3 Strategies

All in `src/Infrastructure/Interpolation/`; "AI.h" below is `AdditionalInterpolators.h`.

- **FR-5 Linear** The system shall return y by linear interpolation between the two
  neighbouring input points, the first input point at or before the first x, the last
  beyond the end, and an error for fewer than 2 points or `steps <= 0`.
  Source: `LinearInterpolator.h:28-75`. Verification: `Linear Interpolation`.
- **FR-6 Nearest Neighbor** The system shall return for each x the y of the input point
  with the smallest |x − xₖ|, and an error for an empty input. Source: `AI.h:41-59`.
  Verification: `Nearest Neighbor Interpolation`.
- **FR-7 Step** The system shall return for each x the y of the last input point with
  xₖ ≤ x (the first point if none), and an error for an empty input.
  Source: `AI.h:75-92`. Verification: `Step Interpolation` (value checks do not execute,
  see `docs/README.md` §7).
- **FR-8 Lagrange** The system shall evaluate the Lagrange polynomial through all input
  points, and return an error for fewer than 2 points. Source: `AI.h:110-136`.
  Verification: `Lagrange Interpolation`.
- **FR-9 Cubic Spline** The system shall evaluate a natural cubic spline (second
  derivative 0 at both ends) from the tridiagonal system, and return an error for fewer
  than 3 points. Source: `AI.h:154-205`. Verification: `Cubic Spline Interpolation`
  (`has_value` only).
- **FR-10 Akima** The system shall compute segment slopes mₖ, node slopes by the Akima
  weighting for interior nodes 2…n−3 (0 when both weights are 0), boundary slopes
  s₀ = m₀, s₁ = (m₀+m₁)/2, sₙ₋₂ = (mₙ₋₃+mₙ₋₂)/2, sₙ₋₁ = mₙ₋₂, evaluate a cubic per
  segment, and return an error for fewer than 5 points. Source: `AI.h:221-270`.
  Verification: `Akima Spline Interpolation` (`has_value` only).
- **FR-11 Cosine** The system shall blend neighbouring y values with weight
  (1 − cos(πμ))/2, hold the end y values outside the range, and return an error for fewer
  than 2 points. Source: `AI.h:286-308`. Verification: `Cosine Interpolation`
  (`has_value` only).
- **FR-12 Sinc** The system shall return Σ yₖ·sinc((x − xₖ)/T) with normalised sinc and
  T the average input spacing (1 when that is 0), return a single input point unchanged
  regardless of `steps`, and return an error for an empty input. Source: `AI.h:324-351`.
  Verification: `Sinc Interpolation` (`has_value` only).
- **FR-13 Moving Average** The system shall resample linearly to `steps` points, replace
  each y by the mean over a centred window of `w/2` points on each side (truncated at the
  ends; `w` defaults to 3), propagate errors from the linear step, and return an error
  for `w < 1`. Source: `AI.h:359-403`. Verification: `Moving Average Smoother`.
- **FR-14 Catmull-Rom** The system shall evaluate the uniform Catmull-Rom basis on the
  segment containing x with neighbour indices clamped at the ends (t = 0 for a
  zero-width segment), and return an error for fewer than 4 points.
  Source: `AI.h:419-468`. Verification: `Catmull-Rom Spline` (`has_value` only).

### 3.4 Point generation

- **FR-15** `generateRandomPoints(count, minX, maxX, minY, maxY)` shall return `count`
  points drawn uniformly from the given ranges with a `std::random_device` seed, sorted by
  x, and an empty vector for `count <= 0`. Source: `PointGenerationService.h:30-49`.
  Verification: no test.
- **FR-16** `generateSineWave(count, minX, maxX)` shall return `count` points with x
  evenly spaced from `minX` to `maxX` and y = sin(x), and an empty vector for
  `count <= 0`. Source: `PointGenerationService.h:59-68`. Verification: no test (not
  called anywhere in the repository).

### 3.5 Desktop window

- **FR-17** On start the system shall create the main window and show it maximised.
  Source: `src/Presentation/Main.cpp:11-17`. Verification: no test.
- **FR-18** The window shall use the Fusion style with a dark palette, the title
  "Interpolation App", and an initial size of 1200×800. Source: `MainWindow.cpp:20-40`.
  Verification: no test.
- **FR-19** The chart shall show input points as a red scatter series "Original Points"
  (marker size 10) and the result as a blue line series "Interpolated", with the dark
  chart theme and antialiasing. Source: `MainWindow.cpp:60-82`. Verification: no test.
- **FR-20** A dock "Controls" (left or right area, right by default) shall offer the
  controls listed in §4.2. Source: `MainWindow.cpp:84-139`. Verification: no test.
- **FR-21** "Generate" shall replace the point set with random points in [0, 100]×[0, 100]
  of the chosen count and update the result; no points are generated at start.
  Source: `MainWindow.cpp:50-51,141-144`. Verification: no test.
- **FR-22** A change of method or steps, and each generation, shall re-plot the points,
  run the selected strategy through `InterpolationService`, show the elapsed time as
  "Time: N µs", plot the result on success (nothing on error), and set the axis ranges to
  the data extent ±5 (x from the input points, y from input and result).
  Source: `MainWindow.cpp:119,126,146-199`. Verification: no test.
- **FR-23** The window shall restore geometry and state from `QSettings` on construction
  and save them on destruction. Source: `MainWindow.cpp:46-48,54-58`. Verification: no test.

### 3.6 Tests, benchmarks, documentation

- **FR-24** With `BUILD_TESTING` ON, the build shall produce `tests_run` from a generated
  runner that defines the doctest `main` and the nanobench implementation.
  Source: `CMakeLists.txt:16,63-81`. Verification: building it.
- **FR-25** The `Benchmarks` case shall time all ten strategies (100 points, 100000
  steps; Lagrange 10 points, 1000 steps; at least 20 iterations per epoch), write
  `benchmark_results_optimized.html` to the working directory, and start `google-chrome`
  on it on Linux or Windows. Source: `InterpolationTests.cpp:137-196`.
  Verification: running `tests_run`.
- **FR-26** `doxygen Doxyfile` shall generate HTML (no LaTeX) into `docs/html` from the
  headers under `src`, with class, call, include and directory graphs.
  Source: `Doxyfile:1-25`, `README.md:89`. Verification: no test.

### 3.7 Shared tooling

- **FR-27** The repository shall scan its full git history for credentials with gitleaks
  on every push, pull request and manual dispatch, as part of the
  [repo-quality-template](https://github.com/IGORSVOLOHOVS/repo-quality-template)
  tooling it carries. Source: `.github/workflows/secret-scan.yml:6-24`.
  Verification: the workflow run on GitHub.

### 3.8 Declared but not implemented

| Claim | Where declared | Finding |
| --- | --- | --- |
| "Sinc (Lanczos)" | `README.md:28`, `MainWindow.cpp:115` | No Lanczos window; plain sinc sum (`AI.h:343-347`) |
| "`constexpr` validation" | `README.md:14` | No `constexpr` in `src/` |
| "Status: Verified" badge | `README.md:5` | No CI job builds or tests; `ctest` has no tests (`CMakeLists.txt:19`, no `add_test`) |
| Catmull-Rom "phantom points" | `AI.h:420`, `InterpolationTests.cpp:90` | Not implemented; 4 points required |
| Full Akima boundary handling ("Standard Akima uses extra estimated points") | `AI.h:232-234` | Boundaries simplified (`AI.h:243-245`) |
| "Zero-Cost Abstractions … no runtime overhead" | `README.md:13` | Not measured anywhere in the repository |
| Further sources | `CMakeLists.txt:44` ("Add other sources here") | None added |

## 4. Interfaces

### 4.1 C++ API

All strategies are default-constructible classes with
`interpolate(const std::vector<Point>&, int steps) const` (FR-2).

| Class | Header:line | Minimum points | Error messages |
| --- | --- | --- | --- |
| `LinearInterpolator` | `LinearInterpolator.h:19` | 2 | "At least two points are required for linear interpolation."; "Steps must be positive." |
| `NearestNeighborInterpolator` | `AI.h:33` | 1 | "Points cannot be empty." |
| `StepInterpolator` | `AI.h:67` | 1 | "Points cannot be empty." |
| `LagrangeInterpolator` | `AI.h:102` | 2 | "Need at least 2 points." |
| `CubicSplineInterpolator` | `AI.h:144` | 3 | "Need at least 3 points for Cubic Spline." |
| `AkimaSplineInterpolator` | `AI.h:213` | 5 | "Need at least 5 points for Akima Spline." |
| `CosineInterpolator` | `AI.h:278` | 2 | "Need at least 2 points." |
| `SincInterpolator` | `AI.h:316` | 1 | "Points cannot be empty." |
| `MovingAverageInterpolator(int w = 3)` | `AI.h:359` | 2 | "Need at least 2 points."; "Window size must be >= 1"; Linear's errors |
| `CatmullRomInterpolator` | `AI.h:411` | 4 | "Catmull-Rom needs at least 4 points (or phantom points)." |

Other API: `InterpolationService<T>::execute` (FR-3),
`PointGenerationService::generateRandomPoints` / `generateSineWave` (FR-15, FR-16),
helper `getSortedPoints` (`AI.h:22-26`).

### 4.2 User interface (`MainWindow.cpp:84-139`)

| Control | Type | Range / items | Default | Effect |
| --- | --- | --- | --- | --- |
| Random Points | `QSpinBox` | 2 – 100000 | 10 | Count for "Generate" |
| Generate | `QPushButton` | — | — | FR-21 |
| Method | `QComboBox` | Linear, Nearest Neighbor, Lagrange Polynomial, Cubic Spline, Akima Spline, Step (Zero-Order), Cosine, Sinc (Lanczos), Moving Average, Catmull-Rom Spline | index 0 | FR-22 |
| Steps | `QSpinBox` | 10 – 5000 | 100 | FR-22 |
| Time label | `QLabel` | "Time: N µs" | "Time: N/A" | Shows call duration |

### 4.3 Build interface

| Item | Kind | Source |
| --- | --- | --- |
| `InterpolationApp` | executable target | `CMakeLists.txt:41-61` |
| `tests_run` | executable target (only with `BUILD_TESTING`) | `CMakeLists.txt:73-80` |
| `BUILD_TESTING` | CMake option, default ON | `CMakeLists.txt:16` |

## 5. Data

| Entity | Form | Storage | Source |
| --- | --- | --- | --- |
| `Coord`, `Point` | Value types of `double` | In memory, `std::vector<Point>` | `Point.h:14-34` |
| `DomainError` | Struct with `std::string message` | In memory | `Interpolator.h:17-20` |
| Current point set | `std::vector<Point> currentPoints` | Window member, not persisted | `MainWindow.h:45` |
| Window geometry/state | `QByteArray` under keys `geometry`, `windowState` | `QSettings`, organisation "MyCompany", application "InterpolationApp" | `MainWindow.cpp:46-58` |
| Benchmark report | HTML (nanobench `htmlBoxplot` template) | `benchmark_results_optimized.html` in the working directory, git-ignored | `InterpolationTests.cpp:186-189`, `.gitignore` |
| API documentation | HTML | `docs/html/`, git-ignored | `Doxyfile:2,7`, `.gitignore` |

## 6. Non-functional requirements

- **NFR-1** The code shall compile as C++23 without compiler extensions.
  Source: `CMakeLists.txt:4-6`, `:80`.
- **NFR-2** The `InterpolationApp` target shall build with warnings as errors
  (`-Wall -Wextra -Wpedantic -Werror`, or `/W4 /WX` on MSVC); `tests_run` has no such
  flags. Source: `CMakeLists.txt:56-61,73-80`.
- **NFR-3** Strategies shall report failures through `std::expected`/`DomainError`;
  `src/` contains no `throw`. Source: `Interpolator.h:13-20`; repository search.
  Deviation: with negative `steps`, Nearest Neighbor, Step, Lagrange and Cubic Spline
  pass it to `reserve` (`AI.h:49,83,119,185`) and throw `std::length_error` (scratch
  run); the UI cannot send negative steps (`MainWindow.cpp:124`).
- **NFR-4** Layer dependencies shall point inward: Domain includes only the standard
  library, Application and Infrastructure include Domain, only Presentation includes Qt.
  Source: include lines of each file under `src/`.
- **NFR-5** UI input shall be bounded to 2–100000 points and 10–5000 steps.
  Source: `MainWindow.cpp:94,124`.
- **NFR-6** Source formatting style is Google-based, 120 columns, indent 4; no hook or CI
  job enforces it. Source: `.clang-format`, `.pre-commit-config.yaml`.
- **NFR-7** No performance target is defined; the benchmark measures without thresholds.
  Lagrange is documented as O(N²) per output point. Source:
  `InterpolationTests.cpp:137-196`, `AI.h:100`.
- **NFR-8** The code is distributed under the MIT licence. Source: `LICENSE`.

## 7. Constraints and dependencies

| Dependency | Version | Source |
| --- | --- | --- |
| CMake | ≥ 3.28 | `CMakeLists.txt:1` |
| Qt6 Core, Gui, Widgets, Charts | not pinned ("6.x" in README) | `CMakeLists.txt:13`, `README.md:7` |
| doctest | v2.4.11 (git tag) | `CMakeLists.txt:25-30` |
| nanobench | v4.3.11 (git tag) | `CMakeLists.txt:32-37` |
| Compiler | C++23; README: Clang 16+ or GCC 11+ | `CMakeLists.txt:4`, `README.md:53` |
| Doxygen, Graphviz | not pinned, optional | `Doxyfile:14`, `README.md:54` |

There is no lock file and no package manager manifest (Conan, vcpkg) for the C++ code.

## 8. Acceptance

1. Configure and build per `README.md:63-67`; both targets build, `InterpolationApp`
   without warnings (NFR-1, NFR-2, FR-24).
2. Run `./tests_run`; all 11 cases pass and `benchmark_results_optimized.html` appears
   in the working directory (FR-4 – FR-14, FR-25).
3. Start `./InterpolationApp`: the window opens maximised and dark, with an empty chart
   and the controls of §4.2 (FR-17 – FR-20).
4. Press "Generate" with 10 points; switch through all ten methods; a curve and a time
   appear for each. With 4 points Akima draws nothing; with 3, Catmull-Rom draws nothing
   (FR-21, FR-22, §4.1).
5. Move the dock, close and restart; the layout is restored (FR-23).
6. Run `doxygen Doxyfile`; `docs/html/index.html` is created (FR-26).

None of these steps was executed for this document (see `docs/README.md` §11).

## 9. Open questions

- Who the program is for and why it exists: no statement in the repository. A CMake
  comment says "simplified for lab mode" (`CMakeLists.txt:21`); that it is a study
  project is an inference from that comment only.
- Supported platforms: `README.md:50` says Linux (Fedora/Ubuntu) tested; the code also has
  Windows and MSVC branches (`InterpolationTests.cpp:194-195`, `CMakeLists.txt:57-58`).
  Which platforms actually build is not recorded.
- Minimum compiler and Qt versions: README gives Clang 16+/GCC 11+ and Qt "6.x"; the code
  uses `std::expected` (`Interpolator.h:4`). Whether those minimums were checked is not
  recorded. Partial answer: `<expected>` first shipped in libstdc++ with GCC 12, so
  "GCC 11+" cannot compile `Interpolator.h` (library fact, not tested here).
- Performance targets: README says up to 100000 points are supported (`README.md:35`);
  no target time exists, and Lagrange at that size was not measured.
- Should "Sinc (Lanczos)" apply a Lanczos window, and should Catmull-Rom accept fewer
  points via phantom points (§3.8)?
- Should the UI show `DomainError` messages instead of an empty curve?
- Is the `QSettings` organisation name "MyCompany" (`MainWindow.cpp:46`) intended?
- Are the Python-oriented `scripts/*.py` and `CONTRIBUTING.md` meant to be adapted to
  this CMake project, and should CI build and run `tests_run`? Partial answer: the
  message of commit `a7a4c20` says a build/test workflow was left out on purpose because
  it "cannot pass without toolchains a hosted runner does not have".
- Should running `tests_run` start a browser (`InterpolationTests.cpp:192-196`)? On
  Linux the command ends in `&` (`:193`), so `tests_run` does not wait for it; the
  Windows branch and the behaviour on a headless machine were not run.
