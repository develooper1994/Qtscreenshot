/* Copyright © 2009 Piotr Lewandowski
 * Copyright © 2009-2018 Jakub Wilk
 * Copyright © 2013 David Lechner
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991.
 */

/*
$ fbcat > screenshot.ppm
$ file screenshot.ppm

/opt/poky-dtsis/3.0.4/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc
\ -mthumb -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -fstack-protector-strong
-D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -pipe
--sysroot=/opt/poky-dtsis/3.0.4/sysroots/cortexa9t2hf-neon-poky-linux-gnueabi
-Wall -Wextra -fPIC -g -O2 -D_FILE_OFFSET_BITS=64 -c -o fbcat.o fbcat.c

/opt/poky-dtsis/3.0.4/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-gcc
\ -mthumb -mfpu=neon -mfloat-abi=hard -mcpu=cortex-a9 -fstack-protector-strong
-D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -pipe
--sysroot=/opt/poky-dtsis/3.0.4/sysroots/cortexa9t2hf-neon-poky-linux-gnueabi
-Wall -Wextra -fPIC -g -O2 -D_FILE_OFFSET_BITS=64 fbcat.o -o fbcat

/opt/poky-dtsis/3.0.4/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-strip
-s fbcat
*/

#include "fbcat.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

#include <stdarg.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

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

static inline void posixError(const char *s, ...) {
  va_list argv;
  va_start(argv, s);
  fprintf(stderr, "fbcat: ");
  vfprintf(stderr, s, argv);
  fprintf(stderr, ": ");
  perror(NULL);
  va_end(argv);
  exit(2);
}

static inline void notSupported(const char *s) {
  fprintf(stderr,
          "fbcat: not yet supported: %s\n"
          "Please file a bug at <%s>.\n",
          s, bugTrackerUrl);
  exit(3);
}

static inline unsigned char
getColor(unsigned int pixel, const fb_bitfield *bitfield, uint16_t *colormap) {
  return colormap[(pixel >> bitfield->offset) &
                  ((1 << bitfield->length) - 1)] >>
         8;
}

static inline unsigned char reverseBits(unsigned char b) {
  /* reverses the order of the bits in a byte
   * from
   * https://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64BitsDiv
   *
   * how it works:
   *
   *   w = 0bABCDEFGH
   *   x = w * 0x0202020202
   *     = 0bABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGH0
   *   y = x & 0x010884422010
   *     = 0bABCDEFGHABCDEFGHABCDEFGHABCDEFGHABCDEFGH0
   *     & 0b10000100010000100010000100010000000010000
   *     = 0bA0000F000B0000G000C0000H000D00000000E0000
   *     = (A << 40) + (B << 31) + (C << 22) + (D << 13) + (E << 4) + (F << 35)
   * + (G << 26) + (H << 17) z = y % 1023 = = (A << 0) + (B << 1) + (C << 2) +
   * (D << 3) + (E << 4) + (F << 5) + (G << 6) + (H << 7) = 0bHGFEDCBA
   */
  return (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
}

static inline void dumpVideoMemoryMono(const unsigned char *video_memory,
                                       const vsi *info, bool black_is_zero,
                                       unsigned int line_length, FILE *fp) {
  unsigned int x, y;
  const unsigned int bytes_per_row = (info->xres + 7) / 8;
  unsigned char *row = (unsigned char *)malloc(bytes_per_row);
  if (row == NULL)
    posixError("malloc failed");
  assert(row != NULL);

  if (info->xoffset % 8)
    notSupported("xoffset not divisible by 8 in 1 bpp mode");
  fprintf(fp, "P4 %" PRIu32 " %" PRIu32 "\n", info->xres, info->yres);
  for (y = 0; y < info->yres; y++) {
    const unsigned char *current;
    current =
        video_memory + (y + info->yoffset) * line_length + info->xoffset / 8;
    for (x = 0; x < bytes_per_row; x++) {
      row[x] = reverseBits(*current++);
      if (black_is_zero)
        row[x] = ~row[x];
    }
    if (fwrite(row, 1, bytes_per_row, fp) != bytes_per_row)
      posixError("write error");
  }

  free(row);
}

static inline void dumpVideoMemory(const unsigned char *video_memory,
                                   const vsi *info, const cmap *colormap,
                                   unsigned int line_length, FILE *fp) {
  unsigned int x, y;
  const unsigned int bytes_per_pixel = (info->bits_per_pixel + 7) / 8;
  unsigned char *row = (unsigned char *)malloc(info->xres * 3);
  if (row == NULL)
    posixError("malloc failed");
  assert(row != NULL);

  fprintf(fp, "P6 %" PRIu32 " %" PRIu32 " 255\n", info->xres, info->yres);
  for (y = 0; y < info->yres; y++) {
    const unsigned char *current;
    current = video_memory + (y + info->yoffset) * line_length +
              info->xoffset * bytes_per_pixel;
    for (x = 0; x < info->xres; x++) {
      unsigned int i;
      unsigned int pixel = 0;
      switch (bytes_per_pixel) {
      /* The following code assumes little-endian byte ordering in the frame
       * buffer. */
      case 4:
        pixel = le32toh(*((uint32_t *)current));
        current += 4;
        break;
      case 2:
        pixel = le16toh(*((uint16_t *)current));
        current += 2;
        break;
      default:
        for (i = 0; i < bytes_per_pixel; i++) {
          pixel |= current[0] << (i * 8);
          current++;
        }
        break;
      }
      row[x * 3 + 0] = getColor(pixel, &info->red, colormap->red);
      row[x * 3 + 1] = getColor(pixel, &info->green, colormap->green);
      row[x * 3 + 2] = getColor(pixel, &info->blue, colormap->blue);
    }
    if (fwrite(row, 1, info->xres * 3, fp) != info->xres * 3)
      posixError("write error");
  }

  free(row);
}

static inline int fbcatTest(int argc, const char **argv) {
  // init
  const char *fbdev_name;
  int fd;
  bool show_usage = false, is_mono = false, black_is_zero = false,
       mmapped_memory = false;
  fsi fix_info;
  vsi var_info;
  uint16_t colormap_data[4][1 << 8];
  cmap colormap = {
      0,
      1 << 8,
      colormap_data[0],
      colormap_data[1],
      colormap_data[2],
      colormap_data[3],
  };

  // checks
  if (isatty(STDOUT_FILENO)) {
    fprintf(stderr, "fbcat: refusing to write binary data to a terminal\n");
    show_usage = true;
  }
  if (argc > 2)
    show_usage = true;
  if (show_usage) {
    fprintf(stderr, "Usage: %s [fbdev]\n", argv[0]);
    return 1;
  }

  if (argc == 2)
    fbdev_name = argv[1];
  else {
    fbdev_name = getenv("FRAMEBUFFER");
    if (fbdev_name == NULL || fbdev_name[0] == '\0')
      fbdev_name = defaultFbdev;
  }

  fd = open(fbdev_name, O_RDONLY);
  if (fd == -1)
    posixError("could not open %s", fbdev_name);

  if (ioctl(fd, FBIOGET_FSCREENINFO, &fix_info))
    posixError("FBIOGET_FSCREENINFO failed");

  if (fix_info.type != FB_TYPE_PACKED_PIXELS)
    notSupported("framebuffer type is not PACKED_PIXELS");

  if (ioctl(fd, FBIOGET_VSCREENINFO, &var_info))
    posixError("FBIOGET_VSCREENINFO failed");

  if (var_info.red.length > 8 || var_info.green.length > 8 ||
      var_info.blue.length > 8)
    notSupported("color depth > 8 bits per component");

  // initColormap
  switch (fix_info.visual) {
  case FB_VISUAL_TRUECOLOR: {
    /* initialize dummy colormap */
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
      posixError("FBIOGETCMAP failed");
    break;
  case FB_VISUAL_MONO01:
    is_mono = true;
    break;
  case FB_VISUAL_MONO10:
    is_mono = true;
    black_is_zero = true;
    break;
  default:
    notSupported("unsupported visual");
  }
  if (var_info.bits_per_pixel < 8 && !is_mono)
    notSupported("< 8 bpp");
  if (var_info.bits_per_pixel != 1 && is_mono)
    notSupported("monochrome framebuffer is not 1 bpp");

  // process
  const size_t mapped_length =
      fix_info.line_length * (var_info.yres + var_info.yoffset);
  unsigned char *video_memory =
      (unsigned char *)mmap(NULL, mapped_length, PROT_READ, MAP_SHARED, fd, 0);
  if (video_memory != MAP_FAILED)
    mmapped_memory = true;
  else {
    mmapped_memory = false;
    const size_t buffer_size = fix_info.line_length * var_info.yres;
    video_memory = (unsigned char *)malloc(buffer_size);
    if (video_memory == NULL)
      posixError("malloc failed");
    off_t offset = lseek(fd, fix_info.line_length * var_info.yoffset, SEEK_SET);
    if (offset == (off_t)-1)
      posixError("lseek failed");
    var_info.yoffset = 0;
    ssize_t read_bytes = read(fd, video_memory, buffer_size);
    if (read_bytes < 0)
      posixError("read failed");
    else if ((size_t)read_bytes != buffer_size) {
      errno = EIO;
      posixError("read failed");
    }
  }
  if (is_mono)
    dumpVideoMemoryMono(video_memory, &var_info, black_is_zero,
                        fix_info.line_length, stdout);
  else
    dumpVideoMemory(video_memory, &var_info, &colormap, fix_info.line_length,
                    stdout);

  // close and free
  if (fclose(stdout))
    posixError("write error");

  /* deliberately ignore errors */
  if (mmapped_memory)
    munmap(video_memory, mapped_length);
  else
    free(video_memory);
  close(fd);
  return 0;
}

/* vim:set ts=2 sts=2 sw=2 et: */

FrameBuffer::FrameBuffer() : FrameBuffer(defaultFbdev) {}

FrameBuffer::FrameBuffer(const char *fbdev_name) : fbdevName(fbdev_name) {}

void FrameBuffer::setFbdevFromEnv() {
  fbdevName = getenv("FRAMEBUFFER");
  if (fbdevName == NULL || fbdevName[0] == '\0')
    fbdevName = defaultFbdev;
}

void FrameBuffer::check() {
  if (isatty(STDOUT_FILENO))
    fprintf(stderr, "fbcat: refusing to write binary data to a terminal\n");

  fd = open(fbdevName, O_RDONLY);
  if (fd == -1)
    posixError("could not open %s", fbdevName);

  if (ioctl(fd, FBIOGET_FSCREENINFO, &fixInfo))
    posixError("FBIOGET_FSCREENINFO failed");

  if (fixInfo.type != FB_TYPE_PACKED_PIXELS)
    notSupported("framebuffer type is not PACKED_PIXELS");

  if (ioctl(fd, FBIOGET_VSCREENINFO, &varInfo))
    posixError("FBIOGET_VSCREENINFO failed");

  if (varInfo.red.length > 8 || varInfo.green.length > 8 ||
      varInfo.blue.length > 8)
    notSupported("color depth > 8 bits per component");
}

void FrameBuffer::initColormap() {
  switch (fixInfo.visual) {
  case FB_VISUAL_TRUECOLOR: {
    /* initialize dummy colormap */
    unsigned int i;
    for (i = 0; i < (1U << varInfo.red.length); i++)
      colormap.red[i] = i * 0xFFFF / ((1 << varInfo.red.length) - 1);
    for (i = 0; i < (1U << varInfo.green.length); i++)
      colormap.green[i] = i * 0xFFFF / ((1 << varInfo.green.length) - 1);
    for (i = 0; i < (1U << varInfo.blue.length); i++)
      colormap.blue[i] = i * 0xFFFF / ((1 << varInfo.blue.length) - 1);
    break;
  }
  case FB_VISUAL_DIRECTCOLOR:
  case FB_VISUAL_PSEUDOCOLOR:
  case FB_VISUAL_STATIC_PSEUDOCOLOR:
    if (ioctl(fd, FBIOGETCMAP, &colormap) != 0)
      posixError("FBIOGETCMAP failed");
    break;
  case FB_VISUAL_MONO01:
    isMono = true;
    break;
  case FB_VISUAL_MONO10:
    isMono = true;
    blackIsZero = true;
    break;
  default:
    notSupported("unsupported visual");
  }
  if (varInfo.bits_per_pixel < 8 && !isMono)
    notSupported("< 8 bpp");
  if (varInfo.bits_per_pixel != 1 && isMono)
    notSupported("monochrome framebuffer is not 1 bpp");
}

void FrameBuffer::process() {
  const size_t mappedLength =
      fixInfo.line_length * (varInfo.yres + varInfo.yoffset);
  unsigned char *videoMemory =
      (unsigned char *)mmap(NULL, mappedLength, PROT_READ, MAP_SHARED, fd, 0);
  if (videoMemory != MAP_FAILED)
    mmappedMemory = true;
  else {
    mmappedMemory = false;
    const size_t buffer_size = fixInfo.line_length * varInfo.yres;
    videoMemory = (unsigned char *)malloc(buffer_size);
    if (videoMemory == NULL)
      posixError("malloc failed");
    off_t offset = lseek(fd, fixInfo.line_length * varInfo.yoffset, SEEK_SET);
    if (offset == (off_t)-1)
      posixError("lseek failed");
    varInfo.yoffset = 0;
    ssize_t read_bytes = read(fd, videoMemory, buffer_size);
    if (read_bytes < 0)
      posixError("read failed");
    else if ((size_t)read_bytes != buffer_size) {
      errno = EIO;
      posixError("read failed");
    }
  }
  if (isMono)
    dumpVideoMemoryMono(videoMemory, &varInfo, blackIsZero, fixInfo.line_length,
                        stdout);
  else
    dumpVideoMemory(videoMemory, &varInfo, &colormap, fixInfo.line_length,
                    stdout);

  // close and free
  if (fclose(stdout))
    posixError("write error");

  /* deliberately ignore errors */
  if (mmappedMemory)
    munmap(videoMemory, mappedLength);
  else
    free(videoMemory);
  ::close(fd);
}

// -*-*-*-*-* public
void FrameBuffer::processAll() {
  check();
  initColormap();
  process();
}

// getter-setter
const char *FrameBuffer::getFbdevName() const { return fbdevName; }

void FrameBuffer::setFbdevName(const char *newFbdevName) {
  fbdevName = newFbdevName;
}
