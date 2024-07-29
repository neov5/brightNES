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
