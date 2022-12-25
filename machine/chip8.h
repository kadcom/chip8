#ifndef CHIP8_CHIP8_H
#define CHIP8_CHIP8_H

#include "common.h"

struct cpu_t {
  /* 
   * General Purpose Register chip 8
   * Pakai union supaya bisa diakses dengan indeks maupun nama 
   * Nilai V[0] sama dengan V0, V[15] sama dengan VF 
   */
  union {
    u8 V[16];
    struct {
      u8 V0, V1, V2, V3,
         V4, V5, V6, V7,
         V8, V9, VA, VB, 
         VC, VD, VE, VF;
    };
  };

  /* Register I
   * Untuk menyimpan alamat memori 
   */
  u16     I;

  /* Register DT dan ST 
   * Menyimpan timer
   */
  u8 DT, ST;

  /* Register PC adalah program counter 
   * Register SP adalah stack pointer 
   */
  u16    PC;
  u8     SP;

};

struct machine_t {
  struct cpu_t cpu; 
  u8           memory[4096]; // 4K of RAM 
  u16          stack[16];    // 256 bytes of stack 
  u64          display[32];  // 32 x 64 bit lines.
};

/* Instruction type
 * biar lebih mudah tau drpd u16
 */
typedef u16 inst_t;

#include "chip8_errors.h"

int init_machine(struct machine_t *m);
int load_machine(struct machine_t *m, void *program, size_t program_length);
#endif
