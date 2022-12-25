#ifndef CHIP8_TYPES_H
#define CHIP8_TYPES_H

#if defined(_MSC_VER) && defined(WIN32)
#include <windows.h>
typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32; 
typedef unsigned __int64 u64;
#else 
#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

#include <stdlib.h>

#endif /* CHIP8_TYPES_H */

