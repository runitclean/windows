#pragma once

#include "global.h"
#include "xdg-shell-protocol.h"

struct xdg_shell;

typedef void (*xdg_configure) (struct xdg_shell *app);
typedef void (*xdg_redraw) (struct xdg_shell *app);

struct xdg_shell {
  struct wl_display    *wl_display;
  struct wl_compositor *wl_compositor;
  struct wl_shm        *wl_shm;
  struct wl_surface    *wl_surface;

  struct xdg_wm_base  *wm_base;
  struct xdg_surface  *surface;
  struct xdg_toplevel *toplevel;

  int32_t  width;
  int32_t  height;
  uint32_t serial;

  bool configured;
  bool running;

  xdg_configure on_configure;
  xdg_redraw    on_redraw;

  void *data;
};

bool xdg_shell_init (struct xdg_shell *shell, const char *title,
                     const char *app_id, xdg_configure configure,
                     xdg_redraw redraw, void *data);
void xdg_shell_commit (struct xdg_shell *shell, struct wl_buffer *buffer);
void xdg_shell_run (struct xdg_shell *shell);
void xdg_shell_destroy (struct xdg_shell *shell);
