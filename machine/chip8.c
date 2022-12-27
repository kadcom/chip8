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
  u16 inst = FLIP_ENDIANNESS_16(instruction);

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
  m->cpu.PC += 2;

  return cb(m, field);
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
