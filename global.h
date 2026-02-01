#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-util.h>

struct state {
  struct ext_foreign_toplevel_list_v1 *toplevel_list;

  struct wl_list toplevels;
};
