#include "expose_algorithm.h"

static void expose_algorithm_phantom (struct expose_algorithm_window *eaw,
                                      uint32_t window_count) {
  for (int32_t i = 0; i < window_count; i++) {
    eaw[i].phantom_width  = eaw[i].width + 30;
    eaw[i].phantom_height = eaw[i].height + 30;
  }
}

static int32_t expose_algorithm_nfdh (int32_t shelf_width,
                                      struct expose_algorithm_window *eaw,
                                      uint32_t window_count) {
  int32_t current_x = 0, current_y = 0, current_shelf_height = 0;

  for (int32_t i = 0; i < window_count; i++) {
    eaw[i].x = current_x;
    eaw[i].y = current_y;

    if (current_x + eaw[i].phantom_width > shelf_width) {
      eaw[i].x = current_x = 0;
      eaw[i].y = current_y += current_shelf_height;
      current_shelf_height  = 0;
    }

    current_x += eaw[i].phantom_width;

    if (eaw[i].phantom_height > current_shelf_height)
      current_shelf_height = eaw[i].phantom_height;
  }

  return current_y + current_shelf_height;
}

static int32_t expose_algorithm_compare (const void *a, const void *b) {
  struct expose_algorithm_window *w1 = (struct expose_algorithm_window *) a;
  struct expose_algorithm_window *w2 = (struct expose_algorithm_window *) b;

  if (w1->height != w2->height)
    return w2->height - w1->height;

  if (w1->width != w2->width)
    return w2->width - w1->width;

  return w2->node - w1->node;
}

static struct expose_algorithm_shelf
expose_algorithm_trial (struct expose_algorithm *ea) {
  qsort (ea->eaw, ea->window_count, sizeof (*ea->eaw),
         expose_algorithm_compare);

  const float target_ratio =
      (float) ea->display_height / (float) ea->display_width;

  int32_t shelf_width_min, shelf_width_max, shelf_height_min, shelf_height_max;
  shelf_width_min = shelf_width_max = 0;

  for (int i = 0; i < ea->window_count; i++) {
    shelf_width_min  = fmax (shelf_width_min, ea->eaw[i].phantom_width);
    shelf_width_max += ea->eaw[i].phantom_width;
  }

  struct expose_algorithm_shelf eas;

  shelf_height_max =
      expose_algorithm_nfdh (shelf_width_min, ea->eaw, ea->window_count);

  float ratio_max = (float) shelf_height_max / (float) shelf_width_min;

  if (ratio_max <= target_ratio) {
    eas.width  = shelf_width_min;
    eas.height = shelf_height_max;
    return eas;
  }

  shelf_height_min =
      expose_algorithm_nfdh (shelf_width_max, ea->eaw, ea->window_count);

  float ratio_min = (float) shelf_height_min / shelf_width_max;

  if (ratio_min >= target_ratio) {
    eas.width  = shelf_width_max;
    eas.height = shelf_height_min;
    return eas;
  }

  uint32_t binary_shelf_width, binary_shelf_height;
  float    binary_ratio;

  while ((float) shelf_width_max / (float) shelf_width_min > 1.2) {
    binary_shelf_width = sqrt (shelf_width_min * shelf_width_max);
    binary_shelf_height =
        expose_algorithm_nfdh (binary_shelf_width, ea->eaw, ea->window_count);

    binary_ratio = (float) binary_shelf_height / (float) binary_shelf_width;

    if (binary_ratio > target_ratio) {
      ratio_max        = binary_ratio;
      shelf_width_min  = binary_shelf_width;
      shelf_height_max = binary_shelf_height;
    } else {
      ratio_min        = binary_ratio;
      shelf_width_max  = binary_shelf_width;
      shelf_height_min = binary_shelf_height;
    }
  }

  if (ratio_max - target_ratio < target_ratio - ratio_min)
    eas.width = shelf_width_min;
  else
    eas.width = shelf_width_max;

  eas.height = expose_algorithm_nfdh (eas.width, ea->eaw, ea->window_count);

  return eas;
}

static void expose_algorithm_pack (struct expose_algorithm_shelf eas,
                                   struct expose_algorithm      *ea) {
  float width_ratio   = (float) ea->display_width / (float) eas.width;
  float height_ratio  = (float) ea->display_height / (float) eas.height;
  float scale_factor  = width_ratio < height_ratio ? width_ratio : height_ratio;
  scale_factor       *= 0.9;

  uint32_t strip_width, strip_height, strip_baseline, strip_index;
  strip_width = strip_height = strip_baseline = strip_index = 0;

  for (uint32_t i = 0; i < ea->window_count; i++) {
    struct expose_algorithm_window eaw = ea->eaw[i];

    if (eaw.y == strip_baseline) {
      if (eaw.x + eaw.phantom_width > strip_width)
        strip_width = eaw.x + eaw.phantom_width;

      if (eaw.phantom_height > strip_height)
        strip_height = eaw.phantom_height;
    } else {
      for (uint32_t j = strip_index; j < i; j++) {
        ea->eaw[j].x += (eas.width - strip_width) / 2;
        ea->eaw[j].y += (strip_height - ea->eaw[j].phantom_height) / 2;
      }

      strip_width    = eaw.x + eaw.phantom_width;
      strip_height   = eaw.phantom_height;
      strip_baseline = eaw.y;
      strip_index    = i;
    }

    if (i == ea->window_count - 1) {
      for (uint32_t j = strip_index; j <= i; j++) {
        ea->eaw[j].x += (eas.width - strip_width) / 2;
        ea->eaw[j].y += (strip_height - ea->eaw[j].phantom_height) / 2;
      }
    }
  }

  for (uint32_t i = 0; i < ea->window_count; i++) {
    struct expose_algorithm_window eaw = ea->eaw[i];

    eaw.scale_factor = scale_factor;

    eaw.x += 15;
    eaw.x *= eaw.scale_factor;

    eaw.y += 15;
    eaw.y *= eaw.scale_factor;
  }
}

void expose_algorithm_init (struct expose_algorithm *ea) {
  expose_algorithm_phantom (ea->eaw, ea->window_count);
  expose_algorithm_pack (expose_algorithm_trial (ea), ea);
}
