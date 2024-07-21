#include "nes.h"
#include "log.h"
#include "dma.h"

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

static char ppu_state_buf[128];
static char cpu_state_buf[64];

u8 nes_cpu_bus_read(u16 addr) {
    if (addr < 0x2000) return state.cpu_mem.wram[addr & 0x7FF];
    else if (addr < 0x4000) {
        u16 eaddr = addr & 0x7;
        switch (eaddr) {
            case 2: return ppu_ppustatus_read(&state.ppu_st);
            case 4: return ppu_oamdata_read(&state.ppu_st);
            case 7: return ppu_ppudata_read(&state.ppu_st);
            default: return ppu_iobus_read(&state.ppu_st);
        }
    }
    else if (addr < 0x4020) return state.cpu_mem.apu_io_reg[addr & 0x3F]; // TODO apu mapping
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
    else return ppu_palette_ram_read(&state.ppu_st, addr & 0x1F);
}

void nes_cpu_bus_write(u8 data, u16 addr) {
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
            default: log_fatal("Cannot write to address 0x%hx", addr); exit(-1);
        }
    }
    else if (addr < 0x4020) {
        if (addr == 0x4014) {
            state.dma_oam.enabled = true;
            state.dma_oam.addr = data;
        }
        else {
            state.cpu_mem.apu_io_reg[addr & 0x3F] = data;
        }
    }
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
    else ppu_palette_ram_write(&state.ppu_st, addr & 0x1F, data);
}

void nes_cpu_tick_callback() {
    // TODO loop unroll hinting via pragmas for GCC/clang
    state.cpu_cycle++;
    for (int i=0; i<3; i++) {
        ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
        state.ppu_cycle++;
        if (state.ppu_cycle % 1000 == 0) {
        }
    }
}

void nes_cpu_init(cpu_state_t *st) {
    st->tick = &nes_cpu_tick_callback;
    st->bus_read = &nes_cpu_bus_read;
    st->bus_write = &nes_cpu_bus_write;

    st->P.u = st->P.B = 1;
    st->RST = 1;
}

void nes_ppu_init(ppu_state_t *st) {
    st->bus_read = &nes_ppu_bus_read;
    st->bus_write = &nes_ppu_bus_write;
    // st->_row = 261;
    // st->_col = -1;

    memcpy(st->_rgb_palette, PALETTE_2C02_NTSC, 192);
}

void nes_init(char* rom_path) {
    disp_init(&state.disp);
    rom_load_from_file(&state.rom, rom_path);
    
    // cpu init code
    nes_cpu_init(&state.cpu_st);
    nes_ppu_init(&state.ppu_st);

    // ppu takes 4 cycles more than cpu?
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st, &state.disp);
    state.ppu_cycle++;
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

u64 breakpoint = 0;
bool stepping = true;
char input_buf[64];

void nes_render_frame() {
#ifdef NES_DEBUG
    while (!state.frame_done) {
        if (state.dma_oam.enabled) {
            u64 cin = state.cpu_cycle;
            if (state.cpu_cycle % 2 == 0) state.cpu_st.tick();
            dma_oam(&state.dma_oam, &state.cpu_st, &state.ppu_st);
            u64 cout = state.cpu_cycle;
            log_debug("DMA took %ld cycles", cout-cin);
            state.dma_oam.enabled = false;
        }
        cpu_exec(&state.cpu_st);
        if ((stepping && state.cpu_cycle >= breakpoint) || (!stepping)) {
            stepping = false;
            ppu_state_to_str(&state.ppu_st, ppu_state_buf);
            log_debug(ppu_state_buf);
            cpu_state_to_str(&state.cpu_st, cpu_state_buf);
            log_debug(cpu_state_buf);
            printf("> ");
            fgets(input_buf, 64, stdin);
            char ch = input_buf[0];
            if (ch == 'e') exit(0);
            else if (ch == 's') {
                breakpoint = strtol(input_buf+2, NULL, 10);
                stepping = true;
            }
        }
    }
    state.frame_done = false;
#else
    while (!state.frame_done) {
        if (state.dma_oam.enabled) {
            if (state.cpu_cycle % 2 != 0) state.cpu_st.tick();
            dma_oam(&state.dma_oam, &state.cpu_st, &state.ppu_st);
        }
        cpu_exec(&state.cpu_st);
    }
    state.frame_done = false;
#endif
}
