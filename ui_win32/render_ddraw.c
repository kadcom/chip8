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

int render_display(struct render_t *renderer, struct machine_t *machine) {
  return chip8_err_unimplemented;
};
