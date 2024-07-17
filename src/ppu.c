#include "ppu.h"
#include "cpu.h"
#include "disp.h"
#include <log.h>

void ppu_reset(ppu_state_t *st) {
    *((u8*)&st->ppuctrl) = 0;
    *((u8*)&st->ppumask) = 0;
    st->ppustatus.S = st->ppustatus.O = st->ppustatus.u = 0;
    st->ppuscroll = 0;
    // TODO
}

// https://www.nesdev.org/wiki/PPU_scrolling#Coarse_X_increment
void ppu_coarse_x_incr(ppu_state_t *st) {
    if (st->_v.X == 0x1F) {
        st->_v.X = 0;
        st->_v.N ^= 1; // switch horizontal nametable
    }
    else {
        st->_v.X++;
    }
}

// https://www.nesdev.org/wiki/PPU_scrolling#Y_increment
void ppu_y_incr(ppu_state_t *st) {
    if (st->_v.y < 7) {
        st->_v.y++;
    }
    else {
        st->_v.y = 0;
        if (st->_v.Y == 29) {
            st->_v.Y = 0;
            st->_v.N ^= 2; // switch vertical nametable
        }
        else if (st->_v.Y == 31) {
            st->_v.Y = 0;
        }
        else {
            st->_v.Y++;
        }
    }
}


void ppu_ppuctrl_write(ppu_state_t *st, ppu_ctrl_t data) {
    st->ppuctrl = data;
    st->_t.N = data.N;
}

u8 ppu_ppustatus_read(ppu_state_t *st) {
    st->_w = 0;
    return *(u8*)&st->ppustatus;
}

void ppu_ppuscroll_write(ppu_state_t *st, u8 data) {
    if (st->_w == 0) {
        st->_t.X = ((data & 0xF8)>>3);
        st->_x = (data & 0x7);
        st->_w = 1;
    }
    else {
        st->_t.Y = ((u16)(data & 0xF8)>>3);
        st->_t.y = (data & 0x7);
        st->_w = 0;
    }
}

void ppu_ppuaddr_write(ppu_state_t *st, u8 data) {
    u16* _t_raw = (u16*)(&st->_t);
    if (st->_w == 0) {
        *_t_raw = ((*_t_raw)&0xFF) | ((u16)(data & 0x3F)<<8);
        st->_w = 1;
    }
    else {
        *_t_raw = ((*_t_raw & 0xFF00) | data);
        st->_v = st->_t;
        st->_w = 0;
    }
}

u8 ppu_ppudata_read(ppu_state_t *st) {
    u8 retval = st->ppudata;
    st->ppudata = st->bus_read(*(u16*)(&st->_v));
    if (st->ppuctrl.I == 0) ppu_coarse_x_incr(st);
    else ppu_y_incr(st);
    return retval;
}

void ppu_ppudata_write(ppu_state_t *st, u8 data) {
    st->bus_write(data, *(u16*)(&st->_v));
    if (st->ppuctrl.I == 0) ppu_coarse_x_incr(st);
    else ppu_y_incr(st);
}

// TODO sprite evaluation

// Iteration 1: Don't do any sprite evaluation. Just render the background.

void ppu_shift_bufs(ppu_state_t *st) {
    st->_pix_sr |= ((u64)(st->_pix_buf)<<32);
}

void ppu_shift_pix_sr(ppu_state_t *st) {
    st->_pix_sr >>= 4;
}

void ppu_put_pixel(ppu_state_t *st, disp_t *disp) {
    u8 pixel = (st->_pix_sr & (0xFU << st->_x))>>st->_x;
    // 4 bit pixel lookup value
    // since this is a background sprite, look up the background palette (MSB=0)
    disp_putpixel(disp, st->_col-1, st->_row,
            st->_rgb_palette[pixel*3], st->_rgb_palette[pixel*3+1], st->_rgb_palette[pixel*3+2]);
}

void ppu_get_next_pixel(ppu_state_t *st) {
    if (st->_col % 8 == 2) {
        // nametable fetch
        u16 nt_addr = 0x2000 | ((*(u16*)(&st->_v)) & 0x0FFF);
        st->_pt_addr = st->bus_read(nt_addr);
        // log_info("tile ptr addr: 0x%hx", nt_addr);
        log_info("Pattern table ptr addr: 0x%hhx", st->_pt_addr);
    }
    else if (st->_col % 8 == 4) {
        // attribute table fetch
        ppu_at_addr_t at_addr = {
            .X = (st->_v.X)>>2,
            .Y = (st->_v.Y)>>2,
            .O = 0xFU,
            .N = st->_v.N,
            .u = 2
        };
        u8 at_blk = st->bus_read(*(u16*)(&at_addr));
        u8 shift_idx = (st->_v.X & 0x2)>>1 | (st->_v.Y & 0x2);
        u32 pal_idx = (at_blk & (0x3 << shift_idx)) >> shift_idx;
        st->_pix_buf = pal_idx;
        st->_pix_buf |= (st->_pix_buf << 4);
        st->_pix_buf |= (st->_pix_buf << 8);
        st->_pix_buf |= (st->_pix_buf << 16);
    }
    else if (st->_col % 8 == 6) {
        // BG LSBits fetch
        ppu_pt_addr_t pt_lsb_addr = {
            .y = st->_v.y,
            .P = 0,
            .N = st->_pt_addr,
            .H = st->ppuctrl.B,
            .Z = 0
        };
        u32 bg_lsb = st->bus_read(*(u16*)(&pt_lsb_addr));
        bg_lsb = (bg_lsb | (bg_lsb << 12)) & 0x000F000F;
        bg_lsb = (bg_lsb | (bg_lsb << 6)) & 0x03030303;
        bg_lsb = (bg_lsb | (bg_lsb << 3)) & 0x11111111;
        st->_pix_buf |= (bg_lsb << 3);
    }
    else if (st->_col % 8 == 0) {
        // BG MSBits fetch
        ppu_pt_addr_t pt_lsb_addr = {
            .y = st->_v.y,
            .P = 1,
            .N = st->_pt_addr,
            .H = st->ppuctrl.B,
            .Z = 0
        };
        u32 bg_msb = st->bus_read(*(u16*)(&pt_lsb_addr));
        bg_msb = (bg_msb | (bg_msb << 12)) & 0x000F000F;
        bg_msb = (bg_msb | (bg_msb << 6)) & 0x03030303;
        bg_msb = (bg_msb | (bg_msb << 3)) & 0x11111111;
        st->_pix_buf |= (bg_msb << 4);

        // TODO incr horiz v 
        ppu_shift_bufs(st);
        ppu_coarse_x_incr(st);
        if (st->_col == 256) ppu_y_incr(st);
    }
}

void ppu_load_vert_addr(ppu_state_t *st) {
    st->_v.y = st->_t.y;
    st->_v.Y = st->_t.Y;
    st->_v.N = (st->_v.N & 0x1) | (st->_t.N & 0x2);
}

void ppu_load_horiz_addr(ppu_state_t *st) {
    st->_v.X = st->_t.X;
    st->_v.N = (st->_v.N & 0x2) | (st->_t.N & 0x1);
}

void ppu_prerender_scanline_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {
    switch (ppu_st->_col) {
        case 1:
            ppu_st->ppustatus.V = ppu_st->ppustatus.S = ppu_st->ppustatus.O = 0;
            break;
        case 280 ... 304:
            // vert(v) = vert(t) if rendering enabled
            if (ppu_st->ppumask.b) ppu_load_vert_addr(ppu_st);
            break;
        case 321 ... 336:
            ppu_shift_pix_sr(ppu_st);
            ppu_get_next_pixel(ppu_st);
            break;
    }
}

// ? How accurate do we want this to be
void ppu_render_visible_scanline_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {
    if (ppu_st->_col == 0) {
        // idle cycle
    }
    else if (ppu_st->_col <= 256) {
        // blit current pixel onto display
        ppu_put_pixel(ppu_st, disp);
        ppu_shift_pix_sr(ppu_st);
        ppu_get_next_pixel(ppu_st);
    }
    else if (ppu_st->_col == 257) {
        ppu_load_horiz_addr(ppu_st);
        // hori(v) = hori(t)
    }
    else if (ppu_st->_col >= 321 && ppu_st->_col <= 336) {
        ppu_shift_pix_sr(ppu_st);
        ppu_get_next_pixel(ppu_st);
    }
    // TODO garbage nametable fetches - MMC5 uses them?
}

void ppu_postrender_scanline_tick(ppu_state_t *ppu_st, disp_t *disp) {
    if (ppu_st->_col == 1) {
        disp_blit(disp); 
        ppu_st->ppustatus.V = 1;
    }
}

void ppu_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {

    // tick forward and produce one pixel worth of data 
    // it's okay to coalesce processing logic and do multiple pixel writes in 
    // a single cycle

    // TODO sprite evaluation
    // TODO rendering (read nametable byte, attribute byte, pattern table lo/hi tile)

    switch (ppu_st->_row) {
        case 261: ppu_prerender_scanline_tick(ppu_st, cpu_st, disp); break;
        case 0 ... 239: ppu_render_visible_scanline_tick(ppu_st, cpu_st, disp); break;
        case 240: break;
        case 241: ppu_postrender_scanline_tick(ppu_st, disp); break;
    }

    ppu_st->_col = (ppu_st->_col+1)%341;
    if (ppu_st->_col == 0) ppu_st->_row = (ppu_st->_row+1)%262;

}
