#ifndef __DISP_H__
#define __DISP_H__

#include <SDL2/SDL.h>

typedef struct {

    SDL_Window *win;
    SDL_Renderer *r;

} disp_t;

int disp_init(disp_t *disp);
int disp_update(disp_t *disp);
int disp_free(disp_t *disp);

#endif
