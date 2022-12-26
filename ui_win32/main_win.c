#include <windows.h>
#include <commctrl.h>

#include "chip8.h"
#include "chip8_errors.h"
#include "render.h"
#include "resource.h"

#define fb_width  64
#define fb_height 32

#define default_scale 10

static char window_class[] = "MainWindowClass";

static struct machine_t machine;
static struct render_t *renderer = NULL;

LRESULT CALLBACK main_window_proc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR cmdline, int show_state) {
  WNDCLASSEX wcex; 
  MSG msg; 
  HWND main_window;
  BOOL res;
  int border, menu;
  u64 one = 0x0100000000000001;

  HBRUSH black_brush = (HBRUSH) GetStockObject(BLACK_BRUSH);
  HICON  app_icon = LoadIcon(instance, IDI_APPLICATION);
  HCURSOR arrow_cursor = LoadCursor(instance, IDC_ARROW);
  HMENU main_menu = LoadMenu(instance, MAKEINTRESOURCE(IDM_MAIN_MENU));

  InitCommonControls();
  ZeroMemory(&wcex, sizeof(WNDCLASSEX));
  ZeroMemory(&msg, sizeof(MSG));

  init_machine(&machine);

  border = GetSystemMetrics(SM_CXBORDER);
  menu = GetSystemMetrics(SM_CYMENU);

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
      fb_height * default_scale + border + menu,
      NULL,
      NULL,
      instance,
      NULL);

  if (0 == main_window) {
    MessageBox(NULL, "Error Creating Window", "Error", MB_ICONHAND | MB_OK);
    UnregisterClass(window_class, instance);
    return -1;
  }

  if (chip8_failed(create_renderer(&renderer))) {
    goto cleanup;
  }

  if (chip8_failed(initialise_renderer(renderer, main_window, fb_width * default_scale, fb_height * default_scale))) {
    goto cleanup;
  }

  // check display 
  machine.display[0] = one;

  ShowWindow(main_window, show_state);
  UpdateWindow(main_window);

  for(;;) {
    if (PeekMessage(&msg, main_window, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        break;
      }

      TranslateMessage(&msg);
      DispatchMessage(&msg);
    } else {
      if (renderer) {
        render_display(renderer, &machine);
      }
    }
  }

cleanup:

  if (renderer) {
    destroy_renderer(renderer);
    renderer = NULL;
  }

  if (main_window) {
    DestroyWindow(main_window);
  }
  UnregisterClass(window_class, instance);

  return (int)msg.wParam;
}

static LRESULT on_close(HWND window) {
  int res = MessageBox(NULL, "Are you sure?", "Confirmation", MB_YESNO | MB_ICONQUESTION);

  if (IDYES == res) {
    if (renderer) {
      destroy_renderer(renderer);
      renderer = NULL; 
      SetWindowLongPtr(window, 0, (LONG_PTR) renderer);
    };

    PostQuitMessage(0);
  }

  return FALSE;
}

static LRESULT on_paint(HWND window) {
  if (!renderer) {
    return FALSE;
  }

  if (chip8_failed(render_display(renderer, &machine))) {
    // ** do nothing **/
  }

  return FALSE;
}

LRESULT CALLBACK main_window_proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message)
  {
    case WM_CLOSE: 
      return on_close(window);
      break;
    default:
      return DefWindowProc(window, message, wParam, lParam);
  }

  return FALSE;
}

