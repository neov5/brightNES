#ifndef __MEM_C__
#define __MEM_C__

#include "types.h"

typedef struct {

    u8 wram[0x800];
    u8 apu_io_reg[0x20];

} mem_cpu_t;

typedef struct {
    u8 vram[0x800];
} mem_ppu_t;

#endif
