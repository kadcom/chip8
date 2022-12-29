#include "disasm_view.h"
#include "chip8_errors.h"

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

int create_disasm_view(struct chip8_disasm_view_t *dv_data, HINSTANCE instance, HWND parent,
    int x, int y, int width, int height) {

  WNDCLASSEX wcex;
  COLORREF bk_color = RGB(255, 255, 224);
  HBRUSH   bk_brush = CreateSolidBrush(bk_color);
  HWND wnd = NULL;
  ATOM ret = FALSE;


  if (dv_data != NULL) {
    ZeroMemory(dv_data, sizeof(struct chip8_disasm_view_t));
  }

  wcex.cbSize  = sizeof(WNDCLASSEX);
  wcex.style   = CS_VREDRAW | CS_VREDRAW;
  wcex.hCursor = LoadCursor(instance, IDC_ARROW);
  wcex.hbrBackground = bk_brush;
  wcex.lpfnWndProc = disasm_view_proc;
  wcex.hInstance = instance;

  ret = RegisterClassEx(&wcex);

  if (!ret && ret != ERROR_CLASS_ALREADY_EXISTS) {
    return chip8_err_unknown;
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
  dv_data->asm_font = create_monospace_font(wnd);
  dv_data->char_width = get_char_width(wnd, dv_data->asm_font);
  dv_data->x = x;
  dv_data->y = y; 
  dv_data->width = width;
  dv_data->height = height;

  SetWindowLongPtr(wnd, GWL_USERDATA, (LONG_PTR) dv_data);
  return chip8_success;
};

int destroy_disasm_view(struct chip8_disasm_view_t *dv) {
  DeleteObject(dv->asm_font);
  DeleteObject(dv->asm_bk_color);
  DestroyWindow(dv->view);

  ZeroMemory(dv, sizeof(struct chip8_disasm_view_t));

  return chip8_success;
};

static LRESULT CALLBACK disasm_view_proc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) {
  return DefWindowProcW(wnd, message, wParam, lParam); 
};
