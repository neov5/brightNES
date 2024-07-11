#include "disp.h"
#include <log.h>

#define guard(s, e) if (((e) = (s))) return (e);

int disp_init(disp_t *disp) {
    int err;
    guard(SDL_Init(SDL_INIT_VIDEO), err);

    disp->win = SDL_CreateWindow("brightNES", 100, 100, 256, 240, SDL_WINDOW_SHOWN);
    if (disp->win == NULL) {
        log_fatal("Could not create Window: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *r = SDL_CreateRenderer(disp->win, -1, SDL_RENDERER_ACCELERATED);
    if (r == NULL) {
        SDL_DestroyWindow(disp->win);
        log_fatal("Could not create Renderer: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    return 0;
}

int disp_free(disp_t *disp) {
    SDL_DestroyRenderer(disp->r);
    SDL_DestroyWindow(disp->win);
    return 0;
}
