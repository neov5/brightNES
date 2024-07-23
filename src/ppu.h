#ifndef __PPU_H__
#define __PPU_H__

#define PPUCTRL 0x2000
#define PPUMASK 0x2001
#define PPUSTATUS 0x2002
#define OAMADDR 0x2003
#define OAMDATA 0x2003
#define PPUSCROLL 0x2005
#define PPUADDR 0x2006
#define PPUDATA 0x2007

#include <stdbool.h>

#include "types.h"
#include "disp.h"
#include "cpu.h"

typedef union {
    struct {
        u8 N: 2; // LSB
        u8 I: 1;
        u8 S: 1;
        u8 B: 1;
        u8 H: 1;
        u8 P: 1;
        u8 V: 1; // MSB
    };
    u8 data;
} ppu_ctrl_t;

typedef union {
    struct {
        u8 S: 1; // LSB
        u8 m: 1;
        u8 M: 1;
        u8 b: 1;
        u8 s: 1;
        u8 R: 1;
        u8 G: 1;
        u8 B: 1; // MSB
    };
    u8 data;
} ppu_mask_t;

typedef union {
    struct {
        u8 u: 5; // LSB
        u8 O: 1;
        u8 S: 1;
        u8 V: 1; // MSB
    };
    u8 data;
} ppu_status_t;

typedef union {
    struct {
        u16 X: 5; // LSB
        u16 Y: 5;
        u16 N: 2;
        u16 y: 3;
        u16 u: 1; // MSB
    };
    u16 data;
} ppu_vram_addr_t;

typedef union {
    struct {
        u16 X: 3; // LSB
        u16 Y: 3;
        u16 O: 4;
        u16 N: 2;
        u16 u: 4; // MSB
    };
    u16 data;
} ppu_at_addr_t;

typedef union {
    struct {
        u16 y: 3; // LSB
        u16 P: 1;
        u16 N: 8;
        u16 H: 1;
        u16 Z: 3; // MSB
    };
    u16 data;
} ppu_pt_addr_t;

typedef union {
    struct {
        u8 y;    // LSB
        u8 index;
        u8 attr;
        u8 x;    // MSB
    };
    u32 data;
} ppu_sprite_t;

typedef struct {

    // Don't access these externally! Use the methods
    ppu_ctrl_t ppuctrl;
    ppu_mask_t ppumask;
    ppu_status_t ppustatus;
    u8 oamaddr;
    u8 oamdata;
    u8 oamdma;

    union {
        ppu_sprite_t sprites[64];
        u8 data[256];
    } oam;

    union {
        ppu_sprite_t sprites[8];
        u8 data[32];
    } sec_oam;

    u32 _sprite_srs[8]; // 4 x 8, shifts left
    s16 _sprite_ctrs[8];
    bool _sprite_priorities[8];

    u8 _oam_ctr;
    u8 _sec_oam_ctr;

    u8 _num_sprites_on_next_scanline;
    u8 _num_sprites_on_curr_scanline;

    u8 palette_ram[0x20];

    u8 (*bus_read)(u16);
    void (*bus_write)(u8, u16);

    bool _init_done;
    bool _odd_frame;
    u64 _frame_ctr;

    s16 _row;
    s16 _col;
    u8 _rgb_palette[192];

    u16 _ppuaddr;
    u8 _io_bus;

    // internal registers
    ppu_vram_addr_t _v;
    ppu_vram_addr_t _t;
    u8 _x; // fine x scroll (3 bits)
    u8 _w; // write toggle (1 bit)

    u64 _pix_sr; // 4x8x2 (P2 P1), shifts right
    u8 _pt_addr;
    u32 _pix_buf; // 4x8

} ppu_state_t;

void ppu_ppuctrl_write(ppu_state_t *st, u8 data);
void ppu_ppumask_write(ppu_state_t *st, u8 data);
u8 ppu_ppustatus_read(ppu_state_t *st);
void ppu_ppuscroll_write(ppu_state_t *st, u8 data);
void ppu_oamaddr_write(ppu_state_t *st, u8 data);
u8 ppu_oamdata_read(ppu_state_t *st);
void ppu_oamdata_write(ppu_state_t *st, u8 data);
void ppu_ppuaddr_write(ppu_state_t *st, u8 data);
u8 ppu_ppudata_read(ppu_state_t *st);
void ppu_ppudata_write(ppu_state_t *st, u8 data);
u8 ppu_iobus_read(ppu_state_t *st);
void ppu_palette_ram_write(ppu_state_t *st, u8 addr, u8 data);
u8 ppu_palette_ram_read(ppu_state_t *st, u8 addr);

void ppu_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp);
void ppu_init(ppu_state_t *st);
void ppu_state_to_str(ppu_state_t *st, char buf[128]);

#endif
