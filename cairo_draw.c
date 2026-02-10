#include "cairo_draw.h"

struct cairo_draw *cairo_draw_init (void *data, int32_t width, int32_t height,
                                    int32_t stride) {
  struct cairo_draw *cd = calloc (1, sizeof (*cd));

  cd->surface = cairo_image_surface_create_for_data (data, CAIRO_FORMAT_ARGB32,
                                                     width, height, stride);
  cd->context = cairo_create (cd->surface);

  cairo_set_source_rgba (cd->context, 0, 0, 0, 1);
  cairo_paint (cd->context);

  return cd;
}

void cairo_draw_destroy (struct cairo_draw *cd) {
  cairo_destroy (cd->context);
  cairo_surface_destroy (cd->surface);
  free (cd);
}

void cairo_draw_window (struct cairo_draw *cd, void *data, int32_t width,
                        int32_t height, int32_t stride, int32_t x, int32_t y,
                        float scale, bool focused) {
  cairo_surface_t *surface = cairo_image_surface_create_for_data (
      data, CAIRO_FORMAT_ARGB32, width, height, stride);

  cairo_save (cd->context);

  cairo_translate (cd->context, x, y);
  cairo_scale (cd->context, scale, scale);

  cairo_set_source_surface (cd->context, surface, 0, 0);
  cairo_paint (cd->context);

  if (focused) {
    cairo_set_source_rgba (cd->context, 1, 1, 1, 1);
    cairo_set_line_width (cd->context, 2 / scale);
    cairo_rectangle (cd->context, -2 / scale, -2 / scale, width + 4 / scale,
                     height + 4 / scale);
    cairo_stroke (cd->context);
  }

  cairo_restore (cd->context);
  cairo_surface_destroy (surface);
}
