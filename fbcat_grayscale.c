#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linux/fb.h>

#if !defined(le32toh) || !defined(le16toh)

#if BYTE_ORDER == LITTLE_ENDIAN
#define le32toh(x) (x)
#define le16toh(x) (x)
#else
#include <byteswap.h>
#define le32toh(x) bswap_32(x)
#define le16toh(x) bswap_16(x)
#endif

#endif

static const char default_fbdev[] = "/dev/fb0";

static const char bug_tracker_url[] = "https://github.com/jwilk/fbcat/issues";

static void posix_error(const char *s, ...)
{
  va_list argv;
  va_start(argv, s);
  fprintf(stderr, "fbcat: ");
  vfprintf(stderr, s, argv);
  fprintf(stderr, ": ");
  perror(NULL);
  va_end(argv);
  exit(2);
}

static void not_supported(const char *s)
{
  fprintf(stderr,
    "fbcat: not yet supported: %s\n"
    "Please file a bug at <%s>.\n",
    s,
    bug_tracker_url
  );
  exit(3);
}

static inline unsigned char get_grayscale(unsigned int pixel, const struct fb_var_screeninfo *info, const struct fb_cmap *colormap) {
  unsigned char r = colormap->red[(pixel >> info->red.offset) & ((1 << info->red.length) - 1)] >> 8;
  unsigned char g = colormap->green[(pixel >> info->green.offset) & ((1 << info->green.length) - 1)] >> 8;
  unsigned char b = colormap->blue[(pixel >> info->blue.offset) & ((1 << info->blue.length) - 1)] >> 8;
  return (unsigned char)(0.3 * r + 0.59 * g + 0.11 * b);
}

static void dump_video_memory_grayscale(
  const unsigned char *video_memory,
  const struct fb_var_screeninfo *info,
  const struct fb_cmap *colormap,
  unsigned int line_length,
  FILE *fp
)
{
  unsigned int x, y;
  const unsigned int bytes_per_pixel = (info->bits_per_pixel + 7) / 8;
  unsigned char *row = (unsigned char *)malloc(info->xres);
  if (row == NULL)
    posix_error("malloc failed");
  assert(row != NULL);

  fprintf(fp, "P5 %" PRIu32 " %" PRIu32 " 255\n", info->xres, info->yres);
  for (y = 0; y < info->yres; y++)
  {
    const unsigned char *current;
    current = video_memory + (y + info->yoffset) * line_length + info->xoffset * bytes_per_pixel;
    for (x = 0; x < info->xres; x++)
    {
      unsigned int i;
      unsigned int pixel = 0;
      switch (bytes_per_pixel)
      {
        case 4:
          pixel = le32toh(*((uint32_t *) current));
          current += 4;
          break;
        case 2:
          pixel = le16toh(*((uint16_t *) current));
          current += 2;
          break;
        default:
          for (i = 0; i < bytes_per_pixel; i++)
          {
            pixel |= current[0] << (i * 8);
            current++;
          }
          break;
      }
      row[x] = get_grayscale(pixel, info, colormap);
    }
    if (fwrite(row, 1, info->xres, fp) != info->xres)
      posix_error("write error");
  }

  free(row);
}

int main(int argc, const char **argv)
{
  const char *fbdev_name;
  int fd;

  bool show_usage = false;
  if (isatty(STDOUT_FILENO))
  {
    fprintf(stderr, "fbcat: refusing to write binary data to a terminal\n");
    show_usage = true;
  }
  if (argc > 2)
    show_usage = true;
  if (show_usage)
  {
    fprintf(stderr, "Usage: %s [fbdev]\n", argv[0]);
    return 1;
  }

  if (argc == 2)
    fbdev_name = argv[1];
  else
  {
    fbdev_name = getenv("FRAMEBUFFER");
    if (fbdev_name == NULL || fbdev_name[0] == '\0')
      fbdev_name = default_fbdev;
  }

  fd = open(fbdev_name, O_RDONLY);
  if (fd == -1)
    posix_error("could not open %s", fbdev_name);

  struct fb_fix_screeninfo fix_info;
  struct fb_var_screeninfo var_info;
  uint16_t colormap_data[4][1 << 8];
  struct fb_cmap colormap =
  {
    0,
    1 << 8,
    colormap_data[0],
    colormap_data[1],
    colormap_data[2],
    colormap_data[3],
  };

  if (ioctl(fd, FBIOGET_FSCREENINFO, &fix_info))
    posix_error("FBIOGET_FSCREENINFO failed");

  if (fix_info.type != FB_TYPE_PACKED_PIXELS)
    not_supported("framebuffer type is not PACKED_PIXELS");

  if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info))
    posix_error("FBIOGET_VSCREENINFO failed");

  if (var_info.red.length > 8 || var_info.green.length > 8 || var_info.blue.length > 8)
    not_supported("color depth > 8 bits per component");

  switch (fix_info.visual)
  {
    case FB_VISUAL_TRUECOLOR:
    {
      unsigned int i;
      for (i = 0; i < (1U << var_info.red.length); i++)
        colormap.red[i] = i * 0xFFFF / ((1 << var_info.red.length) - 1);
      for (i = 0; i < (1U << var_info.green.length); i++)
        colormap.green[i] = i * 0xFFFF / ((1 << var_info.green.length) - 1);
      for (i = 0; i < (1U << var_info.blue.length); i++)
        colormap.blue[i] = i * 0xFFFF / ((1 << var_info.blue.length) - 1);
      break;
    }
    case FB_VISUAL_DIRECTCOLOR:
    case FB_VISUAL_PSEUDOCOLOR:
    case FB_VISUAL_STATIC_PSEUDOCOLOR:
      if (ioctl(fd, FBIOGETCMAP, &colormap) != 0)
        posix_error("FBIOGETCMAP failed");
      break;
    default:
      not_supported("unsupported visual");
  }

  if (var_info.bits_per_pixel < 8)
    not_supported("< 8 bpp");

  const size_t mapped_length = fix_info.line_length * (var_info.yres + var_info.yoffset);
  bool mmapped_memory = false;
  unsigned char *video_memory = (unsigned char *)mmap(NULL, mapped_length, PROT_READ, MAP_SHARED, fd, 0);
  if (video_memory != MAP_FAILED)
    mmapped_memory = true;
  else
  {
    mmapped_memory = false;
    const size_t buffer_size = fix_info.line_length * var_info.yres;
    video_memory = (unsigned char *)malloc(buffer_size);
    if (video_memory == NULL)
      posix_error("malloc failed");
    off_t offset = lseek(fd, fix_info.line_length * var_info.yoffset, SEEK_SET);
    if (offset == (off_t) -1)
      posix_error("lseek failed");
    var_info.yoffset = 0;
    ssize_t read_bytes = read(fd, video_memory, buffer_size);
    if (read_bytes < 0)
      posix_error("read failed");
    else if ((size_t)read_bytes != buffer_size)
    {
      errno = EIO;
      posix_error("read failed");
    }
  }

  dump_video_memory_grayscale(video_memory, &var_info, &colormap, fix_info.line_length, stdout);

  if (fclose(stdout))
    posix_error("write error");

  if (mmapped_memory)
    munmap(video_memory, mapped_length);
  else
    free(video_memory);
  close(fd);
  return 0;
}
