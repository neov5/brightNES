#include "nes.h"
#include "log.h"
#include "parse_args.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char** argv) {

    char *palette_path = NULL;
    char *rom_path = NULL;

    args_option_t options[] = {
        ARGS_POSITIONAL_ARG(ARGTYPE_STRING, &rom_path),
        ARGS_OPTION("-p", "--palette", ARGTYPE_STRING, &palette_path),
        ARGS_END_OF_OPTIONS
    };

#ifdef NES_DEBUG
    log_add_fp(fopen("build/debug/brightnes.log", "w"));
#endif

    if (parse_arguments(argc, argv, options) < 0) {
        printf("usage: brightnes <rom_path> [-p|--palette palette_path]\n");
        return 0;
    }

    if (palette_path != NULL) {
        nes_load_palette(palette_path);
    }

    nes_init(argv[1]);


    bool exit = false;
    struct timespec tic, toc;
    timespec_get(&tic, TIME_UTC);
    while (!exit) {
        exit = nes_update_events();
        nes_render_frame();
#ifndef NES_DEBUG
        timespec_get(&toc, TIME_UTC);
        // 60fps
        long long ns_delta = toc.tv_nsec - tic.tv_nsec;
        if (toc.tv_sec - tic.tv_sec == 0 && ns_delta < 16666666LL) {
            // log_warn("Sleeping for %lld nanos", 16666666LL-ns_delta);
            nanosleep((struct timespec[]){{0, 16666666LL-ns_delta}}, NULL);
        }
        timespec_get(&tic, TIME_UTC);
#endif
    }

    nes_exit();

    return 0;
}
