#ifndef CHIP8_ROUTINES_H
#define CHIP8_ROUTINES_H 
#include "chip8.h"
#include "chip8_internal.h"
#include "common.h"

int CHIP8_CALLBACK clear_screen(struct machine_t *m, struct inst_field_t f);
int CHIP8_CALLBACK jump(struct machine_t *m, struct inst_field_t f);
int CHIP8_CALLBACK set_register(struct machine_t *m, struct inst_field_t f);
int CHIP8_CALLBACK add_to_register(struct machine_t *m, struct inst_field_t f);
int CHIP8_CALLBACK set_index_register(struct machine_t *m, struct inst_field_t f);
int CHIP8_CALLBACK draw(struct machine_t *m, struct inst_field_t f);
int CHIP8_CALLBACK unimplemented(UNUSED struct machine_t *m, UNUSED struct inst_field_t f);

#endif
