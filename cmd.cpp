#include "cmd.h"
#include "defines.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QImage>
#include <QPainter>
#include <QScreen>
#include <QSysInfo>
#include <fstream>
#include <iostream>
#if defined(Q_OS_LINUX)
#include "fbcat.h"
#endif

using ScreenType = ScreenInfo::ScreenType;
using Status = CmdParseResult::Status;

inline void setApplicationSettings() {
  QApplication::setApplicationName(APPLICATIONNAME);
  QApplication::setApplicationVersion(VERSION);
  QApplication::setApplicationDisplayName(APPLICATIONNAME);
}

// -*-*-*-*-* ScreenInfo *-*-*-*-*-
// lookup tables
// TODO: create enum and generate strings from enum. Use QMetaEnum
static const QStringList __screenTypes{
    "n",
    "sn",
    "ScreenNumber",
    "f"
    "fb",
    "Framebuffer",
    "X11",
    "w",
    "wl",
    "Wayland",
};
static const QStringList __imageColorTypes{
    "b",         "bm", "Bitmap",  "g", "gs",
    "GrayScale", "c",  "Colored", "t", "Transparent"};
static const QMap<ScreenType, QString> __screenScreenTypeMessage({
    // compiletime
    {ScreenType::Ok, __Ok},
    {ScreenType::ScreenNumber, "ScreenNumber"},
    {ScreenType::Framebuffer, "Framebuffer"},
    {ScreenType::X11, "X11"},
    {ScreenType::Wayland, "Wayland"},
    {ScreenType::NotSupported, "Not Supported"},
    {ScreenType::ScreenTypeError, __ScreenErrorMessage},
    {ScreenType::Max, "Max"},
});
static const QMap<ImageColor, QString> __screenImageColorMessage({
    // compiletime
    {ImageColor::Ok, __Ok},
    {ImageColor::Bitmap, "Bitmap Image"},
    {ImageColor::GrayScale, "GrayScale Image"},
    {ImageColor::Colored, "Colored Image"},
    {ImageColor::Transparent, "Transparent Image"},
    {ImageColor::NotSupported, "Not Supported"},
    {ImageColor::Max, "Max"},
});

// let the methods guide your way
ScreenInfo::TagScreenInfo() : TagScreenInfo(defaultScreen) {}

ScreenInfo::TagScreenInfo(const QString &screen)
    : TagScreenInfo(screen, defaultImageColor) {}

TagScreenInfo::TagScreenInfo(const QString &screen, ImageColor imageColor)
    : TagScreenInfo(screen, defaultScreenType, imageColor) {
  create();
}

// You are on your own
ScreenInfo::TagScreenInfo(const QString &screen, ScreenType screenType)
    : TagScreenInfo(screen, screenType, defaultImageColor) {}

ScreenInfo::TagScreenInfo(const QString &screen, ScreenType screenType,
                          ImageColor imageColor)
    : screen(screen), screenType(screenType), imageColor(imageColor) {
  init();
}

inline void ScreenInfo::create() {
  extractScreen();
  extractScreenParameters();
}
void ScreenInfo::init() {
  // init whatever you want!
}

inline void ScreenInfo::extractScreen() {
  /* Demorgan
   !(P && Q) = (!P) || (!Q)
   !(!P && !Q) = (P) || (Q)
  */
  /*
   * if os is using framebuffer to display something than assume the screen
   number as framebuffer number. Ex: /dev/fb + QString::number(screen)

   * /dev/fb0
   * <argv[0]> -d -V -s 0 -f img.jpg
  */

  /*
   Sayıya çevirmeyi dene,
    - numberOk == true,
      - numberOfScreen = QGuiApplication::screens().count() olmasıyla
      - screenNumberAvailable kontrolü yapılır
        - screenNumberAvailable == true,
          - screenNumber = screenNumber;
        - screenNumberAvailable == false,
          - screenNumber = defaultScreenNumber;
    - numberOk == false,
      - numberOfScreen=0 olmasıyla
        - screenNumberAvailable=false olur
          - screenNumber == defaultScreenNumber yani sanırım 1. ekran.
   */
  bool numberOk;
  // screen.toStdString().c_str()
  QStringList screenParams = this->screen.split(defaultBindSeperator);
  int screenNumber =
      screenParams.at(0).trimmed().toInt(&numberOk); // screen == 0
  // check if numberOk == true otherwise (0 or false)
  const int numberOfScreen = numberOk ? QGuiApplication::screens().count() : 0;
  QScreen *pscreen = QGuiApplication::primaryScreen();
  int pscreenIndex = QGuiApplication::screens().indexOf(pscreen);
  const bool screenNumberAvailable = screenNumber < numberOfScreen;
  screenNumber = screenNumberAvailable ? screenNumber : pscreenIndex;
  if (!screenNumberAvailable)
    this->screenType = ScreenType::NotSupported;

#if defined(Q_OS_LINUX)

  const bool fbExist = QFile::exists(screen);
  const bool fbUsageCheck = isFramebufferExists(),
             fbOk = fbUsageCheck && fbExist;
  if (fbUsageCheck)
    this->screenType = ScreenType::Framebuffer;

  if (fbOk) {
    // input == "/dev/fb0"
    this->screen = screen;
  } else if (fbUsageCheck) {
    // "/dev/fb" + (input == ?)
    if (numberOk)
      this->screen = QString(Fbdev) + QString::number(screenNumber);
    else
      this->screen = QString(Fbdev);
  } else if (numberOk) {
    // no framebuffer
    this->screen = QString::number(screenNumber);
    this->screenType = ScreenType::ScreenNumber;
  }

#elif defined(Q_OS_WIN) || defined(Q_OS_MACX)
  this->screen = QString::number(screenNumber);
  this->type = ScreenType::ScreenNumber;
#else
  this->screen = screen;
  this->type = ScreenType::ScreenError;
  qDebug() << "Not implemented yet!";
#endif

  qDebug() << "screen: " << this->screen;
}

inline void ScreenInfo::extractScreenParameters() {
  // "" -> [""]
  // ":" -> ["", ""]
  QString ScreenTypeStr;
  QStringList screenParams = screen.split(defaultBindSeperator);
  if (screenParams.count() == 1) {
    // -s ""
    screenType = defaultScreenType;
    return;
  }
  // -*-*-* ScreenType *-*-*-
  if (screenParams.count() > 1) {
    QString temp1 = screenParams.at(1);
    bool tempCheck =
        temp1.isEmpty() &&
        !__screenTypes.contains(temp1, Qt::CaseSensitivity::CaseInsensitive);
    QString command = tempCheck ? QString() : temp1;

    if (tempCheck) {
      // -s "/dev/fd:" // -s ":"
      screenType = defaultScreenType;
      return;
    }
    // -s ":asd" // -s "asd:asd"

    if (!command.compare(__screenTypes.at(2),
                         Qt::CaseSensitivity::CaseInsensitive)) {
      screenType = ScreenType::ScreenNumber;
    } else if (!command.compare(__screenTypes.at(5),
                                Qt::CaseSensitivity::CaseInsensitive)) {
      screenType = ScreenType::Framebuffer;
    } else if (!command.compare(__screenTypes.at(6),
                                Qt::CaseSensitivity::CaseInsensitive)) {
      screenType = ScreenType::X11;
    } else if (!command.compare(__screenTypes.at(9),
                                Qt::CaseSensitivity::CaseInsensitive)) {
      screenType = ScreenType::Wayland;
    } else {
      screenType = ScreenType::NotSupported;
    }
  }

  // -*-*-* ImageColor *-*-*-
  if (screenParams.count() > 2) {
    QString temp1 = screenParams.at(2);
    bool tempCheck =
        temp1.isEmpty() && !__imageColorTypes.contains(
                               temp1, Qt::CaseSensitivity::CaseInsensitive);
    QString command = tempCheck ? QString() : temp1;

    if (tempCheck) {
      // -s "/dev/fd:" // -s ":"
      imageColor = defaultImageColor;
      return;
    }
    // -s ":asd" // -s "asd:asd"

    if (!command.compare(__imageColorTypes.at(2),
                         Qt::CaseSensitivity::CaseInsensitive)) {
      imageColor = ImageColor::Bitmap;
    } else if (!command.compare(__imageColorTypes.at(5),
                                Qt::CaseSensitivity::CaseInsensitive)) {
      imageColor = ImageColor::GrayScale;
    } else if (!command.compare(__imageColorTypes.at(6),
                                Qt::CaseSensitivity::CaseInsensitive)) {
      imageColor = ImageColor::Colored;
    } else if (!command.compare(__imageColorTypes.at(9),
                                Qt::CaseSensitivity::CaseInsensitive)) {
      imageColor = ImageColor::Transparent;
    } else {
      imageColor = ImageColor::NotSupported;
    }
  }
}

inline void ScreenInfo::printLastScreenTypeMessage() {
  // one state can be set a time!
  QMapIterator<ScreenType, QString> it(__screenScreenTypeMessage);

  while (it.hasNext()) {
    if (this->screenType == it.key()) {
      qDebug() << it.value();
      break;
    }
    it.next();
  }
}
inline void ScreenInfo::printLastImageColorMessage() {
  // one state can be set a time!
  QMapIterator<ImageColor, QString> it(__screenImageColorMessage);

  while (it.hasNext()) {
    if (this->imageColor == it.key()) {
      qDebug() << it.value();
      break;
    }
    it.next();
  }
}
inline void ScreenInfo::printLastMessage() {
  printLastScreenTypeMessage();
  printLastImageColorMessage();
}

QPixmap ScreenInfo::captureScreen(ScreenType screenType,
                                  ImageColor imageColor) {
  bool ok;
  int screenNumber = screen.toInt(&ok);

  QPixmap image;

  if (screenType == ScreenType::ScreenNumber) {

    QScreen *screen = QGuiApplication::screens().at(screenNumber);
    if (!screen) {
      qCritical() << "Error: Couldn't get screen";
      return QPixmap();
    }
    image = screen->grabWindow(0); // 0->entire screen
    if (image.isNull()) {
      qCritical() << "Error: Failed to grab screen";
      return QPixmap();
    }

    switch (imageColor) {
    case ImageColor::GrayScale:
      break;
    case ImageColor::Colored:
      return image;
      break;
    case ImageColor::NotSupported:
    case ImageColor::ImageColorError:
    default:
      qFatal("There is a Fatal error about image color capturing!");
      break;
    }
  } else if (screenType == ScreenType::Framebuffer) {
#if defined(Q_OS_LINUX)
    FrameBuffer frameBuffer;
#endif
    switch (imageColor) {
    case ImageColor::GrayScale:
      break;
    case ImageColor::Colored:
      break;
    case ImageColor::NotSupported:
    case ImageColor::ImageColorError:
    default:
      qFatal("There is a Fatal error about image color capturing!");
      break;
    }
  } else {
    qFatal("There is a Fatal error about screen capturing!");
  }

  return image;
}

void ScreenInfo::saveImage(QString &imagename, const QPixmap &shoot) {
  /*
    QString initialPath = QStandardPaths::writableLocation(
        QStandardPaths::StandardLocation::PicturesLocation);
    if (initialPath.isEmpty()) {
      initialPath = QDir::currentPath();
    }
  */
  QString initialPath = QDir::currentPath();
  if (imagename.isEmpty()) {
    imagename = defaultFilename;
  }
  QString fullname =
      QString("%1%2%3").arg(initialPath).arg(QDir::separator()).arg(imagename);
  if (!shoot.save(fullname)) {
    qDebug() << "Save Error: "
             << QString("Image couldn't save to \"%1\"").arg(imagename);
  }
}

QPixmap TagScreenInfo::convertToGrayscale(const QPixmap &pixmap) {
  QImage image = pixmap.toImage().convertToFormat(QImage::Format_Grayscale8);
  return QPixmap::fromImage(image);
}

// -*-*-*-*-* CmdOptions *-*-*-*-*-
inline void CmdOptions::printMessages() { screen.printLastMessage(); }

// -*-*-*-*-* CmdParseResult *-*-*-*-*-
inline void CmdParseResult::printMessages() { options.printMessages(); }

// -*-*-*-*-* Cmd *-*-*-*-*-
// lookup tables
static const QStringList __usageTypes{"c", "client", "s", "server"};
static const QStringList __connectionTypes{"t", "tcp", "u", "udp"};
static QMap<Status, QString> __statusToMessage({
    // runtime
    // {Status::FileNameRequired, __FilenameMessage(cmdParseResult)},
    // {Status::BindRequired, __BindRequiredMessage(ipPort)},
    // {Status::ParserError, __ParserError},
    // {Status::ParserError, QString("%1 %2").arg("There are unknown options: ",
    //                       unknownOptions.join(" "))},
    // compiletime
    {Status::Ok, __Ok},
    {Status::HelpRequested, ""},    // HelpMain, HelpClient, HelpServer
    {Status::VersionRequested, ""}, // VERSION
    {Status::Error, __UnkownError},
    {Status::PositionalArgumentsError, __PositionalArgumentsErrorMessage},
    {Status::VerboseRequired, __VerboseMessage},
    {Status::DebugRequired, __DebugMessage},
    {Status::GuiRequired, __GuiMessage},
    {Status::FileNameError, __FilenameErrorMessage},
    {Status::BindError, __BindErrorMessage},
    {Status::NumberError, __NumberErrorMessage},
    {Status::UsageError, __UsageErrorMessage},
    {Status::UsageClient, __ClientMessage},
    {Status::UsageServer, __ServerMessage},
    {Status::ConnectionTypeError, __ConnectionTypeErrorMessage},
    {Status::ConnectionTypeTCP, __TcpMessage},
    {Status::ConnectionTypeUDP, __UdpMessage},
    {Status::IpError, __IpErrorMessage},
    {Status::IncomePortError, __IncomePortErrorMessage},
    {Status::OutcomePortError, __OutcomePortErrorMessage},
    {Status::Max, "Max"},
});
Cmd::Cmd() {
  init();
  setup();
  load();
}
inline void Cmd::init() {
  parser.addPositionalArgument("subcommand", __usageTypes.join('\n'));
  // QStringList cmdOptionNames = parser.optionNames();
  parser.addOptions(optionList);
}
inline void Cmd::setup() {
  parser.setOptionsAfterPositionalArgumentsMode(
      QCommandLineParser::ParseAsOptions);
  parser.setApplicationDescription(QString(HelpMain));
}
inline void Cmd::load() {}
inline void Cmd::printMessages() {
  // multiple state can be set a time!
  const Status &status = parseResult.status;
  uint64_t temp = static_cast<uint64_t>(Status::Max) - 1;
  // ScreenInfo::printMessages();
  parseResult.printMessages();

  /*
   * 1) shift_end=(MaxValue-1);  while(shift_end){shift_end=shift_end>> 1}
   * 2) **bitset class**
   * 3) *bitfield member*
   * 11001101
   * 01000000
   */
  while (temp > 0) {
    Status tempStatus = static_cast<Status>(temp);
    if (isStatusSet(status, tempStatus))
      qDebug() << __statusToMessage[tempStatus];
    temp >>= 1;
  }
}

inline void Cmd::verboseOptionCheck() {
  parseResult.options.verbose = parser.isSet("verbose");

  if (parseResult.options.verbose) {
    // insertUnique(Status::VerboseRequired);
    setStatus(parseResult.status, Status::VerboseRequired);
  }
}
inline void Cmd::debugOptionCheck() {
  parseResult.options.debug = parser.isSet("debug");

  if (parseResult.options.debug) {
    // insertUnique(Status::DebugRequired);
    setStatus(parseResult.status, Status::DebugRequired);
  }
}
inline void Cmd::guiOptionCheck() {
  parseResult.options.gui = parser.isSet("gui");

  if (parseResult.options.gui) {
    // insertUnique(Status::GuiRequired);
    setStatus(parseResult.status, Status::GuiRequired);
  }
}

inline void Cmd::filenameOptionCheck() {
  parseResult.options.filename = parser.value("filename").trimmed();
  __statusToMessage.insert(Status::FileNameRequired,
                           __FilenameMessage(parseResult.options.filename));

  Status status = parser.isSet("filename") ? Status::FileNameRequired
                                           : Status::FileNameError;

  // insertUnique(status);
  setStatus(parseResult.status, status);
}
inline void Cmd::numberOptionCheck() {
  bool numberOk;
  parseResult.options.number =
      parser.value("number").trimmed().toInt(&numberOk);

  if (!numberOk) {
    // insertUnique(Status::NumberError);
    setStatus(parseResult.status, Status::NumberError);
  }
}
inline void Cmd::screenOptionCheck() {
  /* Demorgan
   !(P && Q) = (!P) || (!Q)
   !(!P && !Q) = (P) || (Q)
  */

  // if os is using framebuffer to display something than assume the screen
  // number as framebuffer number. Ex: /dev/fb + QString::number(screen)
  const QString &screenVal = parser.value("screen"); // .trimmed()

  /*
  // /dev/fb0
  // <argv[0]> -d -V -s 0 -f img.jpg
  bool numberOk;
  int screenAsNumber = screenVal.trimmed().toInt(&numberOk);
  const int numberOfScreen = QGuiApplication::screens().count();

  if (!numberOk || numberOfScreen <= screenAsNumber) {
    // insertUnique(Status::ScreenError);
    setStatus(parseResult.status, Status::ScreenError);
    return;
  }
  */

  parseResult.options.screen = ScreenInfo(screenVal);
}

inline void Cmd::usageOptionCheck() {
  QString usage = parser.value("usage").trimmed();
  bool usageOptCheck =
      usage.isEmpty() &&
      !__usageTypes.contains(usage, Qt::CaseSensitivity::CaseInsensitive);
  QString command = usageOptCheck ? QString() : usage;
  Status status;

  if (command.contains(__usageTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    parseResult.options.client = true;
    parseResult.options.server = false;
    status = Status::UsageClient;
  } else if (command.contains(__usageTypes.at(3),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    parseResult.options.client = false;
    parseResult.options.server = true;
    status = Status::UsageServer;
  } else {
    status = Status::UsageError;
    parseResult.options.client = false;
    parseResult.options.server = false;
    command = QString(defaultUsage);
  }

  parseResult.options.usage = command;
  // insertUnique(status);
  setStatus(parseResult.status, status);
}
inline void Cmd::connectiontypeOptionCheck() {
  QString connectionType = parser.value("connection-type").trimmed();
  bool connectionTypeOptCheck =
      connectionType.isEmpty() &&
      !__connectionTypes.contains(connectionType,
                                  Qt::CaseSensitivity::CaseInsensitive);
  QString command = connectionTypeOptCheck ? QString() : connectionType;
  Status status;

  if (!command.compare(__connectionTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    status = Status::ConnectionTypeTCP;
  } else if (!command.compare(__connectionTypes.at(3),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    status = Status::ConnectionTypeUDP;
  } else {
    status = Status::ConnectionTypeError;
    command = QString(defaultConnectionType);
  }

  parseResult.options.connectiontype = command;
  // insertUnique(status);
  setStatus(parseResult.status, status);
}
inline void Cmd::bindOptionCheck() {
  bool portOk;
  QString bindStr = parser.value("bind").trimmed();
  QStringList ipPort = bindStr.split(defaultBindSeperator);

  if (ipPort.count() < 2) {
    // insertUnique(Status::BindError);
    setStatus(parseResult.status, Status::BindError);
    return;
  } else {
    if (!ipValidate(ipPort.at(0))) {
      // insertUnique(Status::IpError);
      setStatus(parseResult.status, Status::IpError);
      return;
    }
    __statusToMessage.insert(Status::ParseError, __BindRequiredMessage(ipPort));
    // insertUnique(Status::BindRequired);
    setStatus(parseResult.status, Status::BindError);
    parseResult.options.conn.IpStr = ipPort.at(0);

    if (ipPort.count() == 2) {
      const auto IncomePort = ipPort.at(1).toUInt(&portOk);
      if ((IncomePort > 65535) && !portOk) {
        // insertUnique(Status::IncomePortError);
        setStatus(parseResult.status, Status::IncomePortError);
      } else {
        parseResult.options.conn.IncomePort =
            static_cast<uint16_t>(ipPort.at(1).toUInt(&portOk));
      }
    }

    // !not that important!
    if (ipPort.count() > 2) {
      const auto OutcomePort = ipPort.at(2).toUInt(&portOk);
      if ((OutcomePort > 65535) && !portOk) {
        // insertUnique(Status::OutcomePortError);
        setStatus(parseResult.status, Status::OutcomePortError);
      } else {
        parseResult.options.conn.OutcomePort =
            static_cast<uint16_t>(ipPort.at(2).toUInt(&portOk));
      }
    }
  }
}

inline void Cmd::optionChecks() {
  // Verbose option check
  verboseOptionCheck();
  // Gui option check
  guiOptionCheck();
  // Filename option check
  filenameOptionCheck();
  // Screen option check
  screenOptionCheck();
  // Number option check
  numberOptionCheck();
  // Usage option check
  // usageOptionCheck();
  // Bind option check
  bindOptionCheck();
  // ConnectionType option check
  connectiontypeOptionCheck();
}
inline void Cmd::subcommandChecks() {
  QStringList positionalArguments = parser.positionalArguments();
  if (parseResult.options.debug) {
    qDebug() << "positionalArguments: " << positionalArguments;
  }

  QString usageSubcommand =
      positionalArguments.count() > 0 ? positionalArguments.at(0) : QString();
  // QString connectiontypeSubcommand = positionalArguments.count() > 1 ?
  // positionalArguments.at(1) : QString();

  // usage as subcommand
  if (!usageSubcommand.compare(__usageTypes.at(1),
                               Qt::CaseSensitivity::CaseInsensitive)) {
    parseResult.options.client = true;
    parseResult.options.server = false;
    // insertUnique(Status::UsageClient);
    setStatus(parseResult.status, Status::UsageClient);
  } else if (!usageSubcommand.compare(__usageTypes.at(3),
                                      Qt::CaseSensitivity::CaseInsensitive)) {
    parseResult.options.client = false;
    parseResult.options.server = true;
    // insertUnique(Status::UsageServer);
    setStatus(parseResult.status, Status::UsageServer);
  } else {
    parseResult.options.client = false;
    parseResult.options.server = false;
    // insertUnique(Status::UsageError);
    setStatus(parseResult.status, Status::UsageError);
    parseResult.options.usage = defaultUsage;
  }

  /*
  // connection-type as subcommand
  if (!connectiontypeSubcommand.compare(connectionTypes.at(1),
                                        Qt::CaseSensitivity::CaseInsensitive))
  { cmdParseResult.options.connectiontypeIsSet = true;
    // insertUniqueValue(cmdParseResult.status, __TcpMessage,
    // Status::ConnectionTypeTCP);
    insertUnique(Status::ConnectionTypeTCP);
  } else if (!connectiontypeSubcommand.compare(
                 connectionTypes.at(3), Qt::CaseSensitivity::CaseInsensitive))
  { cmdParseResult.options.connectiontypeIsSet = true;
    // insertUniqueValue(cmdParseResult.status, __UdpMessage,
    // Status::ConnectionTypeUDP);
    insertUnique(Status::ConnectionTypeUDP);
  } else {
    cmdParseResult.options.connectiontypeIsSet = false;
    cmdParseResult.options.connectiontype = defaultConnectionType;
    // cmdParseResult.insertWithUniqueValue(status,
    // __ConnectionTypeErrorMessage, Status::ConnectionTypeError);
    insertUnique(Status::ConnectionTypeError);
  }
  */
}

inline void Cmd::parseCommandLine() {

  QStringList arguments = QApplication::arguments();
  bool result = !parser.parse(arguments);
  QStringList optionNames = parser.optionNames(),
              unknownOptions = parser.unknownOptionNames();

  // runtime error message message set
  if (result) {
    __statusToMessage.insert(Status::ParseError, __ParserError);
    // insertUniqueValue(Status::ParseError, __ParserError);
    setStatus(parseResult.status, Status::ParseError);
    return;
  } else if (!unknownOptions.isEmpty()) {
    __statusToMessage.insert(
        Status::ParseError,
        QString("There are unknown options: %2").arg(unknownOptions.join(" ")));
    // insertUniqueValue(Status::ParseError, QString("There are unknown
    // options: %2").arg(unknownOptions.join(" ")));
    setStatus(parseResult.status, Status::ParseError);
    return;
  }

  // Retrieve option values
  // Debug option check
  debugOptionCheck();
  subcommandChecks();
  optionChecks();

  if (parseResult.options.debug) {
    qDebug() << "arguments: " << arguments;
    qDebug() << "optionNames: " << optionNames;
    qDebug() << "unknownOption: " << unknownOptions;
  }

  if (parser.isSet(versionOption)) {
    parser.showVersion(); // closes the application
  } else if (parser.isSet(helpOption)) {
    qDebug() << QApplication::applicationVersion();
    if (parseResult.options.client) {
      qDebug() << "-*-*-*-*-* HelpClient *-*-*-*-*-";
      qDebug() << HelpClient;
    } else if (parseResult.options.server) {
      qDebug() << "-*-*-*-*-* HelpServer *-*-*-*-*-";
      qDebug() << HelpServer;
    } else {
      qDebug() << "-*-*-*-*-* HelpMain *-*-*-*-*-";
      qDebug() << HelpMain;
      // parser.showHelp(0);
    }
    exit(0);
    return;
  }
  return;
}

void Cmd::evalCmd() {
  setApplicationSettings();
  if (parseResult.status == Status::Ok) {
    parseCommandLine();
  }

  const CmdOptions &options = parseResult.options;
  const Status &status = parseResult.status;
  const ScreenType &screenType = parseResult.options.screen.screenType;
  const ImageColor &imageColor = parseResult.options.screen.imageColor;

  bool connectiontypeIsSet = parser.isSet("connection-type"),
       numberIsSet = parser.isSet("connection-type"),
       filenameIsSet = parser.isSet("filename"),
       screenIsSet = parser.isSet("screen"), usageIsSet = parser.isSet("usage"),
       bindInfoIsSet = parser.isSet("bind");

  bool notsetPrintsHelp = usageIsSet || connectiontypeIsSet || bindInfoIsSet ||
                          filenameIsSet || numberIsSet || screenIsSet ||
                          options.verbose || options.gui || options.debug;
  /*
    bool allErrorCheck =
        (isStatusSet(parseResult.status, Status::Error) &&
         isStatusSet(parseResult.status, Status::SubcommandError) &&
         isStatusSet(parseResult.status, Status::PositionalArgumentsError) &&
         isStatusSet(parseResult.status, Status::ParseError) &&
         isStatusSet(parseResult.status, Status::UsageError) &&
         isStatusSet(parseResult.status, Status::FileNameError) &&
         isStatusSet(parseResult.status, Status::BindError) &&
         isStatusSet(parseResult.status, Status::IpError) &&
         isStatusSet(parseResult.status, Status::IncomePortError) &&
         isStatusSet(parseResult.status, Status::OutcomePortError) &&
         isStatusSet(parseResult.status, Status::ConnectionTypeError) &&
         isStatusSet(parseResult.status, Status::NumberError)) ||
        isStatusSet(parseResult.status, Status::ScreenError);
  */

  bool errorsPrintsHelp =
      isStatusSet(status, Status::Error) ||
      isStatusSet(status, Status::SubcommandError) ||
      isStatusSet(status, Status::PositionalArgumentsError) ||
      isStatusSet(status, Status::ParseError) ||
      isStatusSet(status, Status::IpError) ||
      isStatusSet(status, Status::IncomePortError) ||
      isStatusSet(status, Status::OutcomePortError) ||
      isStatusSet(status, Status::ConnectionTypeError) ||
      isStatusSet(status, Status::NumberError) ||
      (screenType == ScreenType::ScreenTypeError) ||
      (imageColor == ImageColor::ImageColorError);

  // Errors
  if (!notsetPrintsHelp || errorsPrintsHelp) {
    // There is an error
    // show HelpGeneral
    qDebug() << QApplication::applicationVersion() << "\n";
    parser.showHelp(1);
  }

  if (options.verbose) {
    printMessages();
  }

  /*
  QMapIterator<Status, QString> statusMessageIterator(statusMessage);
  while (statusMessageIterator.hasNext()) {
    statusMessageIterator.next();
    Status statusCode = statusMessageIterator.key();
    QString message = statusMessageIterator.value();
    // errorMessage = errorMessage.isEmpty() ? "" : QString("Unknown error");

    if (!message.isEmpty()) {
      qWarning() << message;
    }

  }
*/
}

CmdParseResult Cmd::getCmdParseResult() const { return parseResult; }

// -*-*-*-*-* CmdParseResult *-*-*-*-*-
static inline void setStatus(CmdParseResult::Status &status,
                             const CmdParseResult::Status statToSet) {
  status = status | statToSet;
}
bool isStatusSet(TagCmdParseResult::Status stat,
                 const TagCmdParseResult::Status &statToCheck) {
  /*
   * 1011 & 0010 == 0010 => True
   * 0000 & 0000 == 0000 => True
   * stat & statToCheck == stat
   */
  uint64_t a = static_cast<uint64_t>(stat);
  uint64_t b = static_cast<uint64_t>(statToCheck);
  uint64_t result = ((a & b) == b);
  return result;
}
inline bool isStatusEqualAny(const StatMsgType &statMessage,
                             const CmdParseResult::Status &statToCheck) {
  QMapIterator<Status, QString> statusTempIterator(statMessage);
  CmdParseResult::Status statusCode;
  while (statusTempIterator.hasNext()) {
    statusTempIterator.next();
    statusCode = statusTempIterator.key();
    if (statusCode == statToCheck) {
      return true;
    }
  }
  return false;
}
static inline QString
getStatusMessage(const CmdParseResult::Status &statToCheck) {
  const QString &message = __statusToMessage[statToCheck];
  return message.isEmpty() ? QString(__UnkownError) : message;
}
static inline CmdParseResult::Status getMaximumStatus() {
  /*
  QMetaEnum statMetaEnum = QMetaEnum::fromType<CmdParseResult::Status>();
  uint64_t statMetaEnumCount = statMetaEnum.keyCount();
  return statMetaEnum.value(statMetaEnumCount);
  */

  return static_cast<CmdParseResult::Status>(
      std::underlying_type_t<CmdParseResult::Status>(
          CmdParseResult::Status::Max) -
      1);
}
static inline bool ipValidate(const QString &ipStr) {
  QHostAddress address(ipStr);
  return address.protocol() !=
         QAbstractSocket::NetworkLayerProtocol::UnknownNetworkLayerProtocol;
}

// std::underlying_type_t<CmdParseResult::Status>
static inline CmdParseResult::Status operator|(CmdParseResult::Status &lhs,
                                               CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(static_cast<std::uint64_t>(lhs) |
                                             static_cast<std::uint64_t>(rhs));
}

static inline CmdParseResult::Status operator&(CmdParseResult::Status &lhs,
                                               CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(static_cast<std::uint64_t>(lhs) &
                                             static_cast<std::uint64_t>(rhs));
}

static inline CmdParseResult::Status operator^(CmdParseResult::Status &lhs,
                                               CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(static_cast<std::uint64_t>(lhs) |
                                             static_cast<std::uint64_t>(rhs));
}

static inline bool isFramebufferInUse() {
  const char *framebufferDevices[] = {Fbdev, Fbdev "0", Fbdev "1", Fbdev "2",
                                      Fbdev "3"};
  for (const char *device : framebufferDevices) {
    std::ifstream fb(device);
    if (fb.good()) {
      fb.close();
      return true;
    }
  }
  return false;
}

static inline bool isFramebufferExists() {
  const char *framebufferDevices[] = {
      Fbdev,     Fbdev "0", Fbdev "1", Fbdev "2", Fbdev "3", Fbdev "4",
      Fbdev "5", Fbdev "6", Fbdev "7", Fbdev "8", Fbdev "9"};
  for (const char *device : framebufferDevices) {
    if (QFile::exists(QString(device))) {
      return true;
    }
  }
  return false;
}
