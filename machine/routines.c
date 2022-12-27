#include <memory.h>
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

int CHIP8_CALLBACK draw(struct machine_t *m, struct inst_field_t f) {
#define sprite_line_count 15
  u8 sprite[sprite_line_count] = {0}; /* 40 byte sprite */
  u32 i; 
  u64 old_line, line, sprite_line; //scanline
  u8 x, y, ny;

  if (f.n > sizeof(sprite)) {
    return chip8_err_invalid_instruction;
  }
  memcpy(sprite, &m->memory[m->cpu.I], f.n);

  x = m->cpu.V[f.x];
  y = m->cpu.V[f.y];

  for (i = 0; i < sprite_line_count; ++i) {
    sprite_line = ROTR64((u64)sprite[i], x); // wrap x
    
    ny = (y + (u8)i) % 32; // wrap y

    old_line = m->display[ny];
    line = old_line ^ sprite_line;
    
    m->display[y] = line;

    // check if any bit in line flipped from 1 to 0
    m->cpu.VF = ((old_line & line) == line)  && ((sprite_line & line) == 0);
  }
  return chip8_success;
};

int CHIP8_CALLBACK unimplemented(UNUSED struct machine_t *m, UNUSED struct inst_field_t f) {
  UNUSED_PARAM(m);
  UNUSED_PARAM(f);

  return chip8_err_unimplemented;
};

