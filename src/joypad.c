#include "joypad.h"
#include <SDL2/SDL.h>

void joypad_update(joypad_t *joypad) {
    joypad->state = 0;
    SDL_PumpEvents();
    int numkeys;
    const Uint8* kb_state = SDL_GetKeyboardState(&numkeys);
    if (kb_state[SDL_SCANCODE_Q]) joypad->state |= BTN_SELECT;
    if (kb_state[SDL_SCANCODE_W]) joypad->state |= BTN_START;
    if (kb_state[SDL_SCANCODE_A]) joypad->state |= BTN_B;
    if (kb_state[SDL_SCANCODE_S]) joypad->state |= BTN_A;
    if (kb_state[SDL_SCANCODE_UP]) joypad->state |= BTN_UP;
    if (kb_state[SDL_SCANCODE_DOWN]) joypad->state |= BTN_DOWN;
    if (kb_state[SDL_SCANCODE_RIGHT]) joypad->state |= BTN_RIGHT;
    if (kb_state[SDL_SCANCODE_LEFT]) joypad->state |= BTN_LEFT;
}

void joypad_write(joypad_t *joypad, u8 data) {
    joypad->strobe = data;
    if (joypad->strobe) {
        joypad->sr = joypad->state;
    }
}

u8 joypad_read(joypad_t *joypad) {
    if (joypad->strobe) return (joypad->state & 0x1);

    u8 bit = (joypad->sr & 0x1);
    joypad->sr = (joypad->sr >> 1) | 0x80;
    return bit;
}
