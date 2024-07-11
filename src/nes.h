#ifndef __NES_H__
#define __NES_H__

#include "cpu.h"
#include "ppu.h"
#include "mem.h"
#include "rom.h"

typedef struct {

    cpu_state_t cpu_st;
    ppu_state_t ppu_st;
    mem_cpu_t cpu_mem;
    mem_ppu_t ppu_mem;

    rom_t rom;

    disp_t disp;

} nes_state_t;

typedef struct {


} nes_input_t;

u8 nes_cpu_bus_read(u16 addr);
u8 nes_ppu_bus_read(u16 addr);
void nes_cpu_bus_write(u8 data, u16 addr);
void nes_ppu_bus_write(u8 data, u16 addr);

void nes_init(char* rom_path);
bool nes_should_exit();
void nes_update_kbinput();
void nes_step();
void nes_exit();

#endif
