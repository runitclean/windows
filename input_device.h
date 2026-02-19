#pragma once

#include "global.h"

struct input_device {
  struct wl_seat     *seat;
  struct wl_pointer  *pointer;
  struct wl_keyboard *keyboard;
};

void input_device_registry_global (void *data, struct wl_registry *registry,
                                   uint32_t name, const char *interface,
                                   uint32_t version);
void input_device_registry_global_remove (void               *data,
                                          struct wl_registry *registry,
                                          uint32_t            name);

void input_device_init (struct input_device *id);
void input_device_destroy (struct input_device *id);
