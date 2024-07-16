#include "nes.h"
#include <log.h>

nes_state_t state;

u8 nes_cpu_bus_read(u16 addr) {
    if (addr < 0x2000) return state.cpu_mem.wram[addr & 0x7FF];
    else if (addr < 0x4000) return ((u8*)(&state.ppu_st))[addr & 0x7];
    else if (addr < 0x4020) return state.cpu_mem.apu_io_reg[addr & 0x3F];
    else return state.rom.mapper.cpu_read(&state.rom, addr & 0x7FFF); // only ROM map, no WRAM
}

u8 nes_ppu_bus_read(u16 addr) {
    if (addr < 0x2000) return state.rom.mapper.ppu_read(&state.rom, addr);
    else if (addr < 0x3000) {
        u16 eaddr = addr & 0x7FF;
        if (state.rom.mirror_type == HORIZONTAL) {
            // bit 11 is MSB (AABB)
            return state.ppu_mem.vram[eaddr | ((addr & 0x800)>>1)];
        }
        else {
            // bit 10 is MSB (ABAB)
            return state.ppu_mem.vram[eaddr | (addr & 0x400)]; 
        }
        // TODO more mapping types
    }
    else if (addr < 0x3F00) return 0;
    else return state.ppu_st.palette_ram[addr & 0x1F];
}

void nes_cpu_bus_write(u8 data, u16 addr) {
    if (addr < 0x2000) state.cpu_mem.wram[addr & 0x7FF] = data;
    else if (addr < 0x4000) ((u8*)(&state.ppu_st))[addr & 0x7] = data;
    else if (addr < 0x4020) state.cpu_mem.apu_io_reg[addr & 0x3F] = data;
    else state.rom.mapper.cpu_write(&state.rom, data, addr & 0x7FFF);
}

void nes_ppu_bus_write(u8 data, u16 addr) {
    if (addr < 0x2000) state.rom.mapper.ppu_write(&state.rom, data, addr);
    else if (addr < 0x3000) {
        u16 eaddr = addr & 0x7FF;
        if (state.rom.mirror_type == HORIZONTAL) {
            state.ppu_mem.vram[eaddr | ((addr & 0x800)>>1)] = data;
        }
        else {
            state.ppu_mem.vram[eaddr | (addr & 0x400)] = data; 
        }
        // TODO more mapping types
    }
    else if (addr < 0x3F00) { /* do nothing */ }
    else state.ppu_st.palette_ram[addr & 0x1F] = data;
}

void nes_cpu_tick_callback() {
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
}

void nes_init(char* rom_path) {
    disp_init(&state.disp);
    rom_load_from_file(&state.rom, rom_path);
    
    log_debug("Loaded rom and initialized display");
    // cpu init code
    state.cpu_st.tick = &nes_cpu_tick_callback;
}

void nes_exit() {
    disp_free(&state.disp);
    rom_free(&state.rom);
}

void nes_update_kbinput() {

}

bool nes_should_exit() {
    // TODO check SDL input if we should exit
    return true;
}

void nes_step() {
    cpu_exec(&state.cpu_st);
}
