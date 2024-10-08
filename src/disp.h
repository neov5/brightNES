// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 neov5

#ifndef __DISP_H__
#define __DISP_H__

#include "types.h"

int disp_init();
void disp_putpixel(u32 x, u32 y, u8 r, u8 g, u8 b);
void disp_blit();
int disp_free();

#endif
