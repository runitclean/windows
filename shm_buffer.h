#pragma once

#include "global.h"

struct shm_buffer {
  struct wl_buffer *buffer;
  void             *data;

  int32_t width, height, stride;

  enum wl_shm_format format;
};

struct shm_buffer *shm_buffer_init (struct wl_shm     *shm,
                                    enum wl_shm_format format, int32_t width,
                                    int32_t height, int32_t stride);
void               shm_buffer_destory (struct shm_buffer *sb);
