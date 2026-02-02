#pragma once

#include "ext-foreign-toplevel-list-v1-protocol.h"
#include "ext-image-capture-source-v1-protocol.h"
#include "ext-image-copy-capture-v1-protocol.h"
#include "global.h"

struct image_copy_frame {
  struct ext_image_copy_capture_frame_v1   *frame;
  struct ext_image_copy_capture_session_v1 *session;

  struct wl_buffer *buffer;

  bool ready, failed;
};

struct image_copy_frame *
image_copy_from_toplevel (struct state                          *s,
                          struct ext_foreign_toplevel_handle_v1 *handle);

void image_copy_init (struct image_copy_frame *f, struct wl_buffer *buffer);
void image_copy_destroy (struct image_copy_frame *f);
