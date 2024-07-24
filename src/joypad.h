#ifndef __JOYPAD_H__
#define __JOYPAD_H__

#include "types.h"
#include <stdbool.h>

typedef enum {
    BTN_A = 0x1,
    BTN_B = 0x2,
    BTN_SELECT = 0x4,
    BTN_START = 0x8,
    BTN_UP = 0x10,
    BTN_DOWN = 0x20,
    BTN_LEFT = 0x40,
    BTN_RIGHT = 0x80
} joypad_btn_t;

typedef struct {
    u8 state;
    u8 sr;
    bool strobe;
} joypad_t;

void joypad_update(joypad_t *joypad);
void joypad_write(joypad_t *joypad, u8 data);
u8 joypad_read(joypad_t *joypad);

#endif
