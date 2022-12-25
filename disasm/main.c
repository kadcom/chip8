#include <stdio.h>
#include <stdlib.h>

#include "chip8.h"
#include "chip8_internal.h"

int main(int argc, char **argv) {
  void *buf;
  char line[256] = {0};
  size_t linesz = sizeof(line);
  size_t bufsz;
  u16 addr;
  int res;

  if (argc < 2) {
    printf("Usage: %s [ROM file]\n", argv[0]);
    return -1;
  }


  struct machine_t machine;

  FILE *f = fopen(argv[1], "rb+");
  fseek(f, 0, SEEK_END);
  bufsz = ftell(f);
  fseek(f, 0, SEEK_SET);

  buf = malloc(bufsz);
  fread(buf, 1, bufsz, f);
  fclose(f);

  load_machine(&machine, buf, bufsz);

  free(buf); buf = 0;

  for (addr = 0x200; addr < 4096; addr += 2) {
    if (disasm_addr(&machine, addr, line, linesz) != chip8_success) {
      continue;
    }
    printf("0x%04x -- %s\n", addr, line);
  }

  return 0;
}
