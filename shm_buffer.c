#include "shm_buffer.h"

static void shm_buffer_name (char *buffer) {
  struct timespec time;
  clock_gettime (CLOCK_MONOTONIC, &time);

  long seed = time.tv_nsec;

  for (int i = 0; i < 6; ++i) {
    buffer[i]   = 'A' + (seed & 15) + (seed & 16) * 2;
    seed      >>= 5;
  }
}

static int shm_buffer_open (void) {
  char name[]  = "/windows-XXXXXX";
  int  retries = 100;

  do {
    shm_buffer_name (name + strlen (name) - 6);

    int fd = shm_open (name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (fd >= 0) {
      shm_unlink (name);
      return fd;
    }
  } while (--retries > 0);

  return -1;
}

static int shm_buffer_create (off_t size) {
  int fd = shm_buffer_open ();

  if (fd < 0)
    return -1;

  if (ftruncate (fd, size) < 0) {
    close (fd);
    return -1;
  }

  return fd;
}

void shm_buffer_registry_global (void *data, struct wl_registry *registry,
                                 uint32_t name, const char *interface,
                                 uint32_t version) {
  struct shm_buffer *sb = data;

  if (strcmp (interface, wl_shm_interface.name) == 0)
    sb->shm = wl_registry_bind (registry, name, &wl_shm_interface, 1);
}

void shm_buffer_registry_global_remove (void               *data,
                                        struct wl_registry *registry,
                                        uint32_t            name) {}

void shm_buffer_init (struct shm_buffer *sb) {
  // only allocate one global shm_buffer_object for rendering
  sb->sbo = calloc (1, sizeof (*sb->sbo));
}

void shm_buffer_destroy (struct shm_buffer *sb) {
  wl_shm_destroy (sb->shm);
  free (sb->sbo);
}

void shm_buffer_new (struct shm_buffer_object *sbo, struct wl_shm *shm,
                     enum wl_shm_format format, int32_t width, int32_t height) {
  // maybe use cairo_format_stride_for_width?
  int32_t stride = width * 4;
  size_t  size   = height * stride;

  int fd = shm_buffer_create (size);
  if (fd == -1)
    return;

  void *data = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close (fd);
    return;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool (shm, fd, size);
  struct wl_buffer   *buffer =
    wl_shm_pool_create_buffer (pool, 0, width, height, stride, format);

  wl_shm_pool_destroy (pool);
  close (fd);

  sbo->buffer = buffer;
  sbo->width  = width;
  sbo->height = height;
  sbo->stride = stride;
  sbo->data   = data;
}

void shm_buffer_delete (struct shm_buffer_object *sbo) {
  munmap (sbo->data, sbo->height * sbo->stride);
  wl_buffer_destroy (sbo->buffer);
}
