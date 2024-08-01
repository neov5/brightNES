#include "disp.h"
#include "types.h"
#include "nes.h"
#include "log.h"
#include <time.h>
#include <SDL2/SDL.h>

extern nes_state_t state;

static SDL_Window *win;
static SDL_Surface *surf;

int disp_init() {
    int err;
    if ((err = SDL_Init(SDL_INIT_VIDEO))) {
        log_fatal("Could not initialize SDL: %s", SDL_GetError());
        return err;
    }

#ifdef NES_DEBUG
    SDL_DisplayMode disp_mode;
    SDL_GetCurrentDisplayMode(0, &disp_mode);
    int screen_height = disp_mode.h;
    win = SDL_CreateWindow("brightNES (Debug Mode)", 0, screen_height-480, 512, 480, SDL_WINDOW_SHOWN);
#else
    win = SDL_CreateWindow("brightNES", 100, 100, 512, 480, SDL_WINDOW_SHOWN);
#endif
    if (win == NULL) {
        log_fatal("Could not create Window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    surf = SDL_GetWindowSurface(win);
    if (surf == NULL) {
        SDL_DestroyWindow(win);
        log_fatal("Could not create Surface: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    return 0;
}

void disp_putpixel(u32 x, u32 y, u8 r, u8 g, u8 b) {
    x *= 2;
    y *= 2;
    Uint32 color = SDL_MapRGB(surf->format, r, g, b);
    ((Uint32*)surf->pixels)[(y*surf->w) + x] = color;
    ((Uint32*)surf->pixels)[((y+1)*surf->w) + x] = color;
    ((Uint32*)surf->pixels)[((y+1)*surf->w) + x + 1] = color;
    ((Uint32*)surf->pixels)[(y*surf->w) + x + 1] = color;
}

void disp_blit() {
    SDL_UpdateWindowSurface(win);
    state.frame_done = true;
    // clock_t toc = clock();
    // if (disp_ctr > 0) {
    //     double time = (double)(toc-tic)/CLOCKS_PER_SEC;
    //     log_info("fps: %f", 1./time);
    // }
    // tic = toc;
    // disp_ctr++;
}

int disp_free() {
    SDL_DestroyWindowSurface(win);
    SDL_DestroyWindow(win);
    return 0;
}
