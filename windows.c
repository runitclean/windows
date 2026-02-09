#include "windows.h"

static void registry_global (void *data, struct wl_registry *registry,
                             uint32_t name, const char *interface,
                             uint32_t version) {
  struct windows *w = data;

  toplevel_list_registry_global (w->tl, registry, name, interface, version);
  image_copy_registry_global (w->ic, registry, name, interface, version);
  xdg_shell_registry_global (w->xs, registry, name, interface, version);
  shm_buffer_registry_global (&w->shm, registry, name, interface, version);
}

static void registry_global_remove (void *data, struct wl_registry *registry,
                                    uint32_t name) {
  struct windows *w = data;

  toplevel_list_registry_global_remove (w->tl, registry, name);
  image_copy_registry_global_remove (w->ic, registry, name);
  xdg_shell_registry_global_remove (w->xs, registry, name);
  shm_buffer_registry_global_remove (&w->shm, registry, name);
}

static const struct wl_registry_listener registry_listener = {
    .global        = registry_global,
    .global_remove = registry_global_remove,
};

int main (void) {
  struct windows w = {0};

  w.tl = calloc (1, sizeof (*w.tl));
  w.ic = calloc (1, sizeof (*w.ic));
  w.xs = calloc (1, sizeof (*w.xs));
  w.ea = calloc (1, sizeof (*w.ea));

  w.display  = wl_display_connect (NULL);
  w.registry = wl_display_get_registry (w.display);

  wl_registry_add_listener (w.registry, &registry_listener, &w);
  wl_display_roundtrip (w.display);

  toplevel_list_init (w.tl);
  wl_display_roundtrip (w.display);

  struct toplevel_list_object *tlo = {0};
  struct image_copy_frame     *icf = {0};

  wl_list_init (&w.windows);

  wl_list_for_each (tlo, &w.tl->toplevels, link) {
    struct windows_state *ws = calloc (1, sizeof (*ws));

    while (!tlo->closed && !tlo->done)
      wl_display_roundtrip (w.display);

    printf ("id: %s; title: %s; app: %s\n", tlo->identifier, tlo->title,
            tlo->app_id); // TODO: remove

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
                              icf->width *
                                  4); // TODO: use cairo_format_stride_for_width

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

  struct windows_state *ws, *tmp;

  w.ea->eaw = calloc (w.ea->window_count, sizeof (*w.ea->eaw));

  int32_t i = 0;

  wl_list_for_each (ws, &w.windows, link) {
    struct expose_algorithm_window *eaw = &w.ea->eaw[i];

    eaw->node   = i++;
    eaw->width  = ws->sb->width;
    eaw->height = ws->sb->height;
    eaw->data   = ws;
  }

  w.ea->display_width  = w.xs->width * w.xs->preferred_buffer_scale;
  w.ea->display_height = w.xs->height * w.xs->preferred_buffer_scale;

  expose_algorithm_init (w.ea);

  wl_list_for_each_safe (ws, tmp, &w.windows, link) {
    wl_list_remove (&ws->link);

    shm_buffer_destory (ws->sb);

    free (ws->identifier);
    free (ws->title);
    free (ws->app_id);

    free (ws);
  }

  free (w.tl);
  free (w.ic);
  free (w.xs);
  free (w.ea->eaw);
  free (w.ea);
}
