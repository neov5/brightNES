#include "nes.h"
#include "log.h"
#include "dma.h"
#include <errno.h>

nes_state_t state;

// credits: https://pixeltao.itch.io/pixeltao-cxa-nes-palette
u8 palette_memory[192] = {
    0x58, 0x58, 0x58, 0x00, 0x28, 0xa4, 0x00, 0x08, 0xc0, 0x4f, 0x1a, 0xa4, 0x7a, 0x1b, 0x77, 0x7f,
    0x09, 0x2d, 0x72, 0x00, 0x00, 0x56, 0x13, 0x00, 0x2e, 0x30, 0x00, 0x0f, 0x3d, 0x05, 0x02, 0x43,
    0x02, 0x00, 0x3f, 0x1f, 0x0d, 0x3c, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x9b, 0x9b, 0x9b, 0x00, 0x53, 0xee, 0x1a, 0x3f, 0xff, 0x71, 0x33, 0xff, 0xae, 0x2e, 0xd1, 0xd2,
    0x1e, 0x78, 0xd2, 0x2c, 0x00, 0xaa, 0x44, 0x00, 0x6c, 0x5e, 0x00, 0x2d, 0x73, 0x00, 0x00, 0x7d,
    0x06, 0x00, 0x78, 0x38, 0x00, 0x6b, 0x9c, 0x17, 0x17, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf7, 0xf7, 0xf7, 0x18, 0xa2, 0xff, 0x69, 0x87, 0xff, 0x95, 0x7a, 0xff, 0xd1, 0x6e, 0xff, 0xff,
    0x59, 0xcb, 0xf9, 0x70, 0x42, 0xfc, 0x8f, 0x04, 0xc9, 0xa4, 0x0e, 0x6c, 0xbf, 0x00, 0x33, 0xcf,
    0x38, 0x00, 0xc5, 0x53, 0x00, 0xb8, 0xc2, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf7, 0xf7, 0xf7, 0x9d, 0xd2, 0xff, 0xa7, 0xba, 0xff, 0xc4, 0xb5, 0xff, 0xec, 0xb6, 0xff, 0xff,
    0xb9, 0xe4, 0xff, 0xbe, 0xac, 0xf8, 0xcb, 0x89, 0xe6, 0xd4, 0x80, 0xbd, 0xd7, 0x76, 0x90, 0xe1,
    0x88, 0x8a, 0xe6, 0xb5, 0x7e, 0xe2, 0xe6, 0xac, 0xac, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
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
    else if (addr < 0x4020) {
        switch (addr) {
            case 0x4016: return joypad_read(&state.joypad);
            default: return state.cpu_mem.apu_io_reg[addr & 0x3F]; // TODO apu mapping
        }
    }
    else return state.rom.mapper.cpu_read(&state.rom, addr & 0x7FFF); // only ROM map, no WRAM
}

u8 nes_ppu_bus_read(u16 addr) {
    if (addr < 0x2000) return state.rom.mapper.ppu_read(&state.rom, addr);
    else if (addr < 0x3000) {
        u16 eaddr = addr & 0x7FF;
        u8 value = 0;
        if (state.rom.mirror_type == HORIZONTAL) {
            // bit 11 is MSB (AABB)
            value = state.ppu_mem.vram[eaddr | ((addr & 0x800)>>1)];
        }
        else {
            // bit 10 is MSB (ABAB)
            value = state.ppu_mem.vram[eaddr | (addr & 0x400)]; 
        }
        // TODO more mapping types
        return value;
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
        switch (addr) {
            case 0x4014:
                state.dma_oam.enabled = true;
                state.dma_oam.addr = data;
                break;
            case 0x4016: 
                joypad_write(&state.joypad, data);
                break;
            default: 
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
        ppu_tick(&state.ppu_st, &state.cpu_st);
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

    // the pixeltao palette looks better
    st->_rgb_palette = (u8*)palette_memory;
}

void nes_load_palette(char *palette_path) {

    FILE *palette_file = fopen(palette_path, "rb");
    if (palette_file == NULL) {
        log_fatal("Could not open palette file for reading: %s", strerror(errno));
        exit(0);
    }
    if (fread(palette_memory, 1, 192, palette_file) < 192) {
        log_fatal("Could not read all bytes of palette from file: %s", strerror(errno));
        exit(0);
    }

    if (fclose(palette_file)) {
        log_fatal("Could not close palette file: %s", strerror(errno));
        exit(0);
    }
}

void nes_init(char* rom_path) {
    disp_init();
    rom_load_from_file(&state.rom, rom_path);
    
    // cpu init code
    nes_cpu_init(&state.cpu_st);
    nes_ppu_init(&state.ppu_st);

    // ppu takes 4 cycles more than cpu? (source: mesen)
    // TODO debug why mesen's startup state is randomized (PPU takes 27 cycles
    // for excitebike and 25 on most other mapper 0 games)
    ppu_tick(&state.ppu_st, &state.cpu_st);
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st);
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st);
    state.ppu_cycle++;
    ppu_tick(&state.ppu_st, &state.cpu_st);
    state.ppu_cycle++;
}

void nes_exit() {
    disp_free();
    rom_free(&state.rom);
}

bool nes_update_events() {
    SDL_Event event;
    bool exit = false;
    while(SDL_PollEvent(&event)) {
        // TODO process other events too
        // TODO SDL stalls when the window moves to another display (tested 
        // on MacOS 14, moving from a retina to a non-retina display stalls 
        // the emulator. Moving back doesn't unstall it)
        if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT &&
                                       event.window.event == SDL_WINDOWEVENT_CLOSE)) {
            exit = true;
        }
    }
    joypad_update(&state.joypad);
    return exit;
}

#ifdef NES_DEBUG
u64 breakpoint = 0;
bool stepping = true;
char input_buf[64];
#endif

void nes_render_frame() {
#ifdef NES_DEBUG
    while (!state.frame_done) {
        if (state.dma_oam.enabled) {
            if (state.cpu_cycle % 2 == 0) state.cpu_st.tick();
            dma_oam(&state.dma_oam, &state.cpu_st, &state.ppu_st);
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
            if (state.cpu_cycle % 2 == 0) state.cpu_st.tick();
            dma_oam(&state.dma_oam, &state.cpu_st, &state.ppu_st);
            state.dma_oam.enabled = false;
        }
        cpu_exec(&state.cpu_st);
    }
    state.frame_done = false;
#endif
}
