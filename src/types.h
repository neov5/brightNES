#ifndef __TYPES_H__
#define __TYPES_H__

#include <inttypes.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t s8;
typedef int16_t s16;
typedef uint32_t u32;
typedef int32_t i32;

#define KB 0x400 // 1 kB

u8 force_u8(void* ptr) {
    return *(u8*)(ptr);
}

#endif
