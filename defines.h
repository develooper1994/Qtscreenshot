#ifndef DEFINES_H
#define DEFINES_H

#include <cstdint>

#define VERSION_MAJOR "01"
#define VERSION_MINOR "00.00"
#define VERSION VERSION_MAJOR "." VERSION_MINOR
#define APPLICATIONNAME "QtScreenShoot"

// defaults
#define defaultHelp "True"
#define defaultClientHelp "False"
#define defaultServerHelp "False"
#define defaultVersion "True"
#define defaultVerbose "False"
#define defaultDebug "False"
#define defaultGui "False"
#define defaultScreen ""
#define defaultScreenType ScreenType::ScreenNumber
#define defaultImageColor ImageColor::Colored
#define Fbdev "/dev/fb"
#define defaultFbdev Fbdev defaultScreen
#define defaultFilename "untitled.jpg"
#define defaultNumber "1"
#define defaultIp "127.0.0.1"        // ip:port
#define defaultIncomePort 8000       // ip:port
#define defaultIncomePortStr "8000"  // ip:port
#define defaultOutcomePort 8001      // ip:port
#define defaultOutcomePortStr "8001" // ip:port
#define defaultBindSeperator ":"     // :
#define defaultBind                                                            \
  defaultIp defaultBindSeperator defaultIncomePortStr // ip:port
#define defaultUsage "client"
#define defaultConnectionType "TCP"
#define MaxPositionalArgs 0
#define Author "* Author: Mustafa Selçuk Çağlar\n"
#define bugTrackerUrl "https://github.com/develooper1994/Qtscreenshot/issues"

enum class ImageColor : uint8_t {
  Ok = 0,
  //              Ascii, Bitmap
  Bitmap = 1,      // P1, P4
  GrayScale = 2,   // P2, P5
  Colored = 3,     // P3, P6
  Transparent = 4, //   , P7 // !! Not IMplemented Yet !!
  NotSupported = 5,
  ImageColorError = 6,
  Max
};

#include "constmessages.h"
#include "help.h"

#endif // DEFINES_H
