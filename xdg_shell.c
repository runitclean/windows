#include "xdg_shell.h"

static void fractional_scale_preferred_scale (
  void *data, struct wp_fractional_scale_v1 *fractional_scale, uint32_t scale) {
  struct xdg_shell *xs = data;
  xs->scale            = (float) scale / 120.;
}

static const struct wp_fractional_scale_v1_listener fractional_scale_listener =
  {
    .preferred_scale = fractional_scale_preferred_scale,
};

static void xdg_wm_base_ping (void *data, struct xdg_wm_base *wm_base,
                              uint32_t serial) {
  xdg_wm_base_pong (wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
  .ping = xdg_wm_base_ping,
};

static void xdg_surface_configure (void *data, struct xdg_surface *surface,
                                   uint32_t serial) {
  struct xdg_shell *xs = data;
  xs->configure        = true;

  xdg_surface_ack_configure (surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
  .configure = xdg_surface_configure,
};

static void xdg_toplevel_configure (void *data, struct xdg_toplevel *toplevel,
                                    int32_t width, int32_t height,
                                    struct wl_array *states) {}

static void xdg_toplevel_close (void *data, struct xdg_toplevel *toplevel) {
  struct xdg_shell *xs = data;
  xs->close            = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
  .configure = xdg_toplevel_configure,
  .close     = xdg_toplevel_close,
};

void xdg_shell_registry_global (void *data, struct wl_registry *registry,
                                uint32_t name, const char *interface,
                                uint32_t version) {
  struct xdg_shell *xs = data;

  if (strcmp (interface, wl_compositor_interface.name) == 0)
    xs->wl_compositor =
      wl_registry_bind (registry, name, &wl_compositor_interface, 4);
  else if (strcmp (interface, wp_fractional_scale_manager_v1_interface.name) ==
           0)
    xs->fractional_scale_manager = wl_registry_bind (
      registry, name, &wp_fractional_scale_manager_v1_interface, 1);
  else if (strcmp (interface, wp_viewporter_interface.name) == 0)
    xs->viewporter =
      wl_registry_bind (registry, name, &wp_viewporter_interface, 1);
  else if (strcmp (interface, xdg_wm_base_interface.name) == 0)
    xs->xdg_wm_base =
      wl_registry_bind (registry, name, &xdg_wm_base_interface, 3);
}

void xdg_shell_registry_global_remove (void *data, struct wl_registry *registry,
                                       uint32_t name) {}

void xdg_shell_init (struct xdg_shell *xs, const char *title,
                     const char *app_id) {
  xdg_wm_base_add_listener (xs->xdg_wm_base, &xdg_wm_base_listener, xs);

  xs->wl_surface = wl_compositor_create_surface (xs->wl_compositor);

  xs->fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale (
    xs->fractional_scale_manager, xs->wl_surface);
  wp_fractional_scale_v1_add_listener (xs->fractional_scale,
                                       &fractional_scale_listener, xs);

  xs->viewport = wp_viewporter_get_viewport (xs->viewporter, xs->wl_surface);

  xs->xdg_surface =
    xdg_wm_base_get_xdg_surface (xs->xdg_wm_base, xs->wl_surface);
  xdg_surface_add_listener (xs->xdg_surface, &xdg_surface_listener, xs);

  xs->xdg_toplevel = xdg_surface_get_toplevel (xs->xdg_surface);
  xdg_toplevel_add_listener (xs->xdg_toplevel, &xdg_toplevel_listener, xs);

  xdg_toplevel_set_title (xs->xdg_toplevel, title);
  xdg_toplevel_set_app_id (xs->xdg_toplevel, app_id);
  xdg_toplevel_set_fullscreen (xs->xdg_toplevel, NULL);

  wl_surface_commit (xs->wl_surface);
}

void xdg_shell_destroy (struct xdg_shell *xs) {
  xdg_toplevel_destroy (xs->xdg_toplevel);
  xdg_surface_destroy (xs->xdg_surface);
  xdg_wm_base_destroy (xs->xdg_wm_base);

  wp_viewport_destroy (xs->viewport);
  wp_viewporter_destroy (xs->viewporter);

  wp_fractional_scale_v1_destroy (xs->fractional_scale);
  wp_fractional_scale_manager_v1_destroy (xs->fractional_scale_manager);

  wl_surface_destroy (xs->wl_surface);
  wl_compositor_destroy (xs->wl_compositor);
}

void xdg_shell_viewport (struct xdg_shell *xs, int32_t buffer_width,
                         int32_t buffer_height) {
  wp_viewport_set_destination (xs->viewport, buffer_width / xs->scale,
                               buffer_height / xs->scale);
}

void xdg_shell_present (struct xdg_shell *xs, struct wl_buffer *buffer) {
  wl_surface_attach (xs->wl_surface, buffer, 0, 0);
  wl_surface_damage_buffer (xs->wl_surface, 0, 0, INT32_MAX, INT32_MAX);
  wl_surface_commit (xs->wl_surface);
}
