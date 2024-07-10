#ifndef __NES_H__
#define __NES_H__

#include "cpu.h"
#include "ppu.h"
#include "mem.h"
#include "rom.h"

typedef struct {

    cpu_state_t cpu_st;
    ppu_state_t ppu_st;

    rom_t *rom;
    mem_cpu_t *cpu_mem;
    mem_ppu_t *ppu_mem;

} nes_state_t;

u8 nes_cpu_bus_read(u16 addr);
u8 nes_ppu_bus_read(u16 addr);
void nes_cpu_bus_write(u8 data, u16 addr);
void nes_ppu_bus_write(u8 data, u16 addr);

#endif
