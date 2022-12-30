#ifndef CHIP8_DISASM_VIEW
#define CHIP8_DISASM_VIEW

#include <windows.h>
#include "chip8.h"

enum {
  C8_DISV_SELECT_LINE = WM_USER + 100,
};

#define DEFAULT_DISASM_WIDTH  200
#define DEFAULT_DISASM_HEIGHT 300

struct chip8_disasm_view_t {
  HWND view;
  HWND parent;
  HINSTANCE instance;

  HFONT  asm_font;
  HBRUSH asm_bk_color;
  HPEN black_pen;

  int x, y, width, height;

  int char_width;
  int char_height;

  struct machine_t *m;
};

#define CHIP8_DISASM_VIEW_CLASS "Chip8DisasmViewClass"

int create_disasm_view(struct chip8_disasm_view_t *dv_data, 
    HINSTANCE instance, HWND parent, int x, int y, int width, int height, 
    struct machine_t *);

int destroy_disasm_view(struct chip8_disasm_view_t *dv);
#endif /* CHIP8_DISASM_VIEW */

