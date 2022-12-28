#include <windows.h>
#include <commctrl.h>

#include "chip8.h"
#include "chip8_errors.h"
#include "render.h"
#include "resource.h"

#include "diagnostics.h"
#include "debug_view.h"

#define fb_width  64
#define fb_height 32

#define default_scale 10

static char window_class[] = "MainWindowClass";

static u8 programs_IBM_Logo_ch8[] = {
  0x00, 0xe0, 
  0xa2, 0x2a, 
  0x60, 0x0c, 
  0x61, 0x08, 
  0xd0, 0x1f, 
  0x70, 0x09,
  0xa2, 0x39, 
  0xd0, 0x1f, 
  0xa2, 0x48, 
  0x70, 0x08, 
  0xd0, 0x1f, 
  0x70, 0x04,
  0xa2, 0x57, 
  0xd0, 0x1f, 
  0x70, 0x08, 
  0xa2, 0x66, 
  0xd0, 0x1f, 
  0x70, 0x08,
  0xa2, 0x75, 
  0xd0, 0x1f, 
  0x12, 0x28, 

  0xff, 0x00, 0xff, 0x00, 0x3c, 0x00,
  0x3c, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0xff, 0x00, 0xff, 0xff, 0x00, 0xff,
  0x00, 0x38, 0x00, 0x3f, 0x00, 0x3f, 0x00, 0x38, 0x00, 0xff, 0x00, 0xff,
  0x80, 0x00, 0xe0, 0x00, 0xe0, 0x00, 0x80, 0x00, 0x80, 0x00, 0xe0, 0x00,
  0xe0, 0x00, 0x80, 0xf8, 0x00, 0xfc, 0x00, 0x3e, 0x00, 0x3f, 0x00, 0x3b,
  0x00, 0x39, 0x00, 0xf8, 0x00, 0xf8, 0x03, 0x00, 0x07, 0x00, 0x0f, 0x00,
  0xbf, 0x00, 0xfb, 0x00, 0xf3, 0x00, 0xe3, 0x00, 0x43, 0xe0, 0x00, 0xe0,
  0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xe0, 0x00, 0xe0
};
static size_t programs_IBM_Logo_ch8_len = 132;

static LRESULT CALLBACK main_window_proc(HWND, UINT, WPARAM, LPARAM);
static void attach_statusbar(HWND, HINSTANCE, int height);

struct app_data_t {
  struct debug_view_t debug_view;
  struct machine_t machine;
  struct render_t *renderer;

  HINSTANCE instance;
};

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmdline, int show_state) {
  WNDCLASSEX wcex; 
  MSG msg; 
  HWND main_window;
  BOOL res;
  int border, menu, status;
  struct app_data_t app_data;

  HBRUSH black_brush = (HBRUSH) GetStockObject(BLACK_BRUSH);
  HICON  app_icon = LoadIcon(instance, IDI_APPLICATION);
  HCURSOR arrow_cursor = LoadCursor(instance, IDC_ARROW);
  HMENU main_menu = LoadMenu(instance, MAKEINTRESOURCE(IDM_MAIN_MENU));

  LARGE_INTEGER start_time, freq, current_time;
  float elapsed_time = 0.0f;

  InitCommonControls();
  ZeroMemory(&wcex, sizeof(WNDCLASSEX));
  ZeroMemory(&msg, sizeof(MSG));
  ZeroMemory(&app_data, sizeof(struct app_data_t));

  app_data.instance = instance;

  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&start_time);

  init_machine(&app_data.machine);
  load_machine(&app_data.machine, programs_IBM_Logo_ch8, programs_IBM_Logo_ch8_len);

  border = GetSystemMetrics(SM_CXBORDER);
  menu = GetSystemMetrics(SM_CYMENU);
  status = 20;

  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style  = CS_VREDRAW | CS_HREDRAW;
  wcex.hInstance = instance; 
  wcex.hIcon = app_icon;
  wcex.hIconSm = app_icon;
  wcex.hCursor = arrow_cursor;
  wcex.hbrBackground = black_brush;
  wcex.lpszClassName = window_class;
  wcex.lpfnWndProc = main_window_proc;
  wcex.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN_MENU);

  res = RegisterClassEx(&wcex); 

  if (!res) {
    MessageBox(NULL, "Error Registering Window", "Error", MB_ICONHAND | MB_OK);
    return -1;
  }

  main_window = CreateWindowEx( 
      WS_EX_OVERLAPPEDWINDOW | WS_EX_CLIENTEDGE, 
      window_class,
      "Chip 8 Emulator",
      WS_OVERLAPPEDWINDOW & ~ (WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME),
      CW_USEDEFAULT, CW_USEDEFAULT,
      fb_width * default_scale + 2 * border, 
      fb_height * default_scale + border + menu + status,
      NULL,
      NULL,
      instance,
      NULL);

  if (0 == main_window) {
    MessageBox(NULL, "Error Creating Window", "Error", MB_ICONHAND | MB_OK);
    UnregisterClass(window_class, instance);
    return -1;
  }

  if (chip8_failed(create_renderer(&app_data.renderer))) {
    goto cleanup;
  }

  if (chip8_failed(initialise_renderer(app_data.renderer, main_window, fb_width * default_scale, fb_height * default_scale))) {
    goto cleanup;
  }

  ShowWindow(main_window, show_state);
  UpdateWindow(main_window);

  attach_statusbar(main_window, instance, status);
  init_debug_view(&app_data.debug_view, main_window, instance, &app_data.machine);

  SetWindowLongPtr(main_window, GWLP_USERDATA, (LONG) &app_data);

  for(;;) {
    if (PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE)) {
      if (msg.message == WM_QUIT) {
        destroy_debug_view(&app_data.debug_view);
        break;
      }

      if (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    } else {
      QueryPerformanceCounter(&current_time);
      elapsed_time += (float)(current_time.QuadPart - start_time.QuadPart) / (float)freq.QuadPart;

      start_time = current_time;

      /* TODO: Use the elapsed time as execution timing */
      /* TODO: accurate emulation with this timing: https://jackson-s.me/2019/07/13/Chip-8-Instruction-Scheduling-and-Frequency.html */
      fetch_and_execute(&app_data.machine);
      if (app_data.renderer) {
        render_display(app_data.renderer, &app_data.machine);
      }
    }
  }

cleanup:

  if (app_data.renderer) {
    destroy_renderer(app_data.renderer);
    app_data.renderer = NULL;
  }

  if (main_window) {
    DestroyWindow(main_window);
  }
  UnregisterClass(window_class, instance);

  return (int)msg.wParam;
}

static LRESULT on_close(HWND window) {
  struct app_data_t *ad = (struct app_data_t *) GetWindowLongPtr(window, GWLP_USERDATA);

  int res = MessageBox(NULL, "Are you sure?", "Confirmation", MB_YESNO | MB_ICONQUESTION);

  if (IDYES == res) {
    if (ad->renderer) {
      destroy_renderer(ad->renderer);
      ad->renderer = NULL; 
    };
    DestroyWindow(window);
  }
  return FALSE;
}

static LRESULT on_paint(HWND window) {
  struct app_data_t *ad = (struct app_data_t *) GetWindowLongPtr(window, GWLP_USERDATA);

  if (!ad->renderer) {
    return FALSE;
  }

  if (chip8_failed(render_display(ad->renderer, &ad->machine))) {
    // ** do nothing **/
  }

  return FALSE;
}

static LRESULT on_debugger(HWND window) {
  struct app_data_t *ad = (struct app_data_t *) GetWindowLongPtr(window, GWLP_USERDATA);

  if (ad == NULL) {
    return FALSE;
  }

  init_debug_view(&ad->debug_view, window, ad->instance, &ad->machine);

  return FALSE;
}

static LRESULT CALLBACK main_window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  struct app_data_t *ad = (struct app_data_t *) GetWindowLongPtr(window, GWLP_USERDATA);
  switch (message)
  {
    case WM_COMMAND: 
      switch (LOWORD(wParam)) {
        case ID_DIAGNOSTICS_CHECKEREDPATTERN:
          diag_checkered(&ad->machine);
          break;
        case ID_DIAGNOSTICS_FRAMEBUFFER_CLEARFRAMEBUFFER:
          diag_clear(&ad->machine);
          break;
        case ID_FILE_EXIT:
          SendMessage(window, WM_CLOSE, 0, 0);
          break;
        case ID_TOOLS_DEBUGGER:
          return on_debugger(window);
        default:
          return FALSE;
      };
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_CLOSE: 
      return on_close(window);
      break;
    default:
      return DefWindowProc(window, message, wParam, lParam);
  }

  return FALSE;
}

static void attach_statusbar(HWND window, HINSTANCE instance, int height) {
  HWND status_window = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE,
      0, 0, 0, 0, window, NULL, instance, NULL);
  int parts[] = {200, 300, -1};
  RECT rect;
  SendMessage(status_window, SB_SETPARTS, 3, (LPARAM)parts);

  SendMessage(status_window, SB_SETTEXT, 0, (LPARAM)"Ready");
  SendMessage(status_window, SB_SETTEXT, 2, (LPARAM)"https://www.retrocoding.net");
  GetClientRect(window, &rect);
  MoveWindow(status_window, 0, rect.bottom - height, rect.right, height, TRUE);
}
