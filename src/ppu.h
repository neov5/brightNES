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

typedef enum {
    PPU_BLANKING,
    PPU_RENDERING
} ppu_sm_t ;

typedef struct {

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
    
    ppu_sm_t state;
    
} ppu_state_t;

// ? should we tick the PPU forward or simply blit it
// ticking forward seems the best bet. 
// TODO lot of state machines!
bool ppu_tick(ppu_state_t *st, disp_t *disp);

#endif
