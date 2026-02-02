#include "image_copy.h"

static void ext_image_copy_capture_frame_handle_ready (
    void *data, struct ext_image_copy_capture_frame_v1 *frame) {
  struct image_copy_frame *f = data;
  f->ready                   = true;
}

static void ext_image_copy_capture_frame_handle_failed (
    void *data, struct ext_image_copy_capture_frame_v1 *frame,
    uint32_t reason) {
  struct image_copy_frame *f = data;
  f->failed                  = true;
}

static const struct ext_image_copy_capture_frame_v1_listener frame_listener = {
    .ready  = ext_image_copy_capture_frame_handle_ready,
    .failed = ext_image_copy_capture_frame_handle_failed,
};

struct image_copy_frame *
image_copy_from_toplevel (struct state                          *s,
                          struct ext_foreign_toplevel_handle_v1 *handle) {
  struct image_copy_frame *f = calloc (1, sizeof (*f));

  struct ext_image_capture_source_v1 *source =
      ext_foreign_toplevel_image_capture_source_manager_v1_create_source (
          s->toplevel_source_manager, handle);

  f->session = ext_image_copy_capture_manager_v1_create_session (
      s->image_copy_manager, source, NULL);
  f->frame = ext_image_copy_capture_session_v1_create_frame (f->session);

  ext_image_copy_capture_frame_v1_add_listener (f->frame, &frame_listener, f);
  ext_image_capture_source_v1_destroy (source);

  return f;
}

void image_copy_init (struct image_copy_frame *f, struct wl_buffer *buffer) {
  f->buffer = buffer;
  f->ready  = false;
  f->failed = false;

  ext_image_copy_capture_frame_v1_attach_buffer (f->frame, buffer);
  ext_image_copy_capture_frame_v1_damage_buffer (f->frame, 0, 0, INT32_MAX,
                                                 INT32_MAX);
  ext_image_copy_capture_frame_v1_capture (f->frame);
}

void image_copy_destroy (struct image_copy_frame *f) {
  ext_image_copy_capture_frame_v1_destroy (f->frame);
  ext_image_copy_capture_session_v1_destroy (f->session);

  free (f);
}
