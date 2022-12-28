#include <windows.h>
#include <commctrl.h>
#include "debug_view.h"
#include "chip8_errors.h"
#include "resource.h"
#include "wnd_messages.h"

static LRESULT CALLBACK dbg_view_proc(HWND, UINT, WPARAM, LPARAM);
static char dbg_view_class[] = "DebugViewWindowClass";

static int debug_view_shown = 0;

#define debug_view_width  600 
#define debug_view_height 600

static void show_disassembler(struct debug_view_t *dbgview);
static HWND create_disasm_view(struct debug_view_t *dbgview);

int init_debug_view(struct debug_view_t *dbgview, HWND main_window, HINSTANCE instance, struct machine_t *m) {
  WNDCLASSEX wcex;
  HWND wnd;
  int err;
  RECT main_window_rect;

  if (debug_view_shown) {
    return chip8_err_unknown;
  }

  ZeroMemory(dbgview, sizeof(struct debug_view_t));
  ZeroMemory(&wcex, sizeof(WNDCLASSEX));

  wcex.cbSize      = sizeof(WNDCLASSEX);

  if (GetClassInfoEx(NULL, dbg_view_class, &wcex) != 0) {
    goto registered;
  }

  wcex.cbSize      = sizeof(WNDCLASSEX);
  wcex.style       = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = dbg_view_proc;
  wcex.lpszClassName = dbg_view_class;
  wcex.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
  wcex.hInstance   = instance;
  wcex.hCursor     = LoadCursor(instance, IDC_ARROW);

  if (!RegisterClassEx(&wcex)) {
    err = GetLastError();

    if (ERROR_CLASS_ALREADY_EXISTS != err) {
      return chip8_err_unknown;
    }
  }
registered:

  wnd = CreateWindowEx(
      0,
      dbg_view_class,
      "Chip8 Debugger",
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      debug_view_width,
      debug_view_height,
      NULL,
      NULL, 
      instance,
      NULL);

  if (!wnd) {
    UnregisterClass(dbg_view_class, instance);
    return chip8_err_unknown;
  };

  SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG) dbgview);

  /* Move Window to the next of main window */ 
  GetWindowRect(main_window, &main_window_rect);
  MoveWindow(wnd, 
      main_window_rect.right + 5,
      main_window_rect.top,
      debug_view_width,
      debug_view_height,
      TRUE);


  ShowWindow(wnd, SW_SHOW);
  UpdateWindow(wnd);

  dbgview->main_window = main_window;
  dbgview->wnd = wnd;
  dbgview->m = m;

  if (!debug_view_shown) {
    debug_view_shown = TRUE;
  }


  create_disasm_view(dbgview);

  PostMessage(main_window, CHIP8_DEBUG_WINDOW_OPENED, 0, 0);

  return chip8_success;
};

int destroy_debug_view(struct debug_view_t *dbgview) {
  HWND wnd = dbgview->wnd;

  if (!debug_view_shown) {
    return chip8_err_unknown;
  }

  ZeroMemory(dbgview, sizeof(struct debug_view_t));
  DestroyWindow(wnd);

  return chip8_success;
};

static LRESULT CALLBACK dbg_view_proc(HWND wnd, UINT message, WPARAM wParam, LPARAM lParam) {
  struct debug_view_t *dbgview = (struct debug_view_t *)GetWindowLongPtr(wnd, GWLP_USERDATA);
  switch (message) {
    case WM_DESTROY:
      debug_view_shown = FALSE;
      PostMessage(dbgview->main_window, CHIP8_DEBUG_WINDOW_CLOSED, 0, 0);
      break;
    default:
      return DefWindowProc(wnd, message, wParam, lParam);
  }
  return FALSE;
};


#ifndef LVS_MULTIPLESEL 
#define LVS_MULTIPLESEL 0x00000200
#endif 

static HWND create_disasm_view(struct debug_view_t *dbgview) {
  HWND disWnd = NULL;
  RECT rect;
  DWORD bkgd = GetSysColor(COLOR_3DFACE);

  int width, height;

  LVCOLUMN addr_col = {0};
  LVITEM   item = {0};
  HFONT hFont = CreateFont(10, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
      CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH | FF_MODERN, "FixedSys");

  GetClientRect(dbgview->wnd, &rect);
  InflateRect(&rect, -5, -5);

  width = rect.right - rect.left;
  height = rect.bottom - rect.top;

  disWnd = CreateWindowEx( 
      WS_EX_CLIENTEDGE,
      WC_LISTVIEW,
      "Disassembler View",
      WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_MULTIPLESEL | LVS_SHOWSELALWAYS , 
      rect.left, rect.top,
      300, height,
      dbgview->wnd, 
      NULL, NULL, NULL);

  if (disWnd == NULL) {
    return NULL;
  }

  SendMessage(disWnd, LVM_SETBKCOLOR, 0, (LPARAM) bkgd); 

  addr_col.mask    = LVCF_TEXT | LVCF_WIDTH;
  addr_col.pszText = "Address";
  addr_col.cx      = 100;

  SendMessage(disWnd, LVM_INSERTCOLUMN, 0, (LPARAM) &addr_col);
  addr_col.mask    = LVCF_TEXT | LVCF_WIDTH;
  addr_col.pszText = "Instructions";
  addr_col.cx      = 130; 

  SendMessage(disWnd, LVM_INSERTCOLUMN, 1, (LPARAM) &addr_col);


  item.mask = LVIF_TEXT;
  item.iItem = 0;
  item.pszText = "0x200";
  SendMessage(disWnd, LVM_INSERTITEM, 0, (LPARAM)&item);

  // Set the text of the second column
  item.iSubItem = 1;
  item.pszText = "CLS";
  SendMessage(disWnd, LVM_SETITEM, 0, (LPARAM)&item);


  return disWnd;
}