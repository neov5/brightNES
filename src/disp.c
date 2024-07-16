#include "disp.h"
#include "types.h"
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

void disp_putpixel(disp_t *disp, u8 x, u8 y, u8 r, u8 g, u8 b) {
    SDL_SetRenderDrawColor(disp->r, r, g, b, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawPoint(disp->r, x, y);
}

void disp_blit(disp_t *disp) {
    SDL_RenderPresent(disp->r);
    SDL_RenderClear(disp->r);
}

int disp_free(disp_t *disp) {
    SDL_DestroyRenderer(disp->r);
    SDL_DestroyWindow(disp->win);
    return 0;
}
