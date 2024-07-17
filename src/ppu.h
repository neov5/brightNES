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

// Original NTSC palette
// 64 colors, 24 bits each in .pal format (0xrr 0xgg 0xbb)
typedef struct {
    u8 N: 2; // LSB
    u8 I: 1;
    u8 S: 1;
    u8 B: 1;
    u8 H: 1;
    u8 P: 1;
    u8 V: 1; // MSB
} ppu_ctrl_t;

typedef struct {
    u8 S: 1; // LSB
    u8 m: 1;
    u8 M: 1;
    u8 b: 1;
    u8 s: 1;
    u8 R: 1;
    u8 G: 1;
    u8 B: 1; // MSB
} ppu_mask_t;

typedef struct {
    u8 u: 5; // LSB
    u8 O: 1;
    u8 S: 1;
    u8 V: 1; // MSB
} ppu_status_t;

typedef struct {
    u16 X: 5; // LSB
    u16 Y: 5;
    u16 N: 2;
    u16 y: 3;
    u16 u: 1; // MSB
} ppu_vram_addr_t;

typedef struct {
    u16 X: 3; // LSB
    u16 Y: 3;
    u16 O: 4;
    u16 N: 2;
    u16 u: 4; // MSB
} ppu_at_addr_t;

typedef struct {
    u16 y: 3; // LSB
    u16 P: 1;
    u16 N: 8;
    u16 H: 1;
    u16 Z: 1; // MSB
} ppu_pt_addr_t;

typedef struct {

    // Don't access these externally! Use the methods
    ppu_ctrl_t ppuctrl;
    ppu_mask_t ppumask;
    ppu_status_t ppustatus;
    u8 oamaddr;
    u8 oamdata;
    u8 ppuscroll;
    u8 ppuaddr;
    u8 ppudata;
    u8 oamdma;

    u8 oam[0x100];
    u8 palette_ram[0x20];

    u8 (*bus_read)(u16);
    void (*bus_write)(u8, u16);

    u16 _row;
    u16 _col;
    u8 _rgb_palette[192];

    u16 _ppuaddr;

    // internal registers
    ppu_vram_addr_t _v;
    ppu_vram_addr_t _t;
    u8 _x; // fine x scroll (3 bits)
    u8 _w; // write toggle (1 bit)

    u16 _addr_buf;

    u64 _pix_sr; // 4x8x2 (P2 P1), shifts right
    u8 _pt_addr;
    u32 _pix_buf; // 4x8

} ppu_state_t;

void ppu_ppuctrl_write(ppu_state_t *st, ppu_ctrl_t data);
u8 ppu_ppustatus_read(ppu_state_t *st);
void ppu_ppuscroll_write(ppu_state_t *st, u8 data);
void ppu_ppuaddr_write(ppu_state_t *st, u8 data);
u8 ppu_ppudata_read(ppu_state_t *st);
void ppu_ppudata_write(ppu_state_t *st, u8 data);

// ? should we tick the PPU forward or simply blit it
// ticking forward seems the best bet. 
// TODO lot of state machines!
void ppu_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp);
void ppu_init(ppu_state_t *st);

#endif
