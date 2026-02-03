#include "toplevel_list.h"

static void foreign_toplevel_handle_closed (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {}

static void foreign_toplevel_handle_done (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {
  struct toplevel *t = data;
  t->done            = true;
}

static void foreign_toplevel_handle_title (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *title) {
  struct toplevel *t = data;
  free (t->title);
  t->title = strdup (title);
}

static void foreign_toplevel_handle_app_id (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *app_id) {
  struct toplevel *t = data;
  free (t->app_id);
  t->app_id = strdup (app_id);
}

static void foreign_toplevel_handle_identifier (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *identifier) {
  struct toplevel *t = data;
  // the compositor must only send this event when the handle is created
  t->identifier = strdup (identifier);
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
  struct toplevel      *t  = calloc (1, sizeof (*t));
  struct toplevel_list *tl = data;

  wl_list_insert (&tl->toplevels, &t->link);

  t->handle = toplevel_handle;

  ext_foreign_toplevel_handle_v1_add_listener (toplevel_handle,
                                               &foreign_toplevel_listener, t);
}

static void foreign_toplevel_list_handle_finished (
    void *data, struct ext_foreign_toplevel_list_v1 *list) {}

static const struct ext_foreign_toplevel_list_v1_listener
    foreign_toplevel_list_listener = {
        .toplevel = foreign_toplevel_list_handle_toplevel,
        .finished = foreign_toplevel_list_handle_finished,
};

bool toplevel_list_init (struct toplevel_list *tl) {
  if (!tl->toplevel_list)
    return false;

  ext_foreign_toplevel_list_v1_add_listener (
      tl->toplevel_list, &foreign_toplevel_list_listener, tl);
  return true;
}

void toplevel_list_destroy (struct toplevel_list *tl) {
  struct toplevel *t, *tmp;

  wl_list_for_each_safe (t, tmp, &tl->toplevels, link) {
    wl_list_remove (&t->link);

    free (t->title);
    free (t->app_id);
    free (t->identifier);

    ext_foreign_toplevel_handle_v1_destroy (t->handle);

    free (t);
  }

  if (tl->toplevel_list != NULL)
    ext_foreign_toplevel_list_v1_destroy (tl->toplevel_list);
}
