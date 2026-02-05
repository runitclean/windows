#pragma once

#include "ext-foreign-toplevel-list-v1-protocol.h"
#include "ext-image-capture-source-v1-protocol.h"
#include "ext-image-copy-capture-v1-protocol.h"
#include "global.h"

struct image_copy {
  struct ext_foreign_toplevel_image_capture_source_manager_v1
      *foreign_toplevel_image_capture_source_manager;

  struct ext_image_copy_capture_manager_v1 *image_copy_capture_manager;
};

struct image_copy_frame {
  struct ext_image_copy_capture_frame_v1   *frame;
  struct ext_image_copy_capture_session_v1 *session;

  struct wl_buffer *buffer;

  uint32_t width, height, shm_format;

  bool done, stopped, ready, failed;
};

void image_copy_registry_global (void *data, struct wl_registry *registry,
                                 uint32_t name, const char *interface,
                                 uint32_t version);
void image_copy_registry_global_remove (void               *data,
                                        struct wl_registry *registry,
                                        uint32_t            name);

struct image_copy_frame *
image_copy_frame_from_toplevel (struct image_copy                     *ic,
                                struct ext_foreign_toplevel_handle_v1 *handle);

void image_copy_init (struct image_copy_frame *icf, struct wl_buffer *buffer);
void image_copy_destroy (struct image_copy_frame *icf);
