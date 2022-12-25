#ifndef CHIP8_TYPES_H
#define CHIP8_TYPES_H

#if defined(_MSC_VER) && defined(WIN32)
#include <windows.h>
typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32; 
typedef unsigned __int64 u64;

#define UNUSED
#define UNUSED_PARAM(p) ((p))
#define PACKED
#define CHIP8_CALLBACK __stdcall
#else 
#include <stdint.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
#endif

#include <stdlib.h>

#if defined(__GNUC__) || defined(__clang__)
#define UNUSED __attribute__((unused))
#define UNUSED_PARAM(p)
#define CHIP8_CALLBACK
#define PACKED __attribute__((packed))
#endif

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#endif /* CHIP8_TYPES_H */

