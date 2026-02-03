#include "image_copy.h"

static void
image_copy_capture_frame_ready (void                                   *data,
                                struct ext_image_copy_capture_frame_v1 *frame) {
  struct image_copy_frame *icf = data;
  icf->ready                   = true;
}

static void
image_copy_capture_frame_failed (void                                   *data,
                                 struct ext_image_copy_capture_frame_v1 *frame,
                                 uint32_t reason) {
  struct image_copy_frame *icf = data;
  icf->failed                  = true;
}

static const struct ext_image_copy_capture_frame_v1_listener
    image_copy_capture_frame_listener = {
        .ready  = image_copy_capture_frame_ready,
        .failed = image_copy_capture_frame_failed,
};

struct image_copy_frame *
image_copy_from_toplevel (struct image_copy                     *ic,
                          struct ext_foreign_toplevel_handle_v1 *handle) {
  struct image_copy_frame *icf = calloc (1, sizeof (*icf));

  struct ext_image_capture_source_v1 *source =
      ext_foreign_toplevel_image_capture_source_manager_v1_create_source (
          ic->toplevel_source_manager, handle);

  icf->session = ext_image_copy_capture_manager_v1_create_session (
      ic->image_copy_manager, source, false);
  icf->frame = ext_image_copy_capture_session_v1_create_frame (icf->session);

  ext_image_copy_capture_frame_v1_add_listener (
      icf->frame, &image_copy_capture_frame_listener, icf);
  ext_image_capture_source_v1_destroy (source);

  return icf;
}

void image_copy_init (struct image_copy_frame *icf, struct wl_buffer *buffer) {
  icf->buffer = buffer;
  icf->ready  = false;
  icf->failed = false;

  ext_image_copy_capture_frame_v1_attach_buffer (icf->frame, buffer);
  ext_image_copy_capture_frame_v1_damage_buffer (icf->frame, 0, 0, INT32_MAX,
                                                 INT32_MAX);
  ext_image_copy_capture_frame_v1_capture (icf->frame);
}

void image_copy_destroy (struct image_copy_frame *icf) {
  ext_image_copy_capture_frame_v1_destroy (icf->frame);
  ext_image_copy_capture_session_v1_destroy (icf->session);

  free (icf);
}
