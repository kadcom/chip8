#ifndef CHIP8_ROUTINES_H
#define CHIP8_ROUTINES_H 
#include "chip8.h"
#include "common.h"

struct inst_field_t;

typedef void (CHIP8_CALLBACK *c8_sub_routine_t)(struct machine_t *, struct inst_field_t f);
typedef int  (CHIP8_CALLBACK *c8_routine_t)(struct machine_t *, struct inst_field_t f);

#define C8_ROUTINE(fn_name, prefix_or_opcode)\
  int CHIP8_CALLBACK fn_name (struct machine_t *m, struct inst_field_t f)
#define C8_SUB_ROUTINE(fn_name, prefix_or_op_code, suffix)\
  static void CHIP8_CALLBACK fn_name(struct machine_t *m, struct inst_field_t f)

C8_ROUTINE(clear_screen, 0x0E00);
C8_ROUTINE(ret,          0x00EE);
C8_ROUTINE(jump,         0x1);
C8_ROUTINE(call,         0x2);
C8_ROUTINE(skip_equal,   0x3);
C8_ROUTINE(skip_neq,     0x4);
C8_ROUTINE(skip_eq_regs, 0x5);
C8_ROUTINE(set_register, 0x6);
C8_ROUTINE(add_to_register, 0x7);

C8_ROUTINE(regs_ops, 0x8);
C8_SUB_ROUTINE(reg_ld, 0x8, 0x0);
C8_SUB_ROUTINE(reg_or, 0x8, 0x1);
C8_SUB_ROUTINE(reg_and, 0x8, 0x2);
C8_SUB_ROUTINE(reg_xor, 0x8, 0x3);
C8_SUB_ROUTINE(reg_add, 0x8, 0x4);
C8_SUB_ROUTINE(reg_sub, 0x8, 0x5);
C8_SUB_ROUTINE(reg_shr, 0x8, 0x6);
C8_SUB_ROUTINE(reg_subn, 0x8, 0x7);
C8_SUB_ROUTINE(reg_shl, 0x8, 0xE);

C8_ROUTINE(set_index_register, 0xA);
C8_ROUTINE(draw, 0xD); 
C8_ROUTINE(unimplemented, _);


#define _SRNONE ((void*)(-1))

static c8_routine_t c8_routines[0x10] = 
{
  // 0x0          0x1            0x2           0x3 
  _SRNONE     , jump         ,  call,        skip_equal, 
  // 0x4          0x5            0x6           0x7 
  skip_neq, unimplemented,  set_register, add_to_register, 
  // 0x8          0x9            0xA           0xB
  regs_ops, unimplemented, set_index_register, unimplemented, 
  // 0xC          0xD            0xE           0xF
  unimplemented, draw,          unimplemented, unimplemented,
};


#endif
