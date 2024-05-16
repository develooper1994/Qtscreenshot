#ifndef FBCAT_H
#define FBCAT_H

#include "defines.h"
#include <inttypes.h>
#include <linux/fb.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

typedef struct fb_fix_screeninfo fsi;
typedef struct fb_var_screeninfo vsi;
typedef struct fb_cmap cmap;

static inline void posixError(const char *s, ...);
static inline void notSupported(const char *s);
static inline unsigned char getColor(unsigned int pixel,
                                     const struct fb_bitfield *bitfield,
                                     uint16_t *colormap);
static inline unsigned char reverseBits(unsigned char b);
static inline void dumpVideoMemoryMono(const unsigned char *video_memory,
                                       const vsi *info, bool black_is_zero,
                                       unsigned int line_length, FILE *fp);
static inline void dumpVideoMemory(const unsigned char *video_memory,
                                   const vsi *info, const cmap *colormap,
                                   unsigned int line_length, FILE *fp);

static inline int fbcatTest(int argc, const char **argv);

// c++ interface
class FrameBuffer {
public:
  FrameBuffer();
  FrameBuffer(const char *fbdev_name);

  // getter-setter
  const char *getFbdevName() const;
  void setFbdevName(const char *newFbdevName);
  void setFbdevFromEnv();

  void processAll();

private:
  const char *fbdevName;
  int fd = STDOUT_FILENO;
  bool isMono = false, blackIsZero = false, mmappedMemory = false;

  fsi fixInfo;
  vsi varInfo;
  uint16_t colormapData[4][1 << 8];
  cmap colormap = {
      0,
      1 << 8,
      colormapData[0],
      colormapData[1],
      colormapData[2],
      colormapData[3],
  };

  void check();
  void initColormap();
  void process();
};

#endif // FBCAT_H
