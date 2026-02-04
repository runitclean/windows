#include "toplevel_list.h"

static void foreign_toplevel_handle_closed (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {
  struct toplevel_list_object *tlo = data;
  tlo->closed                      = true;
}

static void foreign_toplevel_handle_done (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {
  struct toplevel_list_object *tlo = data;
  tlo->done                        = true;
}

static void foreign_toplevel_handle_title (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *title) {
  struct toplevel_list_object *tlo = data;
  free (tlo->title);
  tlo->title = strdup (title);
}

static void foreign_toplevel_handle_app_id (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *app_id) {
  struct toplevel_list_object *tlo = data;
  free (tlo->app_id);
  tlo->app_id = strdup (app_id);
}

static void foreign_toplevel_handle_identifier (
    void *data, struct ext_foreign_toplevel_handle_v1 *toplevel_handle,
    const char *identifier) {
  struct toplevel_list_object *tlo = data;
  // the compositor must only send this event when the handle is created
  tlo->identifier = strdup (identifier);
}

static const struct ext_foreign_toplevel_handle_v1_listener
    foreign_toplevel_handle_listener = {
        .closed     = foreign_toplevel_handle_closed,
        .done       = foreign_toplevel_handle_done,
        .title      = foreign_toplevel_handle_title,
        .app_id     = foreign_toplevel_handle_app_id,
        .identifier = foreign_toplevel_handle_identifier,
};

static void foreign_toplevel_list_toplevel (
    void *data, struct ext_foreign_toplevel_list_v1 *list,
    struct ext_foreign_toplevel_handle_v1 *toplevel_handle) {
  struct toplevel_list        *tl  = data;
  struct toplevel_list_object *tlo = calloc (1, sizeof (*tlo));

  wl_list_insert (&tl->toplevels, &tlo->link);

  tlo->handle = toplevel_handle;
  ext_foreign_toplevel_handle_v1_add_listener (
      tlo->handle, &foreign_toplevel_handle_listener, tlo);
}

static void
foreign_toplevel_list_finished (void                                *data,
                                struct ext_foreign_toplevel_list_v1 *list) {
  struct toplevel_list *tl = data;
  tl->finished             = true;
}

static const struct ext_foreign_toplevel_list_v1_listener
    foreign_toplevel_list_listener = {
        .toplevel = foreign_toplevel_list_toplevel,
        .finished = foreign_toplevel_list_finished,
};

void toplevel_list_registry_global (void *data, struct wl_registry *registry,
                                    uint32_t name, const char *interface,
                                    uint32_t version) {
  struct toplevel_list *tl = data;

  if (strcmp (interface, ext_foreign_toplevel_list_v1_interface.name) == 0)
    tl->toplevel_list = wl_registry_bind (
        registry, name, &ext_foreign_toplevel_list_v1_interface, 1);
}

void toplevel_list_registry_global_remove (void               *data,
                                           struct wl_registry *registry,
                                           uint32_t            name) {}

void toplevel_list_init (struct toplevel_list *tl) {
  wl_list_init (&tl->toplevels);

  ext_foreign_toplevel_list_v1_add_listener (
      tl->toplevel_list, &foreign_toplevel_list_listener, tl);
}

void toplevel_list_destroy (struct toplevel_list *tl) {
  struct toplevel_list_object *tlo, *tmp;

  wl_list_for_each_safe (tlo, tmp, &tl->toplevels, link) {
    wl_list_remove (&tlo->link);

    free (tlo->title);
    free (tlo->app_id);
    free (tlo->identifier);

    ext_foreign_toplevel_handle_v1_destroy (tlo->handle);

    free (tlo);
  }

  if (tl->toplevel_list != NULL)
    ext_foreign_toplevel_list_v1_destroy (tl->toplevel_list);
}
