#include "nes.h"
#include <stdio.h>
#include "log.h"

int main(int argc, char** argv) {

#ifdef NES_DEBUG
    log_add_fp(fopen("build/debug/brightnes.log", "w"));
#endif

    if (argc <= 1) {
        printf("ROM path not specified. Exiting.\n");
        return 0;
    }

    // TODO palette loading with -p|--palette

    nes_init(argv[1]);

    bool exit = false;
    while (!exit) {
        exit = nes_update_events();
        nes_render_frame();
    }

    nes_exit();

    return 0;
}
