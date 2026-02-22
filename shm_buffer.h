#pragma once

#include "global.h"

struct shm_buffer {
  struct wl_buffer *buffer;
  void             *data;

  int32_t width, height, stride;
};

void shm_buffer_registry_global (void *data, struct wl_registry *registry,
                                 uint32_t name, const char *interface,
                                 uint32_t version);
void shm_buffer_registry_global_remove (void               *data,
                                        struct wl_registry *registry,
                                        uint32_t            name);

void shm_buffer_init (struct shm_buffer *sb, struct wl_shm *shm,
                      enum wl_shm_format format, int32_t width, int32_t height,
                      int32_t stride);
void shm_buffer_destroy (struct shm_buffer *sb);
