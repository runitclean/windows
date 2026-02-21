#include "cairo_draw.h"

void cairo_draw_init (struct cairo_draw *cd, void *data, int32_t width,
                      int32_t height, int32_t stride) {
  cd->surface = cairo_image_surface_create_for_data (data, CAIRO_FORMAT_ARGB32,
                                                     width, height, stride);
  cd->context = cairo_create (cd->surface);

  cairo_set_source_rgba (cd->context, 0, 0, 0, 1);
  cairo_paint (cd->context);
}

void cairo_draw_destroy (struct cairo_draw *cd) {
  cairo_destroy (cd->context);
  cairo_surface_destroy (cd->surface);
}

void cairo_draw_window (struct cairo_draw *cd, void *data, int32_t width,
                        int32_t height, int32_t stride, int32_t x, int32_t y,
                        float scale_factor, bool focused) {
  cairo_surface_t *surface = cairo_image_surface_create_for_data (
    data, CAIRO_FORMAT_ARGB32, width, height, stride);

  cairo_save (cd->context);

  cairo_translate (cd->context, x, y);
  cairo_scale (cd->context, scale_factor, scale_factor);

  cairo_set_source_surface (cd->context, surface, 0, 0);
  cairo_paint (cd->context);

  if (focused) {
    cairo_set_source_rgba (cd->context, 1, 1, 1, 1);
    cairo_set_line_width (cd->context, 1 / scale_factor);
    cairo_rectangle (cd->context, -1 / scale_factor, -1 / scale_factor,
                     width + 2 / scale_factor, height + 2 / scale_factor);
    cairo_stroke (cd->context);
  }

  cairo_restore (cd->context);
  cairo_surface_destroy (surface);
}

void cairo_draw_clear (struct cairo_draw *cd) {
  cairo_save (cd->context);
  cairo_set_operator (cd->context, CAIRO_OPERATOR_CLEAR);
  cairo_paint (cd->context);
  cairo_restore (cd->context);
}
