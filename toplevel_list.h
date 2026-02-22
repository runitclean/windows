#pragma once

#include "ext-foreign-toplevel-list-v1-protocol.h"
#include "global.h"

struct toplevel_list {
  struct ext_foreign_toplevel_list_v1 *toplevel_list;
  struct wl_list                       toplevels;
};

struct toplevel_list_object {
  struct ext_foreign_toplevel_handle_v1 *handle;
  struct wl_list                         link;

  bool  closed, done;
  char *identifier;
};

void toplevel_list_registry_global (void *data, struct wl_registry *registry,
                                    uint32_t name, const char *interface,
                                    uint32_t version);
void toplevel_list_registry_global_remove (void               *data,
                                           struct wl_registry *registry,
                                           uint32_t            name);

void toplevel_list_init (struct toplevel_list *tl);
void toplevel_list_destroy (struct toplevel_list *tl);
