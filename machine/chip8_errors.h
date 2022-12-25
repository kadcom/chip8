#ifndef CHIP8_ERRORS_H
#define CHIP8_ERRORS_H

#define chip8_failed(x) ((x) != 0)

enum {
  chip8_success = 0,
  chip8_err_unknown = -1,
  chip8_err_invalid_instruction = -2,
  chip8_err_invalid_program = -3,
};

#endif
