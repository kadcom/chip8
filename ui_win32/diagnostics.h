#ifndef DIAG_H
#define DIAG_H 
#include "chip8.h"

static void diag_checkered(struct machine_t *m) {
  int i;

  for (i = 0; i < 32; ++i) {
    m->display[i] = _U64(0xFF00FF00FF00FF00) >> (8 * ( i % 2 ));
  }
};

static void diag_clear(struct machine_t *m) {
  int i; 

  for (i = 0; i < 32; ++i) {
    m->display[i] = _U64(0x0);
  }
}

#endif
