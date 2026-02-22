#include "output_info.h"

static void output_listener_geometry (void *data, struct wl_output *output,
                                      int32_t x, int32_t y,
                                      int32_t physical_width,
                                      int32_t physical_height, int32_t subpixel,
                                      const char *make, const char *model,
                                      int32_t transform) {
  // TODO: handle transformation applied to buffer contents during presentation
}

static void output_listener_mode (void *data, struct wl_output *output,
                                  uint32_t flags, int32_t width, int32_t height,
                                  int32_t refresh) {
  struct output_info_object *oio = data;

  // non-current modes are depreciated, while compositors might still decide to
  // advertse it (really?)
  if (flags & WL_OUTPUT_MODE_CURRENT) {
    oio->width  = width;
    oio->height = height;
  }
}

static void output_listener_done (void *data, struct wl_output *output) {
  struct output_info_object *oio = data;
  oio->done                      = true;
}

static void output_listener_scale (void *data, struct wl_output *output,
                                   int32_t factor) {
  // use wl_surface.preferred_buffer_scale instead
}

static void output_listener_name (void *data, struct wl_output *output,
                                  const char *name) {
  struct output_info_object *oio = data;
  oio->monitor                   = strdup (name);
}

static void output_listener_description (void *data, struct wl_output *output,
                                         const char *description) {}

static const struct wl_output_listener output_listener = {
  .geometry    = output_listener_geometry,
  .mode        = output_listener_mode,
  .done        = output_listener_done,
  .scale       = output_listener_scale,
  .name        = output_listener_name,
  .description = output_listener_description,
};

void output_info_registry_global (void *data, struct wl_registry *registry,
                                  uint32_t name, const char *interface,
                                  uint32_t version) {
  struct output_info *oi = data;

  if (strcmp (interface, wl_output_interface.name) == 0) {
    struct wl_output *output =
      wl_registry_bind (registry, name, &wl_output_interface, 4);

    struct output_info_object *oio = calloc (1, sizeof (*oio));

    oio->output = output;
    oio->name   = name;

    wl_list_insert (&oi->outputs, &oio->link);

    // should we use xdg_output insead?
    wl_output_add_listener (output, &output_listener, oio);
  }
}

void output_info_registry_global_remove (void               *data,
                                         struct wl_registry *registry,
                                         uint32_t            name) {
  struct output_info        *oi = data;
  struct output_info_object *oio, *tmp;

  wl_list_for_each_safe (oio, tmp, &oi->outputs, link) if (oio->name == name) {
    wl_list_remove (&oio->link);

    wl_output_destroy (oio->output);
    free (oio->monitor);

    free (oio);
  }
}

void output_info_init (struct output_info *oi) { wl_list_init (&oi->outputs); }

void output_info_destroy (struct output_info *oi) {
  struct output_info_object *oio, *tmp;

  wl_list_for_each_safe (oio, tmp, &oi->outputs, link) {
    wl_list_remove (&oio->link);

    wl_output_destroy (oio->output);
    free (oio->monitor);

    free (oio);
  }
}
