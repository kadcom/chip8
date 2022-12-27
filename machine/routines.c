#include <memory.h>
#include "common.h"
#include "routines.h"

int CHIP8_CALLBACK clear_screen(struct machine_t *m, UNUSED struct inst_field_t f) {
  UNUSED_PARAM(f);

  memset(m->display, 0, 32 * sizeof(u64));
  return chip8_success;
};

int CHIP8_CALLBACK jump(struct machine_t *m, struct inst_field_t f) {
  m->cpu.PC = f.nnn;
  return chip8_success;
};

int CHIP8_CALLBACK set_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.V[f.x] = f.kk;
  return chip8_success;
}

int CHIP8_CALLBACK add_to_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.V[f.x] += f.kk;
  return chip8_success;
}

int CHIP8_CALLBACK set_index_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.I = f.nnn;
  return chip8_success;
}

static inline u8 extract_tgt_sprite(u64 fb_line, u8 x) {
  u8   pos = x % 64;
  u8   n   = 64 - (pos + 8);
  u64  rotated = ROTL64(fb_line, n);
 
  return (u8)(rotated & 0xFF);
}

static inline u64 replace_tgt_line(u64 tgt_line, u8 sp_line, u8 x) {
  u8  pos = x % 64; 

  u64 tgt_sp_line = ROTR64((u64) sp_line, pos);
  u64 tgt_mask    = ROTR64((((u64) 1 << 8) - 1) << pos, pos);

  tgt_line &= tgt_mask;
  tgt_line |= tgt_sp_line;

  return tgt_line; 
}

#define max_sprite_line_count 15

int CHIP8_CALLBACK draw(struct machine_t *m, struct inst_field_t f) {
  u8 sprite[max_sprite_line_count] = {0}; /* 40 byte sprite */
  u8 sprite_tgt, sprite_line, target = 0;
  u32 i, sprite_size; 
  u64 current_line, line; //scanline
  u8 x, y, ny, flip0;

  if (f.n > sizeof(sprite)) {
    return chip8_err_invalid_instruction;
  }

  memcpy(sprite, &m->memory[m->cpu.I], f.n);

  x = m->cpu.V[f.x];
  y = m->cpu.V[f.y];

  for (i = 0; i < max_sprite_line_count; ++i) {
    // wrap on target if any of it > 32 
    ny = (y + (u8)i) % 32;

    // wrap x if necessary
    x = x % 64;

    // get current display scanline 
    current_line = m->display[ny];

    // get current sprite line 
    sprite_line  = sprite[i];

    // get current target sprite line 
    sprite_tgt   = extract_tgt_sprite(current_line, x);

    // apply xor to create final sprite 
    target       = sprite_line ^ sprite_tgt;

    // check if any bit is flipped to zero 
    flip0        = (sprite_tgt & target) != target;

    // replace the bit with the target sprite, wrap if necessary
    line = replace_tgt_line(current_line, target, x);

    // show in display
    m->display[ny] = line;

  }
  return chip8_success;
};

int CHIP8_CALLBACK unimplemented(UNUSED struct machine_t *m, UNUSED struct inst_field_t f) {
  UNUSED_PARAM(m);
  UNUSED_PARAM(f);

  return chip8_err_unimplemented;
};

