#include "chip8_errors.h"
#include "render.h"
#include <ddraw.h>

struct render_t {
  HANDLE wnd;

  IDirectDraw *ddraw;
  IDirectDrawSurface *front; 
  IDirectDrawSurface *back;
  IDirectDrawClipper *clipper;
};


int create_renderer(struct render_t **renderer) {
  HRESULT hr;
  struct render_t *r = (struct render_t *)malloc(sizeof(struct render_t));
  ZeroMemory(r, sizeof(struct render_t));

  hr = DirectDrawCreate(NULL, &r->ddraw, NULL);

  if (FAILED(hr)) {
    *renderer = NULL;
    return chip8_err_unknown;
  }

  *renderer = r;
  return chip8_success;
};

int initialise_renderer(struct render_t *renderer, void *window, int width, int height) {
  HRESULT hr;
  HWND wnd = (HWND) window;
  IDirectDraw *ddraw = renderer->ddraw;
  IDirectDrawSurface *front = NULL, *back = NULL;
  IDirectDrawClipper *clipper = NULL;
  DDSURFACEDESC ddsd;

  renderer->wnd = wnd;
  hr = IDirectDraw_SetCooperativeLevel(ddraw, wnd, DDSCL_NORMAL);

  if (FAILED(hr)) { goto err_handler; }

  ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);
  ddsd.dwFlags = DDSD_CAPS;
  ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  hr = IDirectDraw_CreateSurface(ddraw, &ddsd, &front, NULL);

  if (FAILED(hr)) { goto err_handler; }

  ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(DDSURFACEDESC);
  ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
  ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  ddsd.dwWidth = width;
  ddsd.dwHeight = height;

  hr = IDirectDraw_CreateSurface(ddraw, &ddsd, &back, NULL);

  if (FAILED(hr)) { goto err_handler; }

  hr = IDirectDraw_CreateClipper(ddraw, 0, &clipper, NULL);

  if (FAILED(hr)) { goto err_handler; }

  hr = IDirectDrawClipper_SetHWnd(clipper, 0, wnd);

  if (FAILED(hr)) { goto err_handler; }

  hr = IDirectDrawSurface_SetClipper(front, clipper);

  if (FAILED(hr)) { goto err_handler; }


  renderer->back = back; 
  renderer->front = front;
  renderer->clipper = clipper;

  return chip8_success;

err_handler:

  if (back) {
    IDirectDrawSurface_Release(back);
    back = NULL;
  }

  if (clipper) {
    IDirectDrawSurface_SetClipper(front, NULL);
    IDirectDrawClipper_Release(clipper);
    clipper = NULL;
  }


  if (front) {
    IDirectDrawSurface_Release(front);
    front = NULL;
  }

  if (renderer->ddraw) {
    IDirectDraw_Release(ddraw);
    renderer->ddraw = NULL;
  }

  return chip8_err_unknown;
};

int destroy_renderer(struct render_t *r) {
  if (r->back) {
    IDirectDrawSurface_Release(r->back);
    r->back = NULL;
  }

  if (r->clipper) {
    IDirectDrawSurface_SetClipper(r->front, NULL);
    IDirectDrawClipper_Release(r->clipper);
    r->clipper = NULL;
  }


  if (r->front) {
    IDirectDrawSurface_Release(r->front);
    r->front = NULL;
  }

  if (r->ddraw) {
    IDirectDraw_Release(r->ddraw);
    r->ddraw = NULL;
  }

  return chip8_success;
};

#define scale 10
#define fb_height 32 
#define fb_width 64
#define red_pixel 0xFFFF0000

static void draw_square(u8 *data, int pitch, int x, int y) {
  int i, offset;
  u32 *pixel;
  u32 bit_per_pixel = 32;

  // Draw the square of pixels
  for (i = 0; i < scale * scale; i++) {
    // Calculate the pixel offset
    offset = (y + i / scale) * pitch + (x + i % scale) * (bit_per_pixel / 8);
    pixel = (u32*)(data + offset);
    *pixel = red_pixel;
  }
}

static void draw_bitmap(u8* data, int pitch, u64* bitmap)
{
  int x, y, px, py;
  u64 row;
  u64 n = 1;

  // Draw the Chip-8 bitmap
  for (y = 0; y < fb_height; y++) {
    // Get the current row of the bitmap
    row = bitmap[y];

    // Draw the row of the bitmap
    for (x = 0; x < fb_width; x++)
    {
      // Check if the pixel is set
      if (row & (n << x))
      {
        // Calculate the pixel position
        px = x * scale;
        py = y * scale;

        // Draw the square of pixels
        draw_square(data, pitch, px, py);
      }
    }
  }
}

static void clear_surface(IDirectDrawSurface *surface) {
  DDBLTFX ftx; 
  ZeroMemory(&ftx, sizeof(DDBLTFX));

  ftx.dwSize = sizeof(DDBLTFX);
  ftx.dwFillColor = 0x00000000u;

  IDirectDrawSurface_Blt(surface, NULL, NULL, NULL, DDBLT_WAIT | DDBLT_COLORFILL, &ftx);
}

int render_display(struct render_t *renderer, struct machine_t *machine) {
  DDSURFACEDESC ddsd;
  HRESULT hr;
  int pitch;
  u8 *data;
  RECT rect; 

  /* Clear Back Buffer */
  clear_surface(renderer->back);

  /* Draw to Back Buffer */
  ZeroMemory(&ddsd, sizeof(DDSURFACEDESC));
  ddsd.dwSize = sizeof(ddsd);
  ddsd.dwFlags = DDSD_LPSURFACE | DDSD_PITCH;

  hr = IDirectDrawSurface_Lock(renderer->back, NULL, &ddsd, DDLOCK_WAIT, NULL);

  if (FAILED(hr)) return chip8_err_unknown;

  pitch = ddsd.lPitch;
  data = (u8*)ddsd.lpSurface;
  draw_bitmap(data, pitch, machine->display);

  IDirectDrawSurface_Unlock(renderer->back, data);

  /* Flip to Front Buffer */ 
  ZeroMemory(&rect, sizeof(RECT));
  GetClientRect(renderer->wnd, &rect);

  ClientToScreen(renderer->wnd, (LPPOINT) &rect.left);
  ClientToScreen(renderer->wnd, (LPPOINT) &rect.right);

  IDirectDrawSurface_Blt(renderer->front, &rect, renderer->back, NULL, DDBLT_WAIT, NULL);

  return chip8_success;
};
