#ifndef CHIP8_ROUTINES_H
#define CHIP8_ROUTINES_H 
#include "chip8.h"
#include "chip8_internal.h"
#include "common.h"

#define C8_ROUTINE(fn_name) int CHIP8_CALLBACK fn_name (struct machine_t *m, struct inst_field_t f)

C8_ROUTINE(clear_screen);
C8_ROUTINE(jump);
C8_ROUTINE(set_register);
C8_ROUTINE(add_to_register);
C8_ROUTINE(set_index_register);
C8_ROUTINE(draw); 
C8_ROUTINE(unimplemented);
C8_ROUTINE(ret);
C8_ROUTINE(call);
C8_ROUTINE(skip_equal);
C8_ROUTINE(skip_neq);

#endif
