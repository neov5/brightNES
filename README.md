# brightNES

Cycle-accurate NES emulator

![demo](ghimg/demo.png)

## Features

- Instruction Stepped, Cycle Ticked CPU
- Cycle Ticked PPU
- Load external palettes with the `-p` option
- Smooth horizontal scrolling 
- Sprite 0 flag set

## Quick Start

brightNES requires CMake and a C compiler to build

### Compile with system SDL2

Use this option if you have SDL2 already installed

```
git clone https://github.com/neov5/brightNES.git && cd brightNES
mkdir build build/release
cmake -B build/release -DSYSTEM_SDL=1 -DCMAKE_BUILD_TYPE=release
cmake --build build/release
./build/release/brightnes <rom_path> [-p|--palette palette_path]
```

### Compile with packaged SDL2

This install builds SDL2 from source along with the emulator

```
git clone --recurse-submodules https://github.com/neov5/brightNES.git && cd brightNES
mkdir build build/release
cmake -B build/release -DCMAKE_BUILD_TYPE=release
cmake --build build/release
./build/release/brightnes <rom_path> [-p|--palette palette_path]
```

### Debug builds

```
mkdir build build/debug
cmake -B build/debug [-DSYSTEM_SDL=0/1] -DCMAKE_BUILD_TYPE=debug && cmake --build build/debug
./build/debug/brightnes <rom_path> [-p|--palette palette_path]
```

Note that the debugger stalls at start, and the window can be a bit 
unresponsive as a result of not processing SDL events.

## Debug Mode

BrightNES supp

## TODO

- [ ] Debug remaining mapper 0 games
  - [ ] Ice Climber 
  - [ ] Ice Hockey
  - [ ] SMB rendering when mario is at the top of the screen
- [ ] APU
- [ ] Mappers
  - [ ] MMC1
  - [ ] MMC3
  - [ ] MMC5
- [ ] Full-featured debug mode?
