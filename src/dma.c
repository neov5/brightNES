#include "dma.h"

// alignment will be handled externally
// this method takes 513 cycles
void dma_oam(dma_oam_t *dma, cpu_state_t *cpu_st, ppu_state_t *ppu_st) {
    cpu_st->tick();
    u16 addr = dma->addr <<= 8;
    for (u16 i=addr; i<=(addr | 0xFF); i++) {
        u8 val = cpu_st->bus_read(addr);
        cpu_st->tick();
        ppu_st->bus_write(val, 0x2004);
        cpu_st->tick();
    }
}