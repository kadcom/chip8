#ifndef CHIP8_DEBUGGER
#define CHIP8_DEBUGGER

#include <windows.h>

struct machine_t;

struct debug_view_t {
  HWND wnd;
  HWND main_window;

  HWND dis_window;

  struct machine_t *m;
};

int init_debug_view(struct debug_view_t *dbgview, HWND main_window, HINSTANCE instance, struct machine_t *m);
int destroy_debug_view(struct debug_view_t *dbgview);
#endif /* CHIP8_DEBUGGER */
