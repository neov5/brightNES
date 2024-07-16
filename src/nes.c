#include "nes.h"
#include <log.h>

nes_state_t state;

u8 nes_cpu_bus_read(u16 addr) {
    log_debug("CPU bus reading from 0x%x", addr);
    if (addr < 0x2000) return state.cpu_mem.wram[addr & 0x7FF];
    else if (addr < 0x4000) {
        u16 eaddr = addr & 0x7;
        switch (eaddr) {
            case 2: return ppu_ppustatus_read(&state.ppu_st);
            case 7: return ppu_ppudata_read(&state.ppu_st);
            default: return ((u8*)(&state.ppu_st))[eaddr];
        }
    }
    else if (addr < 0x4020) return state.cpu_mem.apu_io_reg[addr & 0x3F]; // TODO apu mapping
    else return state.rom.mapper.cpu_read(&state.rom, addr & 0x7FFF); // only ROM map, no WRAM
}

u8 nes_ppu_bus_read(u16 addr) {
    log_debug("PPU bus reading from 0x%x", addr);
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
    log_debug("CPU bus writing 0x%xb to 0x%x", data, addr);
    if (addr < 0x2000) state.cpu_mem.wram[addr & 0x7FF] = data;
    else if (addr < 0x4000) {
        u16 eaddr = addr & 0x7;
        switch (eaddr) {
            case 0: ppu_ppuctrl_write(&state.ppu_st, *(ppu_ctrl_t*)(&data)); break;
            case 5: ppu_ppuscroll_write(&state.ppu_st, data); break;
            case 6: ppu_ppuaddr_write(&state.ppu_st, data); break;
            case 7: ppu_ppudata_write(&state.ppu_st, data); break;
            default: ((u8*)(&state.ppu_st))[eaddr] = data;
        }
    }
    else if (addr < 0x4020) state.cpu_mem.apu_io_reg[addr & 0x3F] = data;
    else state.rom.mapper.cpu_write(&state.rom, data, addr & 0x7FFF);
}

void nes_ppu_bus_write(u8 data, u16 addr) {
    log_debug("PPU bus writing 0x%xb to 0x%x", data, addr);
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

void nes_cpu_init(cpu_state_t *st) {
    st->tick = &nes_cpu_tick_callback;
    st->bus_read = &nes_cpu_bus_read;
    st->bus_write = &nes_cpu_bus_write;

    st->A = st->X = st->Y = 0;
    st->PC = 0;
    st->PC |= (st->bus_read(0xFFFC));
    st->PC |= ((u16)st->bus_read(0xFFFD))<<8;
    st->S = 0xFD;
    st->P.C = 0;
    st->P.Z = 0;
    st->P.I = 1;
    st->P.D = 0;
    st->P.V = 0;
    st->P.N = 0;
    st->P.u = st->P.B = 1;

    log_debug("CPU PC initialized to 0x%x", st->PC);
}

void nes_ppu_init(ppu_state_t *st) {
    st->_row = st->_col = 0;

    st->bus_read = &nes_ppu_bus_read;
    st->bus_write = &nes_ppu_bus_write;
}

void nes_init(char* rom_path) {
    disp_init(&state.disp);
    rom_load_from_file(&state.rom, rom_path);
    
    // cpu init code
    nes_cpu_init(&state.cpu_st);
    nes_ppu_init(&state.ppu_st);
    log_debug("NES initialized");
}

void nes_exit() {
    disp_free(&state.disp);
    rom_free(&state.rom);
}

bool nes_update_events() {
    SDL_Event event;
    bool exit = false;
    while(SDL_PollEvent(&event)) {
        // process other events too
        if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT &&
                                       event.window.event == SDL_WINDOWEVENT_CLOSE)) {
            exit = true;
        }
    }
    return exit;
}

void nes_render_frame() {
    while (!state._frame_done) {
        // FIXME do this or put check in display_blit?
        int enter_row = state.ppu_st._row, enter_col = state.ppu_st._col;
        cpu_exec(&state.cpu_st);
        if (state.ppu_st._row < enter_row && state.ppu_st._col < enter_col) state._frame_done = true;
    }
    state._frame_done = false;
}
