#pragma once

#include "global.h"
#include "xdg-shell-protocol.h"

struct xdg_shell {
  struct wl_compositor *wl_compositor;
  struct wl_surface    *wl_surface;

  struct xdg_wm_base  *xdg_wm_base;
  struct xdg_surface  *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;

  bool configure, close;
};

void xdg_shell_registry_global (void *data, struct wl_registry *registry,
                                uint32_t name, const char *interface,
                                uint32_t version);
void xdg_shell_registry_global_remove (void *data, struct wl_registry *registry,
                                       uint32_t name);

void xdg_shell_init (struct xdg_shell *shell, const char *title,
                     const char *app_id);
void xdg_shell_destroy (struct xdg_shell *shell);

void xdg_shell_present (struct xdg_shell *shell, struct wl_buffer *buffer);
