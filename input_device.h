#pragma once

#include "global.h"

struct input_device {
  struct wl_seat     *seat;
  struct wl_pointer  *pointer;
  struct wl_keyboard *keyboard;

  struct xkb_context *context;
  struct xkb_keymap  *keymap;
  struct xkb_state   *state;

  int32_t       repeat_timer, repeat_rate, repeat_delay;
  xkb_keycode_t repeat_key;

  void (*escape) (void);
  void (*left) (void);
  void (*right) (void);
  void (*up) (void);
  void (*down) (void);
  void (*enter) (void);
};

void input_device_registry_global (void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version);
void input_device_registry_global_remove (void               *data,
                                          struct wl_registry *registry,
                                          uint32_t            name);

void input_device_init (struct input_device *id);
void input_device_destroy (struct input_device *id);

void input_device_dispatch (struct input_device *id);
