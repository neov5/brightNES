#include "nes.h"
#include <log.h>

nes_state_t state;

const u8 PALETTE_2C02_NTSC[192] = {
    0x52, 0x52, 0x52, 0x00, 0x00, 0x80, 0x08, 0x00, 0x80, 0x2e, 0x00, 0x7e, 0x4a, 0x00, 0x4e, 0x50,
    0x00, 0x06, 0x44, 0x00, 0x00, 0x26, 0x08, 0x00, 0x0a, 0x20, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x32,
    0x00, 0x00, 0x26, 0x0a, 0x00, 0x1c, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xa4, 0xa4, 0xa4, 0x00, 0x38, 0xce, 0x34, 0x16, 0xec, 0x5e, 0x04, 0xdc, 0x8c, 0x00, 0xb0, 0x9a,
    0x00, 0x4c, 0x90, 0x18, 0x00, 0x70, 0x36, 0x00, 0x4c, 0x54, 0x00, 0x0e, 0x6c, 0x00, 0x00, 0x74,
    0x00, 0x00, 0x6c, 0x2c, 0x00, 0x5e, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0x4c, 0x9c, 0xff, 0x7c, 0x78, 0xff, 0xa6, 0x64, 0xff, 0xda, 0x5a, 0xff, 0xf0,
    0x54, 0xc0, 0xf0, 0x6a, 0x56, 0xd6, 0x86, 0x10, 0xba, 0xa4, 0x00, 0x76, 0xc0, 0x00, 0x46, 0xcc,
    0x1a, 0x2e, 0xc8, 0x66, 0x34, 0xc2, 0xbe, 0x3a, 0x3a, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xb6, 0xda, 0xff, 0xc8, 0xca, 0xff, 0xda, 0xc2, 0xff, 0xf0, 0xbe, 0xff, 0xfc,
    0xbc, 0xee, 0xfa, 0xc2, 0xc0, 0xf2, 0xcc, 0xa2, 0xe6, 0xda, 0x92, 0xcc, 0xe6, 0x8e, 0xb8, 0xee,
    0xa2, 0xae, 0xea, 0xbe, 0xae, 0xe8, 0xe2, 0xb0, 0xb0, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


u8 nes_cpu_bus_read(u16 addr) {
    // log_debug("CPU bus reading from 0x%x", addr);
    if (addr < 0x2000) return state.cpu_mem.wram[addr & 0x7FF];
    else if (addr < 0x4000) {
        u16 eaddr = addr & 0x7;
        switch (eaddr) {
            case 2: return ppu_ppustatus_read(&state.ppu_st);
            case 4: return ppu_oamdata_read(&state.ppu_st);
            case 7: return ppu_ppudata_read(&state.ppu_st);
            default: log_fatal("Cannot read from address 0x%hx", addr); exit(1);
        }
    }
    else if (addr < 0x4020) return state.cpu_mem.apu_io_reg[addr & 0x3F]; // TODO apu mapping
    else return state.rom.mapper.cpu_read(&state.rom, addr & 0x7FFF); // only ROM map, no WRAM
}

u8 nes_ppu_bus_read(u16 addr) {
    // log_info("PPU bus reading from 0x%x", addr);
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
    // log_debug("CPU bus writing 0x%xb to 0x%x", data, addr);
    if (addr < 0x2000) state.cpu_mem.wram[addr & 0x7FF] = data;
    else if (addr < 0x4000) {
        u16 eaddr = addr & 0x7;
        switch (eaddr) {
            case 0: ppu_ppuctrl_write(&state.ppu_st, data); break;
            case 1: ppu_ppumask_write(&state.ppu_st, data); break;
            case 3: ppu_oamaddr_write(&state.ppu_st, data); break;
            case 4: ppu_oamdata_write(&state.ppu_st, data); break;
            case 5: ppu_ppuscroll_write(&state.ppu_st, data); break;
            case 6: ppu_ppuaddr_write(&state.ppu_st, data); break;
            case 7: ppu_ppudata_write(&state.ppu_st, data); break;
            default: log_fatal("Cannot write to address 0x%hx", addr); exit(1);
        }
    }
    else if (addr < 0x4020) state.cpu_mem.apu_io_reg[addr & 0x3F] = data;
    else state.rom.mapper.cpu_write(&state.rom, data, addr & 0x7FFF);
}

void nes_ppu_bus_write(u8 data, u16 addr) {
    // log_info("PPU bus writing 0x%hhx to 0x%hx", data, addr);
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
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    state.ppu_cycle++;
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
    st->_col = 0;
    st->_row = 261;

    st->bus_read = &nes_ppu_bus_read;
    st->bus_write = &nes_ppu_bus_write;

    memcpy(st->_rgb_palette, PALETTE_2C02_NTSC, 192);

    st->ppuctrl.data = 0;
    st->ppumask.data = 0;
    st->ppustatus.data = 0xA0;
    st->oamaddr = 0;
    st->_w = 0;
    st->ppuscroll = 0;
    st->ppuaddr = 0;
    st->ppudata = 0;

    st->_v.data = 0;
    st->_t.data = 0;
    st->_x = 0;
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
    while (!state.frame_done) {
        cpu_exec(&state.cpu_st);
        state.cpu_cycle++;
    }
    state.frame_done = false;
}
