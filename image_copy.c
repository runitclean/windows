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

static void image_copy_capture_session_buffer_size (
    void *data, struct ext_image_copy_capture_session_v1 *session,
    uint32_t width, uint32_t height) {
  struct image_copy_frame *icf = data;
  icf->width                   = width;
  icf->height                  = height;
}

static void image_copy_capture_session_shm_format (
    void *data, struct ext_image_copy_capture_session_v1 *session,
    uint32_t format) {
  struct image_copy_frame *icf = data;

  // all renderers should support argb8888
  if (format == WL_SHM_FORMAT_ARGB8888)
    icf->shm_format = format;
}

static void image_copy_capture_session_dmabuf_device (
    void *data, struct ext_image_copy_capture_session_v1 *session,
    struct wl_array *device) {}

static void image_copy_capture_session_dmabuf_format (
    void *data, struct ext_image_copy_capture_session_v1 *session,
    uint32_t format, struct wl_array *modifiers) {}

static void image_copy_capture_session_done (
    void *data, struct ext_image_copy_capture_session_v1 *session) {
  struct image_copy_frame *icf = data;
  icf->done                    = true;
  icf->frame = ext_image_copy_capture_session_v1_create_frame (session);

  ext_image_copy_capture_frame_v1_add_listener (
      icf->frame, &image_copy_capture_frame_listener, icf);
}

static void image_copy_capture_session_stopped (
    void *data, struct ext_image_copy_capture_session_v1 *session) {
  struct image_copy_frame *icf = data;
  icf->stopped                 = true;
}

static const struct ext_image_copy_capture_session_v1_listener
    image_copy_capture_session_listener = {
        .buffer_size   = image_copy_capture_session_buffer_size,
        .shm_format    = image_copy_capture_session_shm_format,
        .dmabuf_device = image_copy_capture_session_dmabuf_device,
        .dmabuf_format = image_copy_capture_session_dmabuf_format,
        .done          = image_copy_capture_session_done,
        .stopped       = image_copy_capture_session_stopped,
};

void image_copy_registry_global (void *data, struct wl_registry *registry,
                                 uint32_t name, const char *interface,
                                 uint32_t version) {
  struct image_copy *ic = data;

  if (strcmp (interface,
              ext_foreign_toplevel_image_capture_source_manager_v1_interface
                  .name) == 0)
    ic->foreign_toplevel_image_capture_source_manager = wl_registry_bind (
        registry, name,
        &ext_foreign_toplevel_image_capture_source_manager_v1_interface, 1);
  else if (strcmp (interface,
                   ext_image_copy_capture_manager_v1_interface.name) == 0)
    ic->image_copy_capture_manager = wl_registry_bind (
        registry, name, &ext_image_copy_capture_manager_v1_interface, 1);
}

void image_copy_registry_global_remove (void               *data,
                                        struct wl_registry *registry,
                                        uint32_t            name) {}

struct image_copy_frame *
image_copy_frame_from_toplevel (struct image_copy                     *ic,
                                struct ext_foreign_toplevel_handle_v1 *handle) {
  struct image_copy_frame *icf = calloc (1, sizeof (*icf));

  struct ext_image_capture_source_v1 *source =
      ext_foreign_toplevel_image_capture_source_manager_v1_create_source (
          ic->foreign_toplevel_image_capture_source_manager, handle);

  icf->session = ext_image_copy_capture_manager_v1_create_session (
      ic->image_copy_capture_manager, source, false);
  ext_image_copy_capture_session_v1_add_listener (
      icf->session, &image_copy_capture_session_listener, icf);

  ext_image_capture_source_v1_destroy (source);

  return icf;
}

void image_copy_init (struct image_copy_frame *icf, struct wl_buffer *buffer) {
  icf->buffer = buffer;

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
