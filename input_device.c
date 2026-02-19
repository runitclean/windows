#include "input_device.h"

static void pointer_listener_enter (void *data, struct wl_pointer *pointer,
                                    uint32_t serial, struct wl_surface *surface,
                                    wl_fixed_t surface_x,
                                    wl_fixed_t surface_y) {
  // hide the cursor (at least for now)
  wl_pointer_set_cursor (pointer, serial, NULL, 0, 0);
}

static void pointer_listener_leave (void *data, struct wl_pointer *pointer,
                                    uint32_t           serial,
                                    struct wl_surface *surface) {}

static void pointer_listener_motion (void *data, struct wl_pointer *pointer,
                                     uint32_t time, wl_fixed_t surface_x,
                                     wl_fixed_t surface_y) {}

static void pointer_listener_button (void *data, struct wl_pointer *pointer,
                                     uint32_t serial, uint32_t time,
                                     uint32_t button, uint32_t state) {}

static void pointer_listener_axis (void *data, struct wl_pointer *pointer,
                                   uint32_t time, uint32_t axis,
                                   wl_fixed_t value) {}

static void pointer_listener_frame (void *data, struct wl_pointer *pointer) {}

static void pointer_listener_axis_source (void              *data,
                                          struct wl_pointer *pointer,
                                          uint32_t           axis_source) {}

static void pointer_listener_axis_stop (void *data, struct wl_pointer *pointer,
                                        uint32_t time, uint32_t axis) {}

static void pointer_listener_axis_discrete (void              *data,
                                            struct wl_pointer *pointer,
                                            uint32_t axis, int32_t discrete) {}

static void pointer_listener_axis_value120 (void              *data,
                                            struct wl_pointer *pointer,
                                            uint32_t axis, int32_t value120) {}

static void pointer_listener_axis_relative_direction (
    void *data, struct wl_pointer *pointer, uint32_t axis, uint32_t direction) {
}

static const struct wl_pointer_listener pointer_listener = {
    .enter                   = pointer_listener_enter,
    .leave                   = pointer_listener_leave,
    .motion                  = pointer_listener_motion,
    .button                  = pointer_listener_button,
    .axis                    = pointer_listener_axis,
    .frame                   = pointer_listener_frame,
    .axis_source             = pointer_listener_axis_source,
    .axis_stop               = pointer_listener_axis_stop,
    .axis_discrete           = pointer_listener_axis_discrete,
    .axis_value120           = pointer_listener_axis_value120,
    .axis_relative_direction = pointer_listener_axis_relative_direction,
};

static void seat_listener_capabilities (void *data, struct wl_seat *seat,
                                        uint32_t capabilities) {
  struct input_device *id = data;

  bool pointer, keyboard;

  pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;

  if (pointer && id->pointer == NULL) {
    id->pointer = wl_seat_get_pointer (id->seat);
    wl_pointer_add_listener (id->pointer, &pointer_listener, id);
  } else if (!pointer && id->pointer != NULL) {
    wl_pointer_release (id->pointer);
    id->pointer = NULL;
  }
}

static void seat_listener_name (void *data, struct wl_seat *seat,
                                const char *name) {}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_listener_capabilities,
    .name         = seat_listener_name,
};

void input_device_registry_global (void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version) {
  struct input_device *id = data;

  if (strcmp (interface, wl_seat_interface.name) == 0)
    id->seat = wl_registry_bind (registry, name, &wl_seat_interface, 7);
}

void input_device_registry_global_remove (void               *data,
                                          struct wl_registry *registry,
                                          uint32_t            name) {}

void input_device_init (struct input_device *id) {
  wl_seat_add_listener (id->seat, &seat_listener, id);
}

void input_device_destroy (struct input_device *id) {}
