#ifndef CHIP8_TYPES_H
#define CHIP8_TYPES_H

/* compatibility with older msvc */
#if defined(_MSC_VER) && (_MSC_VER <= 1400)

#ifndef inline 
#define inline __inline
#endif 

#ifndef snprintf 
#define snprintf _snprintf 
#endif

#endif

#if (defined(_MSC_VER) || defined(__WATCOMC__)) && (defined(WIN32) || defined(_WIN32))
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

#define FLIP_ENDIANNESS_64(value) \
  (((value) & 0x00000000000000FFu) << 56) | \
  (((value) & 0x000000000000FF00u) << 40) | \
  (((value) & 0x0000000000FF0000u) << 24) | \
  (((value) & 0x00000000FF000000u) << 8)  | \
  (((value) & 0x000000FF00000000u) >> 8)  | \
  (((value) & 0x0000FF0000000000u) >> 24) | \
  (((value) & 0x00FF000000000000u) >> 40) | \
  (((value) & 0xFF00000000000000u) >> 56)

#endif /* CHIP8_TYPES_H */

