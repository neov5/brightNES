#include "nes.h"
#include <stdio.h>

int main(int argc, char** argv) {

    if (argc <= 1) {
        printf("ROM path not specified. Exiting.\n");
    }

    // TODO palette loading with -p|--palette

    nes_init(argv[1]);

    while (!nes_should_exit()) {
        nes_update_kbinput();
        nes_step();
    }

    nes_exit();

    return 0;
}
