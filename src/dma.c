#include "dma.h"
#include "ppu.h"
#include "log.h"

// alignment will be handled externally
// this method takes 513 cycles
void dma_oam(dma_oam_t *dma, cpu_state_t *cpu_st, ppu_state_t *ppu_st) {
    cpu_st->tick();
    u16 addr = dma->addr <<= 8;
    log_debug("OAM Moving a page from 0x%hx", addr);
    for (u16 i=addr; i<=(addr | 0xFF); i++) {
        u8 val = cpu_st->bus_read(i);
        cpu_st->tick();
        ppu_oamdata_write(ppu_st, val);
        cpu_st->tick();
    }
}
