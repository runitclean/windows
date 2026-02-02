#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-util.h>

struct state {
  struct ext_foreign_toplevel_list_v1      *toplevel_list;
  struct ext_image_copy_capture_manager_v1 *image_copy_manager;
  struct ext_foreign_toplevel_image_capture_source_manager_v1
      *toplevel_source_manager;

  struct wl_list toplevels;
};
