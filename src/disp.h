#ifndef __DISP_H__
#define __DISP_H__

#include <SDL2/SDL.h>
#include "types.h"

typedef struct {

    SDL_Window *win;
    SDL_Surface *surf;

} disp_t;

int disp_init(disp_t *disp);
void disp_putpixel(disp_t *disp, u32 x, u32 y, u8 r, u8 g, u8 b);
void disp_blit(disp_t *disp);
int disp_free(disp_t *disp);

#endif
