// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 neov5

#ifndef __ROM_H__ 
#define __ROM_H__ 

#include "types.h"

struct rom_t;
struct rom_mapper_t;

typedef struct rom_t rom_t;
typedef struct rom_mapper_t rom_mapper_t;

typedef enum {
    NONE = 0,
    MMC1 = 1,
    UXROM = 2,
    CNROM = 3,
    MMC3 = 4
} rom_mapper_type_t;

typedef enum {
    VERTICAL = 0,
    HORIZONTAL = 1 
} rom_nt_mirror_t;

typedef enum {
    CPU = 0,
    PPU = 1 
} rom_access_t;

struct rom_mapper_t {
    rom_mapper_type_t type;
    u8 (*cpu_read)(rom_t*, u16);
    u8 (*ppu_read)(rom_t*, u16);
    void (*cpu_write)(rom_t*, u8, u16);
    void (*ppu_write)(rom_t*, u8, u16);
};

struct rom_t {
    u32 prg_rom_size;
    u32 prg_ram_size;
    u32 chr_rom_size;
    rom_nt_mirror_t mirror_type;
    rom_mapper_t mapper;

    u8 (*rom_mapper)(rom_t*, rom_access_t, u16);
    u8 *prg_rom;
    u8 *chr_rom;
    u8 *prg_ram;
};

void rom_load_from_file(rom_t *rom, char* filename);
void rom_free(rom_t *rom);

u8 no_mapper_cpu_read(rom_t *rom, u16 addr);
u8 no_mapper_ppu_read(rom_t *rom, u16 addr);
void no_mapper_cpu_write(rom_t *rom, u8 val, u16 addr);
void no_mapper_ppu_write(rom_t *rom, u8 val, u16 addr);

// u8 mmc1_mapper(rom_t *rom, rom_access_t acc, u16 addr);
// u8 uxrom_mapper(rom_t *rom, rom_access_t acc, u16 addr);
// u8 cnrom_mapper(rom_t *rom, rom_access_t acc, u16 addr);
// u8 mmc3_mapper(rom_t *rom, rom_access_t acc, u16 addr);

#endif 
