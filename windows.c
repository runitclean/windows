#include "windows.h"

static void registry_global (void *data, struct wl_registry *registry,
                             uint32_t name, const char *interface,
                             uint32_t version) {
  struct windows *w = data;

  input_device_registry_global (w->id, registry, name, interface, version);
  toplevel_list_registry_global (w->tl, registry, name, interface, version);
  image_copy_registry_global (w->ic, registry, name, interface, version);
  xdg_shell_registry_global (w->xs, registry, name, interface, version);
  shm_buffer_registry_global (&w->shm, registry, name, interface, version);
}

static void registry_global_remove (void *data, struct wl_registry *registry,
                                    uint32_t name) {
  struct windows *w = data;

  input_device_registry_global_remove (w->id, registry, name);
  toplevel_list_registry_global_remove (w->tl, registry, name);
  image_copy_registry_global_remove (w->ic, registry, name);
  xdg_shell_registry_global_remove (w->xs, registry, name);
  shm_buffer_registry_global_remove (&w->shm, registry, name);
}

static const struct wl_registry_listener registry_listener = {
    .global        = registry_global,
    .global_remove = registry_global_remove,
};

static void usage (FILE *out, const char *name) {
  fprintf (out,
           "Usage: %s [options...]\n"
           "\n"
           " -h             Show this help message and quit.\n"
           " -g <geometry>  Configure monitor geometry.\n"
           " -s <scale>     Configure monitor scale factor.\n"
           "\n"
           "Copyright (C) 2026 Jing Huang.\n",
           name);
}

int32_t main (int32_t argc, char **argv) {
  int32_t opt;

  char   *end;
  int32_t width, height;
  float   scale;

  width = height = scale = 1;

  while ((opt = getopt (argc, argv, "hg:s:")) != -1) {
    switch (opt) {
    case 'h':
      usage (stdout, *argv);
      return 0;
    case 'g':
      width  = strtol (optarg, &end, 10);
      height = strtol (++end, &end, 10);
      break;
    case 's':
      scale = atof (optarg);
      break;
    default:
      usage (stderr, *argv);
      return 1;
    }
  }

  if (width <= 1 || height <= 1)
    return 1;

  struct windows w;

  w.id = calloc (1, sizeof (*w.id));
  w.tl = calloc (1, sizeof (*w.tl));
  w.ic = calloc (1, sizeof (*w.ic));
  w.xs = calloc (1, sizeof (*w.xs));
  w.ea = calloc (1, sizeof (*w.ea));
  w.cd = calloc (1, sizeof (*w.cd));

  w.display  = wl_display_connect (NULL);
  w.registry = wl_display_get_registry (w.display);

  wl_registry_add_listener (w.registry, &registry_listener, &w);
  wl_display_roundtrip (w.display);

  input_device_init (w.id);
  toplevel_list_init (w.tl);

  wl_display_roundtrip (w.display);

  struct toplevel_list_object *tlo;
  struct image_copy_frame     *icf;

  wl_list_init (&w.windows);

  wl_list_for_each (tlo, &w.tl->toplevels, link) {
    struct windows_state *ws = calloc (1, sizeof (*ws));

    while (!tlo->closed && !tlo->done)
      wl_display_roundtrip (w.display);

    ws->identifier = strdup (tlo->identifier);
    ws->title      = strdup (tlo->title);
    ws->app_id     = strdup (tlo->app_id);

    icf = image_copy_frame_from_toplevel (w.ic, tlo->handle);

    while (!icf->done) {
      if (icf->stopped)
        goto error;
      wl_display_roundtrip (w.display);
    }

    ws->sb = shm_buffer_init (w.shm, icf->shm_format, icf->width, icf->height,
                              icf->width * 4);

    image_copy_init (icf, ws->sb->buffer);

    while (!icf->ready) {
      if (icf->failed)
        goto error;
      wl_display_roundtrip (w.display);
    }

  error:
    ws->error = icf->stopped || icf->failed;

    wl_list_insert (&w.windows, &ws->link);
    w.ea->window_count++;

    image_copy_destroy (icf);
  }

  toplevel_list_destroy (w.tl);

  xdg_shell_init (w.xs, "Window Overview", "windows");

  while (!w.xs->configure)
    wl_display_roundtrip (w.display);

  expose_algorithm_init (w.ea);

  struct windows_state *ws, *tmp;
  int32_t               i = 0;

  wl_list_for_each (ws, &w.windows, link) {
    struct expose_algorithm_window *eaw = &w.ea->eaw[i];

    eaw->node   = i++;
    eaw->width  = ws->sb->width;
    eaw->height = ws->sb->height;
    eaw->data   = ws;
  }

  w.ea->display_width  = width / scale;
  w.ea->display_height = height / scale;

  expose_algorithm_decide (w.ea);

  struct shm_buffer *sb =
      shm_buffer_init (w.shm, WL_SHM_FORMAT_ARGB8888, w.ea->display_width,
                       w.ea->display_height, w.ea->display_width * 4);

  cairo_draw_init (w.cd, sb->data, sb->width, sb->height, sb->stride);

  for (uint32_t i = 0; i < w.ea->window_count; i++) {
    struct expose_algorithm_window eaw = w.ea->eaw[i];
    struct windows_state          *ws  = eaw.data;

    cairo_draw_window (w.cd, ws->sb->data, ws->sb->width, ws->sb->height,
                       ws->sb->stride, eaw.x, eaw.y, eaw.scale_factor,
                       eaw.focused);
  }

  xdg_shell_present (w.xs, sb->buffer);

  while (!w.xs->close) {
    if (wl_display_dispatch (w.display) == -1)
      break;
  }

  cairo_draw_destroy (w.cd);
  expose_algorithm_destroy (w.ea);
  xdg_shell_destroy (w.xs);
  input_device_destroy (w.id);

  wl_list_for_each_safe (ws, tmp, &w.windows, link) {
    wl_list_remove (&ws->link);

    shm_buffer_destory (ws->sb);

    free (ws->identifier);
    free (ws->title);
    free (ws->app_id);

    free (ws);
  }

  free (w.id);
  free (w.tl);
  free (w.ic);
  free (w.xs);
  free (w.ea);
  free (w.cd);
}
