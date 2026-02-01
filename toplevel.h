#pragma once

#include "ext-foreign-toplevel-list-v1-protocol.h"
#include "global.h"

struct toplevel {
  struct ext_foreign_toplevel_handle_v1 *handle;

  struct wl_list link;

  bool done;

  char *identifier;
  char *title;
  char *app_id;
};
