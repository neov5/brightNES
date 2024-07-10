#include "nes.h"
#include <stdio.h>

int main(int argc, char** argv) {

    if (argc <= 1) {
        printf("ROM path not specified. Exiting.\n");
    }

    nes_state_t nes;

    nes.rom = rom_load_from_file(argv[1]);

}
