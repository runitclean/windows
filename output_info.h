#pragma once

#include "global.h"

struct output_info {
  struct wl_list outputs;
};

struct output_info_object {
  struct wl_list link;

  struct wl_output *output;

  int32_t  width, height;
  uint32_t name;

  char *monitor;
};

void output_info_registry_global (void *data, struct wl_registry *registry,
                                  uint32_t name, const char *interface,
                                  uint32_t version);
void output_info_registry_global_remove (void               *data,
                                         struct wl_registry *registry,
                                         uint32_t            name);

void output_info_init (struct output_info *oi);
void output_info_destroy (struct output_info *oi);
