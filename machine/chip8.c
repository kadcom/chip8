#include "chip8.h"
#include "chip8_errors.h"
#include "chip8_internal.h"

#include <memory.h>
#include <string.h>

static u8 fonts[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

int init_machine(struct machine_t *m) {
  memset(m, 0, sizeof(struct machine_t));
  
  m->cpu.PC = 0x200;
  memcpy(m->memory, fonts, sizeof(fonts));
  return chip8_success;
}

int load_machine(struct machine_t *m, void *program, size_t program_length) {
  if (program_length == 0) {
    return chip8_err_invalid_program;
  }

  init_machine(m);
  memcpy(m->memory, program, program_length);

  return chip8_success;
}

static int decode(inst_t instruction, struct inst_field_t *field) {
  memcpy(field, &instruction, sizeof(inst_t));

  return chip8_success;
}

static int execute_field(struct machine_t *m, struct inst_field_t field, chip8_callback_t cb) {
  int ret = cb(m, field);
  m->cpu.PC += 2;

  return ret;
}

/* From here onwards it's the interpreter code */
extern draw_cb_t draw_cb_fn;

static int clear_screen(struct machine_t *m, UNUSED struct inst_field_t f) {
  UNUSED_PARAM(f);

  memset(m->display, 0, 32 * sizeof(u64));
  return chip8_success;
};

static int jump(struct machine_t *m, struct inst_field_t f) {
  m->cpu.PC = f.nnn;
  return chip8_success;
};

static int set_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.V[f.x] = f.kk;
  return chip8_success;
}

static int add_to_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.V[f.x] += f.kk;
  return chip8_success;
}

static int set_index_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.I = f.nnn;
  return chip8_success;
}

static int draw(struct machine_t *m, struct inst_field_t f) {
#define sprite_line_count 15
  u8 sprite[sprite_line_count] = {0}; /* 40 byte sprite */
  u32 i; 
  u64 old_line, line, sprite_line; //scanline
  u8 x, y, ny;

  if (f.suffix > sizeof(sprite)) {
    return chip8_err_invalid_instruction;
  }
  memcpy(sprite, &m->memory[m->cpu.I], f.suffix);

  x = m->cpu.V[f.x];
  y = m->cpu.V[f.y];

  for (i = 0; i < sprite_line_count; ++i) {
    sprite_line = ROTR64(sprite[i], x); // wrap x
    
    ny = (y + i) % 32; // wrap y

    old_line = m->display[ny];
    line = old_line ^ sprite_line;
    
    m->display[y] = line;

    // check if any bit in line flipped from 1 to 0
    m->cpu.VF = ((line & old_line) == old_line);
  }
  return chip8_success;
};


