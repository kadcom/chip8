#include "disasm_view.h"
#include "chip8.h"
#include "chip8_errors.h"
#include <stdio.h>

static LRESULT CALLBACK disasm_view_proc(HWND, UINT, WPARAM, LPARAM);

static HFONT create_monospace_font(HWND wnd) {
  HDC dc  = GetDC(wnd);
  int ppi = GetDeviceCaps(dc, LOGPIXELSY);
  int height = -MulDiv(10, ppi, 72); // 1 point = 1/72 inch 

  HFONT fnt = CreateFont(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_MODERN | FIXED_PITCH, "Courier");

  ReleaseDC(wnd, dc);

  return fnt;
}

static int get_char_width(HWND wnd, HFONT fnt) {
  HDC dc = GetDC(wnd);
  HFONT old_font = SelectObject(dc, fnt); 
  TEXTMETRIC tm; 

  GetTextMetrics(dc, &tm);
  SelectObject(dc, old_font);
  ReleaseDC(wnd, dc);

  return tm.tmAveCharWidth;
}

static int get_char_height(HWND wnd, HFONT fnt) {
  HDC dc = GetDC(wnd);
  HFONT old_font = SelectObject(dc, fnt); 
  TEXTMETRIC tm; 

  GetTextMetrics(dc, &tm);
  SelectObject(dc, old_font);
  ReleaseDC(wnd, dc);

  return tm.tmHeight;
}

int create_disasm_view(struct chip8_disasm_view_t *dv_data, HINSTANCE instance, HWND parent,
    int x, int y, int width, int height, struct machine_t *m) {

  WNDCLASSEX wcex;
  COLORREF bk_color = RGB(255, 255, 224);
  //COLORREF bk_color = RGB(0, 0, 0);
  HBRUSH   bk_brush = CreateSolidBrush(bk_color);
  HPEN black_pen = CreatePen(PS_SOLID, 1, RGB(70,70,70));
  HWND wnd = NULL;
  ATOM ret = FALSE;
  DWORD err = 0;


  if (dv_data != NULL) {
    ZeroMemory(dv_data, sizeof(struct chip8_disasm_view_t));
  }

  ZeroMemory(&wcex, sizeof(WNDCLASSEX));

  wcex.cbSize  = sizeof(WNDCLASSEX);
  wcex.style   = CS_VREDRAW | CS_VREDRAW;
  wcex.hCursor = LoadCursor(instance, IDC_ARROW);
  wcex.hbrBackground = bk_brush;
  wcex.lpfnWndProc = disasm_view_proc;
  wcex.hInstance = instance;
  wcex.lpszClassName = CHIP8_DISASM_VIEW_CLASS;

  ret = RegisterClassEx(&wcex);

  if (!ret) {
    err = GetLastError();

    if (err != ERROR_CLASS_ALREADY_EXISTS) {
      return chip8_err_unknown;
    }
  }

  wnd = CreateWindowEx(
      WS_EX_CLIENTEDGE,
      CHIP8_DISASM_VIEW_CLASS,
      "",
      WS_CHILD | WS_VISIBLE, 
      x, y, width, height,
      parent,
      NULL,
      instance,
      NULL);

  dv_data->view = wnd;
  dv_data->parent = parent;
  dv_data->asm_bk_color = bk_brush;
  dv_data->black_pen = black_pen;
  dv_data->asm_font = create_monospace_font(wnd);
  dv_data->char_width = get_char_width(wnd, dv_data->asm_font);
  dv_data->char_height = get_char_height(wnd, dv_data->asm_font);
  dv_data->x = x;
  dv_data->y = y; 
  dv_data->width = width;
  dv_data->height = height;
  dv_data->m = m; 

  SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR) dv_data);
  return chip8_success;
};

int destroy_disasm_view(struct chip8_disasm_view_t *dv) {
  DeleteObject(dv->asm_font);
  DeleteObject(dv->asm_bk_color);
  DestroyWindow(dv->view);

  ZeroMemory(dv, sizeof(struct chip8_disasm_view_t));

  return chip8_success;
};

static LRESULT paint_window(HWND wnd, struct chip8_disasm_view_t *dv_data);

static LRESULT CALLBACK disasm_view_proc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) {
  struct chip8_disasm_view_t *dv_data = (struct chip8_disasm_view_t*) GetWindowLongPtr(wnd, GWLP_USERDATA);
  switch(message) {
    case WM_PAINT: 
      return paint_window(wnd, dv_data);
    default:
      return DefWindowProcW(wnd, message, wParam, lParam); 
  }
  return FALSE;
};

static void paint_disasm(HWND wnd, HDC dc, struct chip8_disasm_view_t *dv_data);

LRESULT paint_window(HWND wnd, struct chip8_disasm_view_t *dv_data) {
  int margin = dv_data->char_width / 2;
  int first_column_width = dv_data->char_width * 4 + 3 * margin;
  int second_column_width = first_column_width + (dv_data->char_width * 4 + 2 * margin);
  int third_column_width = second_column_width + (dv_data->char_width * 18 + 2 * margin);
  HPEN old_pen;
  HFONT old_font; 
  PAINTSTRUCT ps;
  HDC dc;
  int res, err;
  char *sample_line = "0200 0E00 CLS";

  dc = BeginPaint(wnd, &ps);
  

  old_font = SelectObject(dc, dv_data->asm_font);

  SetBkColor(dc, RGB(255, 255, 224));

  paint_disasm(wnd, dc, dv_data);

  old_pen = SelectObject(dc, dv_data->black_pen);
  
  MoveToEx(dc, first_column_width, 0, NULL);
  LineTo(dc, first_column_width, ps.rcPaint.bottom);

  MoveToEx(dc, second_column_width, 0, NULL);
  LineTo(dc, second_column_width, ps.rcPaint.bottom);

  MoveToEx(dc, third_column_width, 0, NULL);
  LineTo(dc, third_column_width, ps.rcPaint.bottom);

  SelectObject(dc, old_pen);
  SelectObject(dc, old_font);
  
  EndPaint(wnd, &ps);


  return 0;
}

void paint_disasm(HWND wnd, HDC dc, struct chip8_disasm_view_t *dv_data) {
#define base_pc_addr 0x200
  struct machine_t *m = dv_data->m;
  char line_buf[256] = {0};
  char disas_buf[64] = {0};
  int i, ret, y, n, j;

  for (i = base_pc_addr, y = 0; i < 4096; i+=2, y+= dv_data->char_height) {
    ret = disasm_addr(m, i, disas_buf, sizeof(disas_buf));

    // convert tabs to space 
    for (j = 0; j < lstrlen(disas_buf); ++j) {
      if (disas_buf[j] == '\t') {
        disas_buf[j] = ' ';
      }
    }

    if (ret != chip8_success) {
      continue;
    }
    snprintf(line_buf, sizeof(line_buf) - 1,"%04X %04X %s", i, *(u16*)&m->memory[i], disas_buf);
    n = lstrlen(line_buf);
    TextOut(dc, dv_data->char_width, y, line_buf, n);
  }

}


