#include "windows.h"

static void registry_global (void *data, struct wl_registry *registry,
                             uint32_t name, const char *interface,
                             uint32_t version) {
  struct windows *w = data;

  output_info_registry_global (w->oi, registry, name, interface, version);
  input_device_registry_global (w->id, registry, name, interface, version);
  toplevel_list_registry_global (w->tl, registry, name, interface, version);
  image_copy_registry_global (w->ic, registry, name, interface, version);
  xdg_shell_registry_global (w->xs, registry, name, interface, version);
  shm_buffer_registry_global (&w->shm, registry, name, interface, version);
}

static void registry_global_remove (void *data, struct wl_registry *registry,
                                    uint32_t name) {
  struct windows *w = data;

  output_info_registry_global_remove (w->oi, registry, name);
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

// forward declaration needed for rescheduling callback
static const struct wl_callback_listener callback_listener;

static void callback_done (void *data, struct wl_callback *callback,
                           uint32_t callback_data) {
  struct windows *w = data;

  wl_callback_destroy (callback);

  w->callback = callback = wl_surface_frame (w->xs->wl_surface);
  wl_callback_add_listener (callback, &callback_listener, w);

  if (w->render) {
    cairo_draw_clear (w->cd);

    for (uint32_t i = 0; i < w->ea->window_count; i++) {
      struct expose_algorithm_window eaw = w->ea->eaw[i];
      struct windows_state          *ws  = eaw.data;

      cairo_draw_window (w->cd, ws->sb->data, ws->sb->width, ws->sb->height,
                         ws->sb->stride, eaw.x, eaw.y, eaw.scale_factor,
                         eaw.focused);
    }
  }

  w->render = false;

  xdg_shell_present (w->xs, w->sb->buffer);
}

static const struct wl_callback_listener callback_listener = {
  .done = callback_done,
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

static void escape (void *data) {
  struct windows *w = data;
  w->xs->close      = true;
}

static void left (void *data) {
  struct windows *w    = data;
  bool            find = false;

  for (uint32_t i = 0; i < w->ea->window_count; i++)
    if (w->ea->eaw[i].focused) {
      find                                   = true;
      w->ea->eaw[i].focused                  = false;
      w->ea->eaw[w->ea->eaw[i].left].focused = true;
      break;
    }

  if (!find)
    w->ea->eaw[0].focused = true;

  w->render = true;
}

static void right (void *data) {
  struct windows *w    = data;
  bool            find = false;

  for (uint32_t i = 0; i < w->ea->window_count; i++)
    if (w->ea->eaw[i].focused) {
      find                                    = true;
      w->ea->eaw[i].focused                   = false;
      w->ea->eaw[w->ea->eaw[i].right].focused = true;
      break;
    }

  if (!find)
    w->ea->eaw[0].focused = true;

  w->render = true;
}

static void up (void *data) {
  struct windows *w    = data;
  bool            find = false;

  for (uint32_t i = 0; i < w->ea->window_count; i++)
    if (w->ea->eaw[i].focused) {
      find                                 = true;
      w->ea->eaw[i].focused                = false;
      w->ea->eaw[w->ea->eaw[i].up].focused = true;
      break;
    }

  if (!find)
    w->ea->eaw[0].focused = true;

  w->render = true;
}

static void down (void *data) {
  struct windows *w    = data;
  bool            find = false;

  for (uint32_t i = 0; i < w->ea->window_count; i++)
    if (w->ea->eaw[i].focused) {
      find                                   = true;
      w->ea->eaw[i].focused                  = false;
      w->ea->eaw[w->ea->eaw[i].down].focused = true;
      break;
    }

  if (!find)
    w->ea->eaw[0].focused = true;

  w->render = true;
}

static void enter (void *data) {
  struct windows *w = data;

  for (uint32_t i = 0; i < w->ea->window_count; i++)
    if (w->ea->eaw[i].focused) {
      struct windows_state *ws = w->ea->eaw[i].data;
      printf ("%s\n", ws->identifier);

      w->xs->close = true;
    }
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

  w.oi = calloc (1, sizeof (*w.oi));
  w.id = calloc (1, sizeof (*w.id));
  w.tl = calloc (1, sizeof (*w.tl));
  w.ic = calloc (1, sizeof (*w.ic));
  w.xs = calloc (1, sizeof (*w.xs));
  w.ea = calloc (1, sizeof (*w.ea));
  w.sb = calloc (1, sizeof (*w.sb));
  w.cd = calloc (1, sizeof (*w.cd));

  w.display  = wl_display_connect (NULL);
  w.registry = wl_display_get_registry (w.display);

  output_info_init (w.oi);
  input_device_init (w.id);

  wl_registry_add_listener (w.registry, &registry_listener, &w);
  wl_display_roundtrip (w.display);

  toplevel_list_init (w.tl);
  image_copy_init (w.ic);

  wl_display_roundtrip (w.display);

  w.id->data   = &w;
  w.id->escape = escape;
  w.id->left   = left;
  w.id->right  = right;
  w.id->up     = up;
  w.id->down   = down;
  w.id->enter  = enter;

  struct toplevel_list_object *tlo;

  wl_list_init (&w.windows);

  wl_list_for_each (tlo, &w.tl->toplevels, link) {
    struct windows_state    *ws  = calloc (1, sizeof (*ws));
    struct image_copy_frame *icf = calloc (1, sizeof (*icf));

    while (!tlo->closed && !tlo->done)
      wl_display_roundtrip (w.display);

    ws->identifier = strdup (tlo->identifier);
    ws->title      = strdup (tlo->title);
    ws->app_id     = strdup (tlo->app_id);

    image_copy_session (w.ic, icf, tlo->handle);

    while (!icf->done) {
      if (icf->stopped)
        goto error;
      wl_display_roundtrip (w.display);
    }

    ws->sb = calloc (1, sizeof (*ws->sb));

    shm_buffer_init (ws->sb, w.shm, icf->shm_format, icf->width, icf->height,
                     icf->width * 4);

    image_copy_toplevel (icf, ws->sb->buffer);

    while (!icf->ready) {
      if (icf->failed)
        goto error;
      wl_display_roundtrip (w.display);
    }

  error:
    ws->error = icf->stopped || icf->failed;
    free (icf);

    wl_list_insert (&w.windows, &ws->link);
    w.ea->window_count++;
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

  shm_buffer_init (w.sb, w.shm, WL_SHM_FORMAT_ARGB8888, w.ea->display_width,
                   w.ea->display_height, w.ea->display_width * 4);
  cairo_draw_init (w.cd, w.sb->data, w.sb->width, w.sb->height, w.sb->stride);

  struct wl_callback *callback = wl_surface_frame (w.xs->wl_surface);
  wl_callback_add_listener (callback, &callback_listener, &w);

  // submit an initial frame for the frame done callback
  for (uint32_t i = 0; i < w.ea->window_count; i++) {
    struct expose_algorithm_window eaw = w.ea->eaw[i];
    struct windows_state          *ws  = eaw.data;

    cairo_draw_window (w.cd, ws->sb->data, ws->sb->width, ws->sb->height,
                       ws->sb->stride, eaw.x, eaw.y, eaw.scale_factor,
                       eaw.focused);
  }

  w.render = false;

  xdg_shell_present (w.xs, w.sb->buffer);

  struct pollfd fds[2] = {
    {
      .fd     = wl_display_get_fd (w.display),
      .events = POLLIN,
    },
    {
      .fd     = w.id->repeat_timer,
      .events = POLLIN,
    },
  };

  while (!w.xs->close) {
    wl_display_flush (w.display);

    if (poll (fds, 2, -1) == -1)
      break;

    if (fds[0].revents & POLLIN)
      wl_display_dispatch (w.display);

    if (fds[1].revents & POLLIN)
      input_device_dispatch (w.id);
  }

  cairo_draw_destroy (w.cd);
  shm_buffer_destroy (w.sb);
  expose_algorithm_destroy (w.ea);
  xdg_shell_destroy (w.xs);
  image_copy_destroy (w.ic);
  input_device_destroy (w.id);
  output_info_destroy (w.oi);

  wl_shm_destroy (w.shm);

  wl_list_for_each_safe (ws, tmp, &w.windows, link) {
    wl_list_remove (&ws->link);

    shm_buffer_destroy (ws->sb);

    free (ws->sb);
    free (ws->identifier);
    free (ws->title);
    free (ws->app_id);

    free (ws);
  }

  wl_callback_destroy (w.callback);
  wl_registry_destroy (w.registry);
  wl_display_disconnect (w.display);

  free (w.oi);
  free (w.id);
  free (w.tl);
  free (w.ic);
  free (w.xs);
  free (w.ea);
  free (w.sb);
  free (w.cd);
}
