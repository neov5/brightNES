#include "ppu.h"
#include "cpu.h"
#include "disp.h"
#include "log.h"

void ppu_state_to_str(ppu_state_t* st, char buf[128]) {
    snprintf(buf, 128, "[PPU r:%03hd c:%03hd iobus: %02hhx ctrl:%02hhx mask:%02hhx status:%02hhx v:%04hx t:%04hx x:%01hx w:%d addr:%04hx]", 
            st->_row, st->_col, st->_io_bus, st->ppuctrl.data, st->ppumask.data, st->ppustatus.data, st->_v.data, 
            st->_t.data, st->_x, st->_w, st->_ppuaddr);
}

void ppu_reset(ppu_state_t *st) {
    st->ppuctrl.data = 0;
    st->ppumask.data = 0;
    st->ppustatus.S = st->ppustatus.O = st->ppustatus.u = 0;
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

u8 ppu_oamdata_read(ppu_state_t *st) {
    if (st->_row >= 0 && st->_row <= 239 && st->_col >= 1 && st->_col <= 64) return 0xFF;
    return st->oam.data[st->oamaddr];
}

void ppu_oamdata_write(ppu_state_t *st, u8 data) {
    log_info("OAM Writing 0x%hhx to 0x%hx", data, st->oamaddr);
    st->oam.data[st->oamaddr++] = data;
}

void ppu_oamaddr_write(ppu_state_t *st, u8 data) {
    st->oamaddr = data;
}

void ppu_ppuctrl_write(ppu_state_t *st, u8 data) {
    st->_io_bus = data;
    st->ppuctrl.data = st->_io_bus;
    st->_t.N = st->ppuctrl.N;
}

u8 ppu_ppustatus_read(ppu_state_t *st) {
    st->_w = 0;
    st->_io_bus = (st->ppustatus.data & 0xE0) | (st->_io_bus & 0x1F);
    st->ppustatus.V = 0;
    return st->_io_bus;
}

u8 ppu_iobus_read(ppu_state_t *st) {
    return st->_io_bus;
}

void ppu_ppuscroll_write(ppu_state_t *st, u8 data) {
    st->_io_bus = data;
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
    st->_io_bus = data;
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
    // ROM, RAM takes one extra cycle to read this from outbound memory
    // palette accesses are done immediately
    // however, the CPU will have a dummy read so this is ok to do for both
    st->_io_bus = st->bus_read(st->_v.data);
    if (st->ppuctrl.I == 0) {
        st->_v.data++;
    }
    else {
        st->_v.data += 32;
    }
    return st->_io_bus;
}

void ppu_ppumask_write(ppu_state_t *st, u8 data) {
    st->_io_bus = data;
    st->ppumask.data = data;
}

void ppu_ppudata_write(ppu_state_t *st, u8 data) {
    st->_io_bus = data;
    st->bus_write(data, st->_v.data);
    if (st->ppuctrl.I == 0) {
        st->_v.data++;
    }
    else {
        st->_v.data += 32;
    }
}

u8 ppu_palette_ram_read(ppu_state_t *st, u8 addr) {
    switch (addr) {
        // mirrors
        case 0x00: case 0x10:
        case 0x04: case 0x14:
        case 0x08: case 0x18:
        case 0x0C: case 0x1C: 
            return st->palette_ram[addr & 0xF];
        default: 
            return st->palette_ram[addr];
    }
}

void ppu_palette_ram_write(ppu_state_t *st, u8 addr, u8 data) {
    switch (addr) {
        // mirrors
        case 0x00: case 0x10:
        case 0x04: case 0x14:
        case 0x08: case 0x18:
        case 0x0C: case 0x1C: 
            st->palette_ram[addr & 0xF] = data; break;
        default: 
            st->palette_ram[addr] = data; break;
    }
}

// TODO sprite evaluation

// Iteration 1: Don't do any sprite evaluation. Just render the background.

// TODO better name
void ppu_shift_bufs(ppu_state_t *st) {
    st->_pix_sr |= st->_pix_buf;
}

void ppu_shift_pix_sr(ppu_state_t *st) {
    st->_pix_sr <<= 4;
}

void ppu_put_pixel(ppu_state_t *st, disp_t *disp) {
    u8 shift = 60 - (st->_x*4);
    u64 mask = 0xFULL << shift;
    u8 bg_pixel = (st->_pix_sr & mask)>>shift;

    u8 sprite_pixel = 0;
    bool sprite_priority = false;
    bool sprite_pixel_found = false;
    for (int i=0; i<st->_num_sprites_on_curr_scanline; i++) {
        if (st->_sprite_ctrs[i] <= 7 && st->_sprite_ctrs[i] >= 0) {
            // TODO sprite priority quirk with BG sprites
            sprite_pixel_found = true;
            shift = st->_sprite_ctrs[i]*4;
            mask = 0xFU << shift;
            sprite_pixel = (st->_sprite_srs[i] & mask)>>shift;
            sprite_priority = st->_sprite_priorities[i];
            // this is a higher priority sprite than other sprites which may be drawn
            break;
        }
    }
    sprite_pixel |= 0x10; // sprite palette is from 0x10-0x1F

    u8 final_pixel = 0;
    if (!sprite_pixel_found) {
        final_pixel = bg_pixel;
    }
    else {
        // priority mux (https://www.nesdev.org/wiki/PPU_rendering#Picture_region)
        if ((bg_pixel & 0x3) != 0 && (sprite_pixel & 0x3) != 0) {
            if (sprite_priority) final_pixel = bg_pixel; // yes, sprite priority 1 = bg pixel
            else final_pixel = sprite_pixel;
        }
        else if ((bg_pixel & 0x3) == 0 && (sprite_pixel & 0x3) == 0) {
            final_pixel = 0;
        }
        else if ((bg_pixel & 0x3) == 0 && (sprite_pixel & 0x3) != 0) {
            final_pixel = sprite_pixel;
        }
        else if ((bg_pixel & 0x3) != 0 && (sprite_pixel & 0x3) == 0) {
            final_pixel = bg_pixel;
        }
    }

    u8 palette_idx = ppu_palette_ram_read(st, final_pixel);

    disp_putpixel(disp, st->_col-1, st->_row,
            st->_rgb_palette[palette_idx*3], st->_rgb_palette[palette_idx*3+1], st->_rgb_palette[palette_idx*3+2]);
}

void ppu_get_next_pixel(ppu_state_t *st) {
    if (st->_col % 8 == 2) {
        // nametable fetch
        u16 nt_addr = 0x2000 | (st->_v.data & 0x0FFF);
        st->_pt_addr = st->bus_read(nt_addr);
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
        u8 at_blk = st->bus_read(at_addr.data);
        u8 shift_idx = (st->_v.X & 0x2)>>1 | (st->_v.Y & 0x2);
        shift_idx *= 2;
        u32 pal_idx = (at_blk & (0x3 << shift_idx)) >> shift_idx;
        st->_pix_buf = pal_idx;
        st->_pix_buf |= (st->_pix_buf << 4);
        st->_pix_buf |= (st->_pix_buf << 8);
        st->_pix_buf |= (st->_pix_buf << 16);
        st->_pix_buf <<= 2;
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
        u8 lsb = st->bus_read(pt_lsb_addr.data);
        u32 bg_lsb = lsb;
        bg_lsb = (bg_lsb | (bg_lsb << 12)) & 0x000F000F;
        bg_lsb = (bg_lsb | (bg_lsb << 6))  & 0x03030303;
        bg_lsb = (bg_lsb | (bg_lsb << 3))  & 0x11111111;
        st->_pix_buf |= bg_lsb;
    }
    else if (st->_col % 8 == 0) {
        // BG MSBits fetch
        ppu_pt_addr_t pt_msb_addr = {
            .y = st->_v.y,
            .P = 1,
            .N = st->_pt_addr,
            .H = st->ppuctrl.B,
            .Z = 0
        };
        u8 msb = st->bus_read(pt_msb_addr.data);
        u32 bg_msb = msb;
        bg_msb = (bg_msb | (bg_msb << 12)) & 0x000F000F;
        bg_msb = (bg_msb | (bg_msb << 6))  & 0x03030303;
        bg_msb = (bg_msb | (bg_msb << 3))  & 0x11111111;
        st->_pix_buf |= (bg_msb << 1);

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
        case 8 ... 256:
            if (ppu_st->ppumask.b) {
                if (ppu_st->_col % 8 == 0) ppu_coarse_x_incr(ppu_st);
                if (ppu_st->_col == 256) ppu_y_incr(ppu_st);
            }
            break;
        case 257:
            if (ppu_st->ppumask.b) {
                ppu_load_horiz_addr(ppu_st);
            }
            break;
        case 280 ... 304:
            // vert(v) = vert(t) if rendering enabled
            if (ppu_st->ppumask.b) {
                ppu_load_vert_addr(ppu_st);
            }
            break;
        case 321 ... 336:
            if (ppu_st->ppumask.b) {
                ppu_shift_pix_sr(ppu_st);
                ppu_get_next_pixel(ppu_st);
            }
            break;
        case 339:
            if (!ppu_st->_init_done) {
                ppu_st->_init_done = true;
                ppu_st->_frame_ctr = 0;
            }
            else {
                if (ppu_st->ppumask.b || ppu_st->ppumask.s) {
                    // if rendering is enabled, skip the last cycle on odd frames
                    // TODO confirm 'rendering enabled' = either b or s is on
                    if (ppu_st->_frame_ctr >= 2 && ppu_st->_frame_ctr % 2 == 0) {
                        ppu_st->_col++; // skip on odd;
                    }
                }
                ppu_st->_frame_ctr++;
            }
            break;
    }
}

// https://www.nesdev.org/wiki/PPU_sprite_evaluation
void ppu_sprite_eval(ppu_state_t *st) {
    if (st->_col < 64) {
        // do nothing
    }
    else if (st->_col == 64) {
        memset(st->sec_oam.data, 0xFF, 32);
        st->_oam_ctr = 0;
        st->_sec_oam_ctr = 0;
    }
    else if (st->_col <= 256 && st->_oam_ctr < 64) {
        // TODO is the _oam_ctr check correct?
        //
        // instead of the odd-even cycle thing, do the read and write both 
        // on the even cycle
        if (st->_col % 2 == 0) {
            // range check
            ppu_sprite_t sp = st->oam.sprites[st->_oam_ctr];
            // log_info("Evaluating Sprite %d: (%hhx,%hhx,%hhx,%hhx)", st->_oam_ctr, sp.y, sp.index, sp.attr, sp.x);
            if (sp.y <= st->_row && sp.y >= st->_row-7) {
                // in range
                // log_info("Sprite %d: (%hhx,%hhx,%hhx,%hhx) in range", st->_oam_ctr, sp.y, sp.index, sp.attr, sp.x);
                if (st->_sec_oam_ctr == 8) return;
                st->sec_oam.sprites[st->_sec_oam_ctr++] = sp;
            }
            // TODO sprite overflow (step 2.3)
            if (st->_oam_ctr < 63) st->_oam_ctr++;
        }
    }
}

void ppu_sprite_reload(ppu_state_t *st) {
    if (st->_num_sprites_on_next_scanline > 0) {
        log_info("Got %d sprites on next scanline", st->_num_sprites_on_next_scanline);
    }
    st->_num_sprites_on_curr_scanline = st->_num_sprites_on_next_scanline;
    for (int i=0; i<st->_num_sprites_on_curr_scanline; i++) {
        st->_sprite_ctrs[i] = st->sec_oam.sprites[i].x + 7;
        st->_sprite_priorities[i] = (st->sec_oam.sprites[i].attr & 0x20);
    }
}

void ppu_sprite_fetch(ppu_state_t *st) {
    // sprite fetches
    // similar to rendering the background sprites, load into the ith shift
    // register
    //
    // 8 cyc/sprite. cyc 1-4: read from OAM, cyc 5-8: fetch sprite tiledata 
    // TODO 8x16 sprites

    if (st->_col == 257) {
        st->_num_sprites_on_next_scanline = st->_sec_oam_ctr;
        st->_sec_oam_ctr = 0;
    }
    if (st->_sec_oam_ctr >= st->_num_sprites_on_next_scanline) return;

    ppu_sprite_t sprite = st->sec_oam.sprites[st->_sec_oam_ctr];
    u32* sprite_sr = &st->_sprite_srs[st->_sec_oam_ctr];

    if (st->_col % 8 == 4) {
        // shift the palette in
        u32 pal = (sprite.attr & 0x3) << 2;
        *sprite_sr = pal;
        *sprite_sr |= (*sprite_sr << 4);
        *sprite_sr |= (*sprite_sr << 8);
        *sprite_sr |= (*sprite_sr << 16);
    }
    else if (st->_col % 8 == 6) {
        // Sprite LSBits fetch
        u8 y_addr = st->_row - sprite.y;
        if (sprite.attr & 0x80) y_addr = 7-y_addr; // vertical flip
        ppu_pt_addr_t pt_lsb_addr = {
            .y = y_addr,
            .P = 0,
            .N = sprite.index,
            .H = st->ppuctrl.S,
            .Z = 0
        };
        u8 lsb = st->bus_read(pt_lsb_addr.data);
        u32 bg_lsb = lsb;
        if (sprite.attr & 0x40) { // horizontal flip
            bg_lsb = ((bg_lsb >> 4) | (bg_lsb << 16)) & 0x000F000F;
            bg_lsb = ((bg_lsb >> 2) | (bg_lsb << 8))  & 0x03030303;
            bg_lsb = ((bg_lsb >> 1) | (bg_lsb << 4))  & 0x11111111;
        }
        else {
            bg_lsb = (bg_lsb | (bg_lsb << 12)) & 0x000F000F;
            bg_lsb = (bg_lsb | (bg_lsb << 6))  & 0x03030303;
            bg_lsb = (bg_lsb | (bg_lsb << 3))  & 0x11111111;
        }
        *sprite_sr |= bg_lsb;
    }
    else if (st->_col % 8 == 0) {
        // Sprite MSBits fetch
        u8 y_addr = st->_row - sprite.y;
        if (sprite.attr & 0x80) y_addr = 7-y_addr; // vertical flip
        ppu_pt_addr_t pt_msb_addr = {
            .y = y_addr,
            .P = 1,
            .N = sprite.index,
            .H = st->ppuctrl.S,
            .Z = 0
        };
        u8 msb = st->bus_read(pt_msb_addr.data);
        u32 bg_msb = msb;
        if (sprite.attr & 0x40) { // horizontal flip
            bg_msb = ((bg_msb >> 4) | (bg_msb << 16)) & 0x000F000F;
            bg_msb = ((bg_msb >> 2) | (bg_msb << 8))  & 0x03030303;
            bg_msb = ((bg_msb >> 1) | (bg_msb << 4))  & 0x11111111;
        }
        else {
            bg_msb = (bg_msb | (bg_msb << 12)) & 0x000F000F;
            bg_msb = (bg_msb | (bg_msb << 6))  & 0x03030303;
            bg_msb = (bg_msb | (bg_msb << 3))  & 0x11111111;
        }
        *sprite_sr |= (bg_msb << 1);

        st->_sec_oam_ctr++;
    }
}

void ppu_sprite_update(ppu_state_t *st) {
    if (st->_col <= 256) {
        for (int i=0; i<st->_num_sprites_on_curr_scanline; i++) {
            st->_sprite_ctrs[i]--;
        }
    }
}

void ppu_render_visible_scanline_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {
    if (ppu_st->_col == 0) {
        // idle cycle
    }
    else if (ppu_st->_col <= 256) {
        // blit current pixel onto display
        if (ppu_st->ppumask.b) {
            if (ppu_st->ppumask.s) {
                ppu_sprite_eval(ppu_st);
            }
            ppu_put_pixel(ppu_st, disp);
            ppu_sprite_update(ppu_st);
            ppu_shift_pix_sr(ppu_st);
            ppu_get_next_pixel(ppu_st);
        }
    }
    else if (ppu_st->_col <= 320) {
        if (ppu_st->_col == 257 && ppu_st->ppumask.b) ppu_load_horiz_addr(ppu_st);
        if (ppu_st->ppumask.s) ppu_sprite_fetch(ppu_st);

    }
    else if (ppu_st->_col <= 336) {
        if (ppu_st->_col == 321 && ppu_st->ppumask.s) ppu_sprite_reload(ppu_st);
        if (ppu_st->ppumask.b) {
            ppu_shift_pix_sr(ppu_st);
            ppu_get_next_pixel(ppu_st);
        }
    }
    // TODO garbage nametable fetches - MMC5 uses them?
}

void ppu_postrender_scanline_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {
    if (ppu_st->_col == 1) {
        if (ppu_st->ppumask.b) disp_blit(disp); 
        ppu_st->ppustatus.V = 1;
    }
    else if (ppu_st->_col == 4) {
        if (ppu_st->ppuctrl.V == 1) cpu_st->NMI = 1;
    }
}

// TODO we are one cycle behind on some frames. I guess it's a timing issue.
// Check the skip cycle.
//
// The bug is right above, with the timing of NMI mattering. Some times the 
// instruction after the NMI is supposed to execute, and it doesn't.
// Also the skip cycle can be disabled!

void ppu_tick(ppu_state_t *ppu_st, cpu_state_t *cpu_st, disp_t *disp) {

    ppu_st->_col = (ppu_st->_col+1)%341;
    if (ppu_st->_col == 0) ppu_st->_row = (ppu_st->_row+1)%262;
    // tick forward and produce one pixel worth of data 
    // it's okay to coalesce processing logic and do multiple pixel writes in 
    // a single cycle

    // TODO sprite evaluation

    switch (ppu_st->_row) {
        case 261: ppu_prerender_scanline_tick(ppu_st, cpu_st, disp); break;
        case 0 ... 239: ppu_render_visible_scanline_tick(ppu_st, cpu_st, disp); break;
        case 241: ppu_postrender_scanline_tick(ppu_st, cpu_st, disp); break;
    }


}
