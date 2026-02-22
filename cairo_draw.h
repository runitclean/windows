#pragma once

#include "global.h"

struct cairo_draw {
  cairo_surface_t *surface;
  cairo_t         *context;
};

void cairo_draw_init (struct cairo_draw *cd, void *data, int32_t width,
                      int32_t height, int32_t stride);
void cairo_draw_destroy (struct cairo_draw *cd);

void cairo_draw_window (struct cairo_draw *cd, void *data, int32_t width,
                        int32_t height, int32_t stride, int32_t x, int32_t y,
                        float scale, bool focused);
void cairo_draw_clear (struct cairo_draw *cd);
