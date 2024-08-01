// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 neov5

#ifndef __CPU_H__
#define __CPU_H__

#include <inttypes.h>

typedef uint16_t u16;
typedef int16_t s16;
typedef uint8_t u8;
typedef int8_t s8;
typedef uint32_t u32;

typedef union {
    struct {
        u8 C: 1;
        u8 Z: 1;
        u8 I: 1;
        u8 D: 1;
        u8 B: 1;
        u8 u: 1;
        u8 V: 1;
        u8 N: 1;
    };
    u8 data;
} cpu_sr_t;

typedef struct {
    u8 A;
    u8 Y;
    u8 X;
    u16 PC;
    u8 S;
    cpu_sr_t P;

    u8 IRQ;
    u8 NMI;
    u8 RST;

    u8 (*bus_read)(u16);
    void (*bus_write)(u8, u16);

    void (*tick)(void); 

} cpu_state_t;

int cpu_exec(cpu_state_t *st);
void cpu_reset(cpu_state_t *st);
void cpu_state_to_str(cpu_state_t *st, char buf[64]);

#endif
