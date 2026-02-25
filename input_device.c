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
  void *data, struct wl_pointer *pointer, uint32_t axis, uint32_t direction) {}

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

static void input_device_keyboard_dispatch (struct input_device_seat *ids,
                                            xkb_keycode_t             keycode) {
  struct input_device *id = ids->id;

  xkb_keysym_t keysym = xkb_state_key_get_one_sym (ids->state, keycode);

  switch (keysym) {
  case XKB_KEY_Escape:
    if (id->escape)
      id->escape (id->data);
    break;
  case XKB_KEY_Left:
    if (id->left)
      id->left (id->data);
    break;
  case XKB_KEY_Right:
    if (id->right)
      id->right (id->data);
    break;
  case XKB_KEY_Up:
    if (id->up)
      id->up (id->data);
    break;
  case XKB_KEY_Down:
    if (id->down)
      id->down (id->data);
    break;
  case XKB_KEY_Return:
    if (id->enter)
      id->enter (id->data);
    break;
  }
}

static void keyboard_listener_keymap (void *data, struct wl_keyboard *keyboard,
                                      uint32_t format, int32_t fd,
                                      uint32_t size) {
  struct input_device_seat *ids = data;

  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)
    return;

  xkb_state_unref (ids->state);
  xkb_keymap_unref (ids->keymap);

  // from wl_seat version 7 onwards, the fd must be mapped with MAP_PRIVATE by
  // the recipient, as MAP_SHARED may fail
  char *shm = mmap (NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

  ids->keymap = xkb_keymap_new_from_string (
    ids->context, shm, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
  ids->state = xkb_state_new (ids->keymap);

  munmap (shm, size);
  close (fd);
}

static void keyboard_listener_enter (void *data, struct wl_keyboard *keyboard,
                                     uint32_t           serial,
                                     struct wl_surface *surface,
                                     struct wl_array   *keys) {}

static void keyboard_listener_leave (void *data, struct wl_keyboard *keyboard,
                                     uint32_t           serial,
                                     struct wl_surface *surface) {}

static void keyboard_listener_key (void *data, struct wl_keyboard *keyboard,
                                   uint32_t serial, uint32_t time, uint32_t key,
                                   uint32_t state) {
  struct input_device_seat *ids = data;

  ids->id->keyboard = keyboard;

  xkb_keycode_t keycode = key + 8;

  switch (state) {
  case WL_KEYBOARD_KEY_STATE_PRESSED:
    xkb_state_update_key (ids->state, keycode, XKB_KEY_DOWN);
    input_device_keyboard_dispatch (ids, keycode);
    break;
  case WL_KEYBOARD_KEY_STATE_RELEASED:
    xkb_state_update_key (ids->state, keycode, XKB_KEY_UP);
    break;
  case WL_KEYBOARD_KEY_STATE_REPEATED:
    // ``repeat'' pseudo-state available only since version 10
    if (!xkb_keymap_key_repeats (ids->keymap, keycode))
      return;
    input_device_keyboard_dispatch (ids, keycode);
    break;
  }

  if (state == WL_KEYBOARD_KEY_STATE_PRESSED && ids->repeat_rate > 0 &&
      xkb_keymap_key_repeats (ids->keymap, keycode)) {
    struct itimerspec timer = {
      .it_value =
        {
          .tv_sec  = ids->repeat_delay / (int32_t) 1e3,
          .tv_nsec = (ids->repeat_delay % (int32_t) 1e3) * (int32_t) 1e6,
        },
      .it_interval =
        {
          .tv_sec  = 0,
          .tv_nsec = (int32_t) 1e9 / ids->repeat_rate,
        },
    };

    timerfd_settime (ids->id->repeat_timer, 0, &timer, NULL);
    ids->id->repeat_key = keycode;
  }

  if (state == WL_KEYBOARD_KEY_STATE_RELEASED &&
      keycode == ids->id->repeat_key) {
    struct itimerspec timer = {0};

    timerfd_settime (ids->id->repeat_timer, 0, &timer, NULL);
    ids->id->repeat_key = 0;
  }
}

static void keyboard_listener_modifiers (void               *data,
                                         struct wl_keyboard *keyboard,
                                         uint32_t            serial,
                                         uint32_t            mods_depressed,
                                         uint32_t            mods_latched,
                                         uint32_t mods_locked, uint32_t group) {
  struct input_device_seat *ids = data;
  xkb_state_update_mask (ids->state, mods_depressed, mods_latched, mods_locked,
                         0, 0, group);
}

static void keyboard_listener_repeat_info (void               *data,
                                           struct wl_keyboard *keyboard,
                                           int32_t rate, int32_t delay) {
  struct input_device_seat *ids = data;
  ids->repeat_rate              = rate;
  ids->repeat_delay             = delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
  .keymap      = keyboard_listener_keymap,
  .enter       = keyboard_listener_enter,
  .leave       = keyboard_listener_leave,
  .key         = keyboard_listener_key,
  .modifiers   = keyboard_listener_modifiers,
  .repeat_info = keyboard_listener_repeat_info,
};

static void seat_listener_capabilities (void *data, struct wl_seat *seat,
                                        uint32_t capabilities) {
  struct input_device_seat *ids = data;

  bool pointer, keyboard;

  pointer  = capabilities & WL_SEAT_CAPABILITY_POINTER;
  keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

  if (pointer && ids->pointer == NULL) {
    ids->pointer = wl_seat_get_pointer (ids->seat);
    wl_pointer_add_listener (ids->pointer, &pointer_listener, ids);
  } else if (!pointer && ids->pointer != NULL) {
    wl_pointer_release (ids->pointer);
    ids->pointer = NULL;
  }

  if (keyboard && ids->keyboard == NULL) {
    ids->keyboard = wl_seat_get_keyboard (ids->seat);
    wl_keyboard_add_listener (ids->keyboard, &keyboard_listener, ids);
  } else if (!keyboard && ids->keyboard != NULL) {
    wl_keyboard_release (ids->keyboard);
    ids->keyboard = NULL;
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

  if (strcmp (interface, wl_seat_interface.name) == 0) {
    struct wl_seat *seat =
      wl_registry_bind (registry, name, &wl_seat_interface, 7);

    struct input_device_seat *ids = calloc (1, sizeof (*ids));

    ids->seat    = seat;
    ids->id      = id;
    ids->name    = name;
    ids->context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);

    wl_list_insert (&id->seats, &ids->link);

    wl_seat_add_listener (seat, &seat_listener, ids);
  }
}

void input_device_registry_global_remove (void               *data,
                                          struct wl_registry *registry,
                                          uint32_t            name) {
  struct input_device      *id = data;
  struct input_device_seat *ids, *tmp;

  wl_list_for_each_safe (ids, tmp, &id->seats, link) if (ids->name == name) {
    wl_list_remove (&ids->link);

    if (ids->pointer)
      wl_pointer_release (ids->pointer);

    if (ids->keyboard)
      wl_keyboard_release (ids->keyboard);

    wl_seat_release (ids->seat);

    xkb_state_unref (ids->state);
    xkb_keymap_unref (ids->keymap);
    xkb_context_unref (ids->context);

    free (ids);
  }
}

void input_device_init (struct input_device *id) {
  wl_list_init (&id->seats);

  id->repeat_timer = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
  id->repeat_key   = 0;
}

void input_device_destroy (struct input_device *id) {
  struct input_device_seat *ids, *tmp;

  wl_list_for_each_safe (ids, tmp, &id->seats, link) {
    wl_list_remove (&ids->link);

    if (ids->pointer)
      wl_pointer_release (ids->pointer);

    if (ids->keyboard)
      wl_keyboard_release (ids->keyboard);

    wl_seat_release (ids->seat);

    xkb_state_unref (ids->state);
    xkb_keymap_unref (ids->keymap);
    xkb_context_unref (ids->context);

    free (ids);
  }

  close (id->repeat_timer);
}

void input_device_dispatch (struct input_device *id) {
  uint64_t expirations;

  if (!id->repeat_key || read (id->repeat_timer, &expirations,
                               sizeof (expirations)) != sizeof (expirations))
    return;

  struct input_device_seat *ids;

  wl_list_for_each (ids, &id->seats,
                    link) if (id->keyboard ==
                              ids->keyboard) for (uint64_t i = 0;
                                                  i < expirations; i++)
    input_device_keyboard_dispatch (ids, id->repeat_key);
}
