#include "ppu.h"

bool ppu_tick(ppu_state_t *st, display_t *disp) {

    // tick forward and produce one pixel worth of data 
    // it's okay to coalesce processing logic and do multiple pixel writes in 
    // a single cycle

    // TODO sprite evaluation
    // TODO rendering (read nametable byte, attribute byte, pattern table lo/hi tile)

}
