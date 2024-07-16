#include "nes.h"
#include <log.h>
#include <stdio.h>


int main(int argc, char** argv) {

    log_set_level(LOG_INFO);

    if (argc <= 1) {
        printf("ROM path not specified. Exiting.\n");
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
