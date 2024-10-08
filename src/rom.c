// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2024 neov5

#include "rom.h"
#include "log.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static const char MAGIC[4] = { 0x4E, 0x45, 0x53, 0x1A }; // NES\r

void rom_load_from_file(rom_t *rom, char* filename) {

    char header[16];

    FILE *rom_file = fopen(filename, "rb");
    if (rom_file == NULL) {
        log_fatal("Could not open palette file for reading: error %s", strerror(errno));
        exit(0);
    }
    // TODO add error checks if we couldn't read
    fread(header, 16, 1, rom_file);

    if (memcmp(header, MAGIC, 4) != 0) {
        log_fatal("File header does not match iNES magic: "
                  "file is not a NES file or is corrupted");
        exit(0);
    }

    rom->prg_rom_size = 16*0x400*header[4]; // 16 kB * val
    rom->chr_rom_size = 8*0x400*header[5]; // 8 kB * val

    u8 mapper = ((header[6] & 0xF0)>>4) | (header[7] & 0xF0);
    rom->mirror_type = (header[6] & 1) ? HORIZONTAL : VERTICAL;
    if (header[6] & 0x4) fseek(rom_file, 512, SEEK_CUR);
    rom->prg_rom = malloc(rom->prg_rom_size);
    rom->chr_rom = malloc(rom->chr_rom_size);

    fread(rom->prg_rom, rom->prg_rom_size, 1, rom_file);
    fread(rom->chr_rom, rom->chr_rom_size, 1, rom_file);

    fclose(rom_file);

    switch (mapper) {
        case 0x00: 
            rom->mapper = (rom_mapper_t){
                           .type = NONE,
                           .cpu_read = &no_mapper_cpu_read,
                           .cpu_write = &no_mapper_cpu_write,
                           .ppu_read = &no_mapper_ppu_read,
                           .ppu_write = &no_mapper_ppu_write
                          }; 
            break;
        // TODO more mappers
        default:
            log_fatal("Mapper %d is not supported currently\n", mapper);
            exit(0);
    }

}

u8 no_mapper_cpu_read(rom_t *rom, u16 addr) {
    return rom->prg_rom[addr % rom->prg_rom_size];
}

u8 no_mapper_ppu_read(rom_t *rom, u16 addr) {
    return rom->chr_rom[addr % rom->chr_rom_size];
}

void no_mapper_cpu_write(rom_t *rom, u8 val, u16 addr) {
    // not allowed
}

void no_mapper_ppu_write(rom_t *rom, u8 val, u16 addr) {
    // not allowed
}

void rom_free(rom_t *rom) {
    free(rom->prg_rom);
    free(rom->chr_rom);
}
