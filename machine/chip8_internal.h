#ifndef CHIP8_INTERNAL_H 
#define CHIP8_INTERNAL_H

#include "chip8.h"
#include "common.h"

#ifdef _MSC_VER
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

#ifdef _MSC_VER
#pragma pack(pop)
#endif

#define ROTR64(v,n) ((v) >> (n) | (v) << (64 - (n)))

typedef int (CHIP8_CALLBACK *chip8_callback_t)(struct machine_t*, struct inst_field_t);
extern draw_cb_t draw_cb_fn;
#endif
