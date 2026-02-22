#pragma once

#include "fractional-scale-v1-protocol.h"
#include "global.h"
#include "viewporter.h"
#include "xdg-shell-protocol.h"

struct xdg_shell {
  struct wl_compositor *wl_compositor;
  struct wl_surface    *wl_surface;

  struct wp_fractional_scale_manager_v1 *fractional_scale_manager;
  struct wp_fractional_scale_v1         *fractional_scale;

  struct wp_viewporter *viewporter;
  struct wp_viewport   *viewport;

  struct xdg_wm_base  *xdg_wm_base;
  struct xdg_surface  *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;

  float scale;
  bool  configure, close;
};

void xdg_shell_registry_global (void *data, struct wl_registry *registry,
                                uint32_t name, const char *interface,
                                uint32_t version);
void xdg_shell_registry_global_remove (void *data, struct wl_registry *registry,
                                       uint32_t name);

void xdg_shell_init (struct xdg_shell *shell, const char *title,
                     const char *app_id);
void xdg_shell_destroy (struct xdg_shell *shell);

void xdg_shell_viewport (struct xdg_shell *xs, int32_t buffer_width,
                         int32_t buffer_height);
void xdg_shell_present (struct xdg_shell *shell, struct wl_buffer *buffer);
