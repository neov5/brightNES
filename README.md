# brightNES

Cycle-accurate NES emulator

![demo](ghimg/demo.png)

## Quick Start

brightNES requires a C compiler (gcc/clang) and CMake to compile.

### Compile with system SDL

```
git clone https://github.com/neov5/brightNES.git && cd brightNES
mkdir build build/release
cmake -B build/release -DSYSTEM_SDL=1 -DCMAKE_BUILD_TYPE=release
cmake --build build/release
./build/release/brightnes <rom_path> [-p|--palette palette_path]
```

### Compile with packaged SDL

```
git clone --recurse-submodules https://github.com/neov5/brightNES.git && cd brightNES
mkdir build build/release
cmake -B build/release -DCMAKE_BUILD_TYPE=release
cmake --build build/release
./build/release/brightnes <rom_path> [-p|--palette palette_path]
```

Compiling with the packaged SDL takes more time, as SDL2 is also compiled along 
with the emulator.

### Debug builds

```
mkdir build build/debug
cmake -B build/debug -DCMAKE_BUILD_TYPE=debug && cmake --build build/debug
./build/debug/brightnes <rom_path> [-p|--palette palette_path]
```

Note that the debugger stalls at start, and the window can be a bit 
unresponsive as a result of not processing SDL events.

