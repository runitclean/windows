#include "shm_buffer.h"

static void shm_buffer_name (char *buffer) {
  struct timespec time;
  clock_gettime (CLOCK_REALTIME, &time);

  long seed = time.tv_nsec;

  for (int i = 0; i < 6; ++i) {
    buffer[i]   = 'A' + (seed & 15) + (seed & 16) * 2;
    seed      >>= 5;
  }
}

static int shm_buffer_open (void) {
  char *name    = "/windows-XXXXXX";
  int   retries = 100;

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
    return fd;

  if (ftruncate (fd, size) < 0) {
    close (fd);
    return -1;
  }

  return fd;
}

void shm_buffer_registry_global (void *data, struct wl_registry *registry,
                                 uint32_t name, const char *interface,
                                 uint32_t version) {
  struct wl_shm *shm = data;

  if (strcmp (interface, wl_shm_interface.name) == 0)
    shm = wl_registry_bind (registry, name, &wl_shm_interface, 1);
}

void shm_buffer_registry_global_remove (void               *data,
                                        struct wl_registry *registry,
                                        uint32_t            name) {}

struct shm_buffer *shm_buffer_init (struct wl_shm     *shm,
                                    enum wl_shm_format format, int32_t width,
                                    int32_t height, int32_t stride) {
  size_t size = height * stride;

  int fd = shm_buffer_create (size);
  if (fd == -1)
    return NULL;

  void *data = mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (data == MAP_FAILED) {
    close (fd);
    return NULL;
  }

  struct wl_shm_pool *pool = wl_shm_create_pool (shm, fd, size);
  struct wl_buffer   *buffer =
      wl_shm_pool_create_buffer (pool, 0, width, height, stride, format);

  wl_shm_pool_destroy (pool);
  close (fd);

  struct shm_buffer *sb = calloc (1, sizeof (struct shm_buffer));

  sb->buffer = buffer;
  sb->data   = data;
  sb->width  = width;
  sb->height = height;
  sb->stride = stride;
  sb->format = format;

  return sb;
}

void shm_buffer_destory (struct shm_buffer *sb) {
  if (sb == NULL)
    return;

  munmap (sb->data, sb->height * sb->stride);
  wl_buffer_destroy (sb->buffer);
  free (sb);
}
