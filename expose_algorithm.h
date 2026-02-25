#pragma once

#include "global.h"

#define window_margin 100

struct expose_algorithm {
  struct expose_algorithm_window *eaw;

  int32_t  width, height;
  uint32_t window_count;
};

struct expose_algorithm_shelf {
  int32_t width, height;
};

struct expose_algorithm_window {
  bool     focused;
  int32_t  width, height, phantom_width, phantom_height, x, y;
  uint32_t index, left, right, up, down;
  float    scale;

  void *data;
};

void expose_algorithm_init (struct expose_algorithm *ea, int32_t width,
                            int32_t height, int32_t window_count);
void expose_algorithm_destroy (struct expose_algorithm *ea);

void expose_algorithm_prepare (struct expose_algorithm *ea);
