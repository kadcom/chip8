#ifndef CHIP8_INTERNAL_H 
#define CHIP8_INTERNAL_H

#include "chip8.h"
#include "common.h"

struct inst_field_t {
  u8 prefix:4;

  union {
    u16 nnn:12;
    struct {
      u8 x:4;
      union {
        u8 kk:8;
        struct {
          u8 y:4;
          u8 suffix: 4;
        };
      };
    };
  };
};

#define ROTR64(v,n) ((v) >> (n) | (v) << (64 - (n)))

typedef int (*chip8_callback_t)(struct machine_t*, struct inst_field_t);
extern draw_cb_t draw_cb_fn;
#endif
