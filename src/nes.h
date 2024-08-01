// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 neov5

#ifndef __NES_H__
#define __NES_H__

#include "cpu.h"
#include "ppu.h"
#include "mem.h"
#include "rom.h"
#include "dma.h"
#include "joypad.h"

typedef struct {

    cpu_state_t cpu_st;
    ppu_state_t ppu_st;
    mem_cpu_t cpu_mem;
    mem_ppu_t ppu_mem;

    rom_t rom;

    joypad_t joypad;

    bool frame_done;
    dma_oam_t dma_oam;

    u64 ppu_cycle;
    u64 cpu_cycle;
} nes_state_t;

typedef struct {


} nes_input_t;

u8 nes_cpu_bus_read(u16 addr);
u8 nes_ppu_bus_read(u16 addr);
void nes_cpu_bus_write(u8 data, u16 addr);
void nes_ppu_bus_write(u8 data, u16 addr);

void nes_load_palette(char* palette_path);
void nes_init(char* rom_path);
bool nes_update_events();
void nes_exit();
void nes_render_frame();

#endif
