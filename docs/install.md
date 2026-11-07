# Install

The computational core needs no installing: `src/Domain`, `src/Application` and `src/Infrastructure` are headers. Put `src` on your include path and build with C++23.

To build and test it here:

1. Install mingw g++ 13 or newer, CMake 3.28 or newer, Ninja, and LLVM for `clang-tidy` and `clang-format`.
2. Clone this repository.
3. Run `.\recipe.ps1 test`. The first run downloads doctest and nanobench, whose archives `CMakeLists.txt` pins by hash.

`recipe.ps1` looks for g++ in `C:\ProgramData\mingw64\mingw64\bin` and for LLVM in `C:\Program Files\LLVM\bin`, and adds those folders to its own PATH.

`.\recipe.ps1 build` compiles the core and its tests into `build\`. `.\recipe.ps1 install` does nothing: there is no installer yet.

## The window

The window needs Qt6 with Core, Gui, Widgets and Charts, and `recipe.ps1` does not build it. With Qt on the machine:

```powershell
cmake -S . -B build-window -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build-window
```

`INTERPOLATION_APP_WINDOW` is on by default, so a plain configure wants Qt.
