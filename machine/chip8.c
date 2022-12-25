#include "chip8.h"
#include "chip8_errors.h"
#include "chip8_internal.h"

#include <memory.h>
#include <string.h>
#include <stdio.h>

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

static u16 program_base_addr = 0x200;

static inline u16 flip_endian(u16 x) {
  return (x >> 8) | (x << 8);
}

int init_machine(struct machine_t *m) {
  memset(m, 0, sizeof(struct machine_t));
  
  m->cpu.PC = program_base_addr;
  memcpy(m->memory, fonts, sizeof(fonts));
  return chip8_success;
}

int load_machine(struct machine_t *m, void *program, size_t program_length) {
  if (program_length == 0) {
    return chip8_err_invalid_program;
  }

  init_machine(m);
  memcpy(m->memory + program_base_addr, program, program_length);

  return chip8_success;
}

static int fetch(struct machine_t *m, inst_t *instruction) {
  *instruction = *(inst_t*) (m->memory + m->cpu.PC);
  return chip8_success;
};

static int decode(inst_t instruction, struct inst_field_t *field) {

  // flip the endian for little endian
  u16 inst = flip_endian(instruction);

  field->inst = inst;

  field->prefix = (inst >> 12) & 0x0F;
  field->x      = (inst >> 8) & 0x0F;
  field->y      = (inst >> 4) & 0x0F;
  field->n      = inst & 0x000F;
  field->nnn    = inst & 0x0FFF;
  field->kk     = inst & 0x00FF;

  return chip8_success;
}

static int execute_field(struct machine_t *m, struct inst_field_t field, chip8_callback_t cb) {
  int ret = cb(m, field);
  m->cpu.PC += 2;

  return ret;
}

static int disasm_inst(inst_t instruction, char *buf, size_t buf_len) {
  struct inst_field_t df; 
  decode(instruction, &df);

  switch (df.prefix) {
    case 0x0:
      switch (df.lo) {
        case 0xE0:
          strncpy(buf, "cls", buf_len); 
          break; 
        case 0xEE:
          strncpy(buf, "ret", buf_len);
          break;
        default:
          return chip8_err_invalid_instruction;
      };
      break; 
    case 0x1:
      snprintf(buf, buf_len, "jmp\t0x%04x", df.nnn);
      break;
    case 0x6: 
      snprintf(buf, buf_len, "ld\tV%X, 0x%02x", df.x, df.kk);
      break; 
    case 0x7:
      snprintf(buf, buf_len, "add\tV%X, 0x%02x", df.x, df.kk);
      break;
    case 0xA:
      snprintf(buf, buf_len, "ld\tI, 0x%04x", df.nnn);
      break;
    case 0xD:
      snprintf(buf, buf_len, "drw\tV%X, V%X, 0x%x", df.x, df.y, df.n);
      break;
    default:
      return chip8_err_invalid_instruction;
  };
  return chip8_success;
};

int disasm_addr(struct machine_t *m, u16 addr, char *buf, size_t buf_len) {
  inst_t inst = *(inst_t*) (m->memory + addr);
  return disasm_inst(inst, buf, buf_len);
}

int disasm_pc(struct machine_t *m, char *buf, size_t buf_len) {
  inst_t inst;
  fetch(m, &inst);
  return disasm_inst(inst, buf, buf_len);
};

/* From here onwards it's the interpreter code */
extern draw_cb_t draw_cb_fn;

static int CHIP8_CALLBACK clear_screen(struct machine_t *m, UNUSED struct inst_field_t f) {
  UNUSED_PARAM(f);

  memset(m->display, 0, 32 * sizeof(u64));
  return chip8_success;
};

static int CHIP8_CALLBACK jump(struct machine_t *m, struct inst_field_t f) {
  m->cpu.PC = f.nnn;
  return chip8_success;
};

static int CHIP8_CALLBACK set_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.V[f.x] = f.kk;
  return chip8_success;
}

static int CHIP8_CALLBACK add_to_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.V[f.x] += f.kk;
  return chip8_success;
}

static int CHIP8_CALLBACK set_index_register(struct machine_t *m, struct inst_field_t f) {
  m->cpu.I = f.nnn;
  return chip8_success;
}

static int CHIP8_CALLBACK draw(struct machine_t *m, struct inst_field_t f) {
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
    sprite_line = ROTR64(sprite[i], x); // wrap x
    
    ny = (y + (u8)i) % 32; // wrap y

    old_line = m->display[ny];
    line = old_line ^ sprite_line;
    
    m->display[y] = line;

    // check if any bit in line flipped from 1 to 0
    m->cpu.VF = ((line & old_line) == old_line) & 0x1;
  }
  return chip8_success;
};

static int CHIP8_CALLBACK unimplemented(UNUSED struct machine_t *m, UNUSED struct inst_field_t f) {
  UNUSED_PARAM(m);
  UNUSED_PARAM(f);

  return chip8_err_unimplemented;
};

static chip8_callback_t callbacks[] = 
{
  // 0x0          0x1            0x2           0x3 
  (void*) -1   , jump         , unimplemented, unimplemented, 
  // 0x4          0x5            0x6           0x7 
  unimplemented, unimplemented,  set_register, add_to_register, 
  // 0x8          0x9            0xA           0xB
  unimplemented, unimplemented, set_index_register, unimplemented, 
  // 0xC          0xD            0xE           0xF
  unimplemented, draw,          unimplemented, unimplemented,
};

int fetch_and_execute(struct machine_t *m) {
  inst_t inst;
  struct inst_field_t df;
	chip8_callback_t cb;

  fetch(m, &inst);
  decode(inst, &df);

  if (df.prefix == 0x0) {
      switch (df.lo) {
        case 0xE0:
          cb = clear_screen;
          goto end;
          break; 
        case 0xEE:
          return chip8_err_unimplemented;
          break;
        default:
          return chip8_err_invalid_instruction;
    };
  };

  cb = callbacks[df.prefix]; 
end:
  execute_field(m, df, cb);
  return chip8_success;
}
