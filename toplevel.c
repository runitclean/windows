#include "toplevel.h"

static void foreign_toplevel_handle_closed (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {}

static void foreign_toplevel_handle_done (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {
  struct toplevel *tl = data;
  tl->done            = true;
}

static void foreign_toplevel_handle_title (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *title) {
  struct toplevel *tl = data;
  free (tl->title);
  tl->title = strdup (title);
}

static void foreign_toplevel_handle_app_id (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *app_id) {
  struct toplevel *tl = data;
  free (tl->app_id);
  tl->app_id = strdup (app_id);
}

static void foreign_toplevel_handle_identifier (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *identifier) {
  struct toplevel *tl = data;
  // the compositor must only send this event when the handle is created
  tl->identifier = strdup (identifier);
}

static const struct ext_foreign_toplevel_handle_v1_listener
    foreign_toplevel_listener = {
        .closed     = foreign_toplevel_handle_closed,
        .done       = foreign_toplevel_handle_done,
        .title      = foreign_toplevel_handle_title,
        .app_id     = foreign_toplevel_handle_app_id,
        .identifier = foreign_toplevel_handle_identifier,
};

static void foreign_toplevel_list_handle_toplevel (
    void *data, struct ext_foreign_toplevel_list_v1 *list,
    struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {
  struct state    *s  = data;
  struct toplevel *tl = calloc (1, sizeof (*tl));

  wl_list_insert (&s->toplevels, &tl->link);

  tl->handle = toplevel_handle;

  ext_foreign_toplevel_handle_v1_add_listener (toplevel_handle,
                                               &foreign_toplevel_listener, tl);
}

static void foreign_toplevel_list_handle_finished (
    void *data, struct ext_foreign_toplevel_list_v1 *list) {}

static const struct ext_foreign_toplevel_list_v1_listener
    foreign_toplevel_list_listener = {
        .toplevel = foreign_toplevel_list_handle_toplevel,
        .finished = foreign_toplevel_list_handle_finished,
};

bool toplevel_init (struct state *s) {
  if (!s->toplevel_list)
    return false;

  ext_foreign_toplevel_list_v1_add_listener (
      s->toplevel_list, &foreign_toplevel_list_listener, s);
  return true;
}

void toplevel_destroy (struct state *s) {
  struct toplevel *tl, *tl_tmp;

  wl_list_for_each_safe (tl, tl_tmp, &s->toplevels, link) {
    wl_list_remove (&tl->link);

    free (tl->title);
    free (tl->app_id);
    free (tl->identifier);

    ext_foreign_toplevel_handle_v1_destroy (tl->handle);

    free (tl);
  }

  if (s->toplevel_list != NULL)
    ext_foreign_toplevel_list_v1_destroy (s->toplevel_list);
}
