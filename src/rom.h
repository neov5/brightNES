#ifndef __ROM_H__ 
#define __ROM_H__ 

#include "types.h"

typedef enum {
    NONE = 0,
    MMC1 = 1,
    UXROM = 2,
    CNROM = 3,
    MMC3 = 4
} rom_mapper_t;

typedef struct {
    rom_mapper_t mapper;
    u8 *prg_rom;
    u8 *chr_rom;
} rom_t;

rom_t* rom_load_from_file(char* filename);
void rom_free(rom_t* rom);

#endif 
