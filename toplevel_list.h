#pragma once

#include "ext-foreign-toplevel-list-v1-protocol.h"
#include "global.h"

struct toplevel {
  struct ext_foreign_toplevel_handle_v1 *handle;
  struct wl_list                         link;

  bool done;

  char *identifier;
  char *title;
  char *app_id;
};

struct toplevel_list {
  struct ext_foreign_toplevel_list_v1 *toplevel_list;
  struct wl_list                       toplevels;
};

bool toplevel_list_init (struct toplevel_list *tl);
void toplevel_list_destroy (struct toplevel_list *tl);
