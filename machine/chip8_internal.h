#ifndef CHIP8_INTERNAL_H 
#define CHIP8_INTERNAL_H

#include "chip8.h"
#include "common.h"

#if defined(_MSC_VER) || defined(__WATCOMC__)
#pragma pack(push,1)
#endif

struct PACKED inst_field_t {
  union {
    u16 inst;
    struct {
      u8 lo;
      u8 hi;
    };
  };

  u8  prefix;
  u16 nnn;

  u8  x;
  u8  kk;
  u8  y;
  u8  n;
};

#if defined(_MSC_VER) || defined(__WATCOMC__)
#pragma pack(pop)
#endif

#include "routines.h"
#endif
