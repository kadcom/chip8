#ifndef CHIP8_TYPES_H
#define CHIP8_TYPES_H

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

  #if defined (_MSC_VER)

    #if (_MSC_VER <= 1200)
      #define _U64(x) x##ui64
      #define INLINE_U64_MACRO (_inline u64)
      #define INLINE_U32_MACRO (_inline u32)
      #define INLINE_U16_MACRO (_inline u16)
    #else
      #define INLINE_U64_MACRO (u64)
      #define INLINE_U32_MACRO (u32)
      #define INLINE_U16_MACRO (u16)
      #define _U64(x) x##ull
    #endif // _MSC_VER < 1200

    #if (_MSC_VER <= 1500)

      #ifndef inline 
        #define inline __inline
      #endif 

      #ifndef snprintf 
        #define snprintf _snprintf 
      #endif

    #endif // _MSC_VER < 1400
  #endif // _MSC_VER

#else 
  #include <stdint.h>
  typedef uint8_t u8;
  typedef uint16_t u16;
  typedef uint32_t u32;
  typedef uint64_t u64;

  #define INLINE_U32_MACRO
  #define INLINE_U16_MACRO
  #define INLINE_U64_MACRO
  #define _U64(x) x##ull
#endif


#include <stdlib.h>

#if defined(__GNUC__) || defined(__clang__)
  #define UNUSED __attribute__((unused))
  #define UNUSED_PARAM(p)
  #define CHIP8_CALLBACK
  #define PACKED __attribute__((packed))
#endif

#define FLIP_ENDIANNESS_32(value) \
  (((value) & 0x000000FFul) << 24) | \
  (((value) & 0x0000FF00ul) << 8)  | \
  (((value) & 0x00FF0000ul) >> 8)  | \
  (((value) & 0xFF000000ul) >> 24)

#define FLIP_ENDIANNESS_16(value) \
  (((value) & 0x00FF) << 8) | \
  (((value) & 0xFF00) >> 8)

#define FLIP_ENDIANNESS_64(value) \
  INLINE_U64_MACRO \
  (((value) & _U64(0x00000000000000FF)) << 56) | \
  (((value) & _U64(0x000000000000FF00)) << 40) | \
  (((value) & _U64(0x0000000000FF0000)) << 24) | \
  (((value) & _U64(0x00000000FF000000)) << 8)  | \
  (((value) & _U64(0x000000FF00000000)) >> 8)  | \
  (((value) & _U64(0x0000FF0000000000)) >> 24) | \
  (((value) & _U64(0x00FF000000000000)) >> 40) | \
  (((value) & _U64(0xFF00000000000000)) >> 56)


#define FLIP_ENDIANNESS_8(value) (FLIP_ENDIANNESS_32((value)) & 0xFF)

#define DIR_LEFT  1 
#define DIR_RIGHT 0

#define ROT64(v,n,d) ((d) ? (((v) << (n)) | ((v) >> (64-(n)))) : (((v) >> (n)) | ((v) << (64-(n)))))

#define ROTR64(v,n) ROT64((v), (n), DIR_RIGHT)
#define ROTL64(v,n) ROT64((v), (n), DIR_LEFT)

#endif /* CHIP8_TYPES_H */

