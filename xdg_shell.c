#include "xdg_shell.h"

static void xdg_wm_base_ping (void *data, struct xdg_wm_base *wm_base,
                              uint32_t serial) {
  xdg_wm_base_pong (wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void wl_surface_preferred_buffer_scale (void              *data,
                                               struct wl_surface *surface,
                                               int32_t            scale) {
  struct xdg_shell *shell       = data;
  shell->preferred_buffer_scale = scale;
}

static const struct wl_surface_listener wl_surface_listener = {
    .preferred_buffer_scale = wl_surface_preferred_buffer_scale,
};

static void xdg_surface_configure (void *data, struct xdg_surface *surface,
                                   uint32_t serial) {
  struct xdg_shell *shell = data;

  xdg_surface_ack_configure (surface, serial);
  shell->configure = true;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_toplevel_configure (void *data, struct xdg_toplevel *toplevel,
                                    int32_t width, int32_t height,
                                    struct wl_array *states) {
  struct xdg_shell *shell = data;

  // see also wl_surface::preferred_buffer_scale
  if (width > 0)
    shell->width = width;
  if (height > 0)
    shell->height = height;
}

static void xdg_toplevel_close (void *data, struct xdg_toplevel *toplevel) {
  struct xdg_shell *shell = data;
  shell->close            = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close     = xdg_toplevel_close,
};

void xdg_shell_registry_global (void *data, struct wl_registry *registry,
                                uint32_t name, const char *interface,
                                uint32_t version) {
  struct xdg_shell *shell = data;

  if (strcmp (interface, wl_compositor_interface.name) == 0)
    shell->wl_compositor =
        wl_registry_bind (registry, name, &wl_compositor_interface, 4);
  else if (strcmp (interface, xdg_wm_base_interface.name) == 0)
    shell->xdg_wm_base =
        wl_registry_bind (registry, name, &xdg_wm_base_interface, 3);
}

void xdg_shell_registry_global_remove (void *data, struct wl_registry *registry,
                                       uint32_t name) {}

void xdg_shell_init (struct xdg_shell *shell, const char *title,
                     const char *app_id) {
  xdg_wm_base_add_listener (shell->xdg_wm_base, &xdg_wm_base_listener, shell);

  shell->wl_surface = wl_compositor_create_surface (shell->wl_compositor);
  wl_surface_add_listener (shell->wl_surface, &wl_surface_listener, shell);

  shell->xdg_surface =
      xdg_wm_base_get_xdg_surface (shell->xdg_wm_base, shell->wl_surface);
  xdg_surface_add_listener (shell->xdg_surface, &xdg_surface_listener, shell);

  shell->xdg_toplevel = xdg_surface_get_toplevel (shell->xdg_surface);

  xdg_toplevel_add_listener (shell->xdg_toplevel, &xdg_toplevel_listener,
                             shell);

  xdg_toplevel_set_title (shell->xdg_toplevel, title);
  xdg_toplevel_set_app_id (shell->xdg_toplevel, app_id);
  xdg_toplevel_set_fullscreen (shell->xdg_toplevel, NULL);

  wl_surface_commit (shell->wl_surface);
}

void xdg_shell_destroy (struct xdg_shell *shell) {
  if (shell->xdg_toplevel)
    xdg_toplevel_destroy (shell->xdg_toplevel);

  if (shell->xdg_surface)
    xdg_surface_destroy (shell->xdg_surface);

  if (shell->xdg_wm_base)
    xdg_wm_base_destroy (shell->xdg_wm_base);

  if (shell->wl_surface)
    wl_surface_destroy (shell->wl_surface);
}

void xdg_shell_present (struct xdg_shell *shell, struct wl_buffer *buffer) {
  wl_surface_attach (shell->wl_surface, buffer, 0, 0);
  wl_surface_damage_buffer (shell->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit (shell->wl_surface);
}
