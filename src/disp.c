#include "disp.h"
#include "types.h"
#include "nes.h"
#include "log.h"
#include <time.h>

extern nes_state_t state;

// int disp_ctr = 0;
// clock_t tic;

int disp_init(disp_t *disp) {
    int err;
    if ((err = SDL_Init(SDL_INIT_VIDEO))) {
        log_fatal("Could not initialize SDL: %s", SDL_GetError());
        return err;
    }

#ifdef NES_DEBUG
    disp->win = SDL_CreateWindow("", 100, 100, 512, 480, SDL_WINDOW_SHOWN);
#else
    disp->win = SDL_CreateWindow("brightNES", 100, 100, 512, 480, SDL_WINDOW_SHOWN);
#endif
    if (disp->win == NULL) {
        log_fatal("Could not create Window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    disp->surf = SDL_GetWindowSurface(disp->win);
    if (disp->surf == NULL) {
        SDL_DestroyWindow(disp->win);
        log_fatal("Could not create Surface: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    return 0;
}

void disp_putpixel(disp_t *disp, u32 x, u32 y, u8 r, u8 g, u8 b) {
    x *= 2;
    y *= 2;
    Uint32 color = SDL_MapRGB(disp->surf->format, r, g, b);
    ((Uint32*)disp->surf->pixels)[(y*disp->surf->w) + x] = color;
    ((Uint32*)disp->surf->pixels)[((y+1)*disp->surf->w) + x] = color;
    ((Uint32*)disp->surf->pixels)[((y+1)*disp->surf->w) + x + 1] = color;
    ((Uint32*)disp->surf->pixels)[(y*disp->surf->w) + x + 1] = color;
}

void disp_blit(disp_t *disp) {
    SDL_UpdateWindowSurface(disp->win);
    state.frame_done = true;
    // clock_t toc = clock();
    // if (disp_ctr > 0) {
    //     double time = (double)(toc-tic)/CLOCKS_PER_SEC;
    //     log_info("fps: %f", 1./time);
    // }
    // tic = toc;
    // disp_ctr++;
}

int disp_free(disp_t *disp) {
    SDL_DestroyWindowSurface(disp->win);
    SDL_DestroyWindow(disp->win);
    return 0;
}
