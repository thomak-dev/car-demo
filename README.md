# Car Demo

![Hero shot](docs/screenshot.jpg)

Discontinued 3D Engine project. Implements an interactive and physically simulated car-driving demo based on PhysX and with dynamic engine audio based on FMOD. Uses OpenGL 3.2 for rendering, SDL for window/input and AssImp for asset loading. Also has a simple scene-graph with JSON-based authoring.

## Requirements (Windows x64)

- MSVC Build Tools 14.51+
- CMake 3.21+
- FMOD Engine 2.03.x
- FMOD Studio 2.03.x — for `fmodstudiocl`, build produces banks from the contained FMOD project

Both FMOD components are expected at their respective default installation locations.

## Build

I am recommending a shallow clone because the repo including submodules is quite big:

```
git clone --depth 1 --recurse-submodules --shallow-submodules https://github.com/thomak-dev/car-demo.git
cd car-demo
cmake -S . -B build -A x64
cmake --build build --config Debug
```

Substitute `Debug` for `Release` for a release build.
