#include "ppu.h"
#include "cpu.h"

// TODO sprite evaluation
// TODO y increment and x increment in v

void ppu_coarse_x_incr(ppu_state_t *st) {

}

void ppu_coarse_y_incr(ppu_state_t *st) {

}

void ppu_prerender_scanline_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {
    switch (ppu_st->_col) {
        case 1:
            ppu_st->ppustatus.V = ppu_st->ppustatus.S = ppu_st->ppustatus.O = 0;
        case 2:

    }
}

void ppu_render_visible_scanline_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {

}

void ppu_postrender_scanline_tick(ppu_state_t *ppu_st) {
    if (ppu_st->_col == 1) ppu_st->ppustatus.V = 1;
}

void ppu_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {

    // tick forward and produce one pixel worth of data 
    // it's okay to coalesce processing logic and do multiple pixel writes in 
    // a single cycle

    // TODO sprite evaluation
    // TODO rendering (read nametable byte, attribute byte, pattern table lo/hi tile)

    switch (ppu_st->_row) {
        case 261: ppu_prerender_scanline_tick(ppu_st, cpu_st, disp); break;
        case 0 ... 239: ppu_render_visible_scanline_tick(ppu_st, cpu_st, disp); break;
        case 241: ppu_postrender_scanline_tick(ppu_st); break;
    }

    ppu_st->_col = (ppu_st->_col+1)%341;
    if (ppu_st->_col == 0) ppu_st->_row = (ppu_st->_row+1)%262;

}
