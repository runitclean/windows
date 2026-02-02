#include "xdg_shell.h"

static void registry_global (void *data, struct wl_registry *registry,
                             uint32_t name, const char *interface,
                             uint32_t version) {
  struct xdg_shell *app = data;

  if (strcmp (interface, wl_compositor_interface.name) == 0)
    app->wl_compositor =
        wl_registry_bind (registry, name, &wl_compositor_interface, 4);
  else if (strcmp (interface, wl_shm_interface.name) == 0)
    app->wl_shm = wl_registry_bind (registry, name, &wl_shm_interface, 1);
  else if (strcmp (interface, xdg_wm_base_interface.name) == 0)
    app->wm_base = wl_registry_bind (registry, name, &xdg_wm_base_interface, 3);
}

static void registry_global_remove (void *data, struct wl_registry *registry,
                                    uint32_t name) {}

static const struct wl_registry_listener registry_listener = {
    .global        = registry_global,
    .global_remove = registry_global_remove,
};

static void wm_base_ping (void *data, struct xdg_wm_base *wm, uint32_t serial) {
  xdg_wm_base_pong (wm, serial);
}

static const struct xdg_wm_base_listener wm_base_listener = {
    .ping = wm_base_ping,
};

static void xdg_surface_configure (void *data, struct xdg_surface *surface,
                                   uint32_t serial) {
  struct xdg_shell *shell = data;

  shell->serial = serial;
  xdg_surface_ack_configure (surface, serial);

  if (!shell->configured) {
    shell->configured = true;

    if (shell->on_configure)
      shell->on_configure (shell);
  }
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_toplevel_configure (void *data, struct xdg_toplevel *toplevel,
                                    int32_t width, int32_t height,
                                    struct wl_array *states) {
  struct xdg_shell *shell = data;

  if (width > 0)
    shell->width = width;
  if (height > 0)
    shell->height = height;
}

static void xdg_toplevel_close (void *data, struct xdg_toplevel *toplevel) {
  struct xdg_shell *shell = data;
  shell->running          = false;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close     = xdg_toplevel_close,
};

bool xdg_shell_init (struct xdg_shell *shell, const char *title,
                     const char *app_id, xdg_configure configure,
                     xdg_redraw redraw, void *data) {
  shell->on_configure = configure;
  shell->on_redraw    = redraw;
  shell->data         = data;
  shell->running      = true;

  shell->wl_display = wl_display_connect (NULL);

  if (!shell->wl_display)
    return false;

  struct wl_registry *registry = wl_display_get_registry (shell->wl_display);
  wl_registry_add_listener (registry, &registry_listener, shell);

  wl_display_roundtrip (shell->wl_display);

  if (!shell->wl_compositor || !shell->wl_shm || !shell->wm_base)
    return false;

  xdg_wm_base_add_listener (shell->wm_base, &wm_base_listener, shell);

  shell->wl_surface = wl_compositor_create_surface (shell->wl_compositor);
  shell->surface =
      xdg_wm_base_get_xdg_surface (shell->wm_base, shell->wl_surface);
  shell->toplevel = xdg_surface_get_toplevel (shell->surface);

  xdg_toplevel_set_fullscreen (shell->toplevel, NULL);

  xdg_surface_add_listener (shell->surface, &xdg_surface_listener, shell);
  xdg_toplevel_add_listener (shell->toplevel, &xdg_toplevel_listener, shell);

  xdg_toplevel_set_title (shell->toplevel, title);
  xdg_toplevel_set_app_id (shell->toplevel, app_id);

  wl_surface_commit (shell->wl_surface);
  wl_display_roundtrip (shell->wl_display);

  return true;
}

void xdg_shell_commit (struct xdg_shell *shell, struct wl_buffer *buffer) {
  wl_surface_attach (shell->wl_surface, buffer, 0, 0);
  wl_surface_damage_buffer (shell->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit (shell->wl_surface);
}

void xdg_shell_run (struct xdg_shell *shell) {
  while (shell->running) {
    wl_display_dispatch (shell->wl_display);

    if (shell->configured && shell->on_redraw)
      shell->on_redraw (shell);
  }
}

void xdg_shell_destroy (struct xdg_shell *shell) {
  if (shell->toplevel)
    xdg_toplevel_destroy (shell->toplevel);
  if (shell->surface)
    xdg_surface_destroy (shell->surface);
  if (shell->wl_surface)
    wl_surface_destroy (shell->wl_surface);
  if (shell->wm_base)
    xdg_wm_base_destroy (shell->wm_base);
  if (shell->wl_display)
    wl_display_disconnect (shell->wl_display);
}
