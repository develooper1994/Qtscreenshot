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
static inline uint8_t getColor(uint32_t pixel,
                               const struct fb_bitfield *bitfield,
                               uint16_t *colormap);
static inline uint8_t getGrayscale(uint32_t pixel, const vsi *info,
                                   const struct fb_cmap *colormap);
static inline uint8_t reverseBits(uint8_t b);
static inline void dumpVideoMemoryBitmap(const uint8_t *video_memory,
                                         const vsi *info, bool black_is_zero,
                                         uint32_t line_length, FILE *fp);
static void dumpVideoMemoryGrayscale(const uint8_t *video_memory,
                                     const vsi *info,
                                     const struct fb_cmap *colormap,
                                     uint32_t line_length, FILE *fp);
static inline void dumpVideoMemory(const uint8_t *video_memory, const vsi *info,
                                   const cmap *colormap, uint32_t line_length,
                                   FILE *fp);

static inline int fbcatTest(int argc, const char **argv);
static inline int fbcatGrayScaleTest(int argc, const char **argv);

// -*-*-*-*-* FrameBuffer(c++ interface) *-*-*-*-*-
class FrameBuffer {
  /*
    enum class FrameType : uint8_t {
      Bitmap = 0,    // P1, P4
      GrayScale = 1, // P2, P5
      Colored = 2,   // P3, P6
    };
  */

public:
  FrameBuffer();
  FrameBuffer(const char *fbdevName);
  FrameBuffer(const char *fbdevName, ImageColor imageColor);
  void processAll(ImageColor imageColor = ImageColor::Colored);

  // getter-setter
  const char *getFbdevName() const;
  void setFbdevName(const char *newFbdevName);
  void setFbdevFromEnv();

  ImageColor getFrameType() const;
  void setFrameType(ImageColor newFrameType);

private:
  const char *fbdevName;
  ImageColor frameType = ImageColor::Colored;
  // do not change or modify these variables!
  int fd = STDOUT_FILENO;
  bool blackIsZero = false, mmappedMemory = false;
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
  void process(ImageColor imageColor = ImageColor::Colored);
};

#endif // FBCAT_H
