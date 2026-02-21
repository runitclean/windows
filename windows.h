#pragma once

#include "cairo_draw.h"
#include "expose_algorithm.h"
#include "global.h"
#include "image_copy.h"
#include "input_device.h"
#include "shm_buffer.h"
#include "toplevel_list.h"
#include "xdg_shell.h"

struct windows {
  struct input_device     *id;
  struct toplevel_list    *tl;
  struct image_copy       *ic;
  struct xdg_shell        *xs;
  struct expose_algorithm *ea;
  struct shm_buffer       *sb;
  struct cairo_draw       *cd;

  struct wl_display  *display;
  struct wl_registry *registry;
  struct wl_shm      *shm;
  struct wl_callback *callback;

  struct wl_list windows;

  bool render;
};

struct windows_state {
  struct wl_list link;

  bool  error;
  char *identifier, *title, *app_id;

  struct shm_buffer *sb;
};
