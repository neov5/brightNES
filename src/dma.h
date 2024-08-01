// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 neov5

#ifndef __DMA_H__
#define __DMA_H__

#include "types.h"
#include "cpu.h"
#include "ppu.h"

typedef struct {
    u16 addr;
    bool enabled;
} dma_oam_t;

void dma_oam(dma_oam_t *dma, cpu_state_t *cpu_st, ppu_state_t *ppu_st);
void dma_dmc(dma_oam_t *dma, cpu_state_t *cpu_st, ppu_state_t *ppu_st);

#endif
