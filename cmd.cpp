#include "cmd.h"
#include "defines.h"

#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QScreen>
#include <QSysInfo>
#include <fstream>
#include <iostream>

using Status = CmdParseResult::Status;

static const QStringList usageTypes{"c", "client", "s", "server"};
static const QStringList connectionTypes{"t", "tcp", "u", "udp"};

// lookup table
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
    {Status::ScreenError, __ScreenErrorMessage},
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
});

inline void setApplicationSettings() {
  QApplication::setApplicationName(APPLICATIONNAME);
  QApplication::setApplicationVersion(VERSION);
  QApplication::setApplicationDisplayName(APPLICATIONNAME);
}

// -*-*-*-*-* ScreenInfo *-*-*-*-*-
ScreenInfo::TagScreenInfo() : TagScreenInfo(defaultScreen) {}

ScreenInfo::TagScreenInfo(const QString &screen)
    : TagScreenInfo(screen, ScreenType::ScreenNumber) {}

ScreenInfo::TagScreenInfo(const QString &screen, ScreenType type)
    : screen(screen), type(type) {
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
  int screenNumber = screen.trimmed().toInt(&numberOk); // screen == 0
  // check if numberOk == true otherwise (0 or false)
  const int numberOfScreen = numberOk ? QGuiApplication::screens().count() : 0;
  QScreen *pscreen = QGuiApplication::primaryScreen();
  int defaultScreenNumber = QGuiApplication::screens().indexOf(pscreen);
  const bool screenNumberAvailable = screenNumber < numberOfScreen;
  screenNumber = screenNumberAvailable ? screenNumber : defaultScreenNumber;
  if (!screenNumberAvailable)
    this->type = ScreenType::ScreenError;

#if defined(Q_OS_LINUX)

  const bool fbExist = QFile::exists(screen);
  const bool fbUsageCheck = isFramebufferExists(),
             fbOk = fbUsageCheck && fbExist;
  if (fbUsageCheck)
    this->type = ScreenType::Framebuffer;

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
    this->type = ScreenType::ScreenNumber;
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

// -*-*-*-*-* Cmd *-*-*-*-*-
Cmd::Cmd() {
  init();
  setup();
  load();
}
inline void Cmd::init() {
  parser.addPositionalArgument("subcommand", usageTypes.join('\n'));
  // QStringList cmdOptionNames = parser.optionNames();
  parser.addOptions(optionList);
}
inline void Cmd::setup() {
  parser.setOptionsAfterPositionalArgumentsMode(
      QCommandLineParser::ParseAsOptions);
  parser.setApplicationDescription(QString(HelpMain));
}
inline void Cmd::load() {}

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
      !usageTypes.contains(usage, Qt::CaseSensitivity::CaseInsensitive);
  QString command = usageOptCheck ? QString() : usage;
  Status status;

  if (command.contains(usageTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    parseResult.options.client = true;
    parseResult.options.server = false;
    status = Status::UsageClient;
  } else if (command.contains(usageTypes.at(3),
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
      !connectionTypes.contains(connectionType,
                                Qt::CaseSensitivity::CaseInsensitive);
  QString command = connectionTypeOptCheck ? QString() : connectionType;
  Status status;

  if (!command.compare(connectionTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    status = Status::ConnectionTypeTCP;
  } else if (!command.compare(connectionTypes.at(3),
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
  if (!usageSubcommand.compare(usageTypes.at(1),
                               Qt::CaseSensitivity::CaseInsensitive)) {
    parseResult.options.client = true;
    parseResult.options.server = false;
    // insertUnique(Status::UsageClient);
    setStatus(parseResult.status, Status::UsageClient);
  } else if (!usageSubcommand.compare(usageTypes.at(3),
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
      isStatusSet(status, Status::ScreenError);

  // Errors
  if (!notsetPrintsHelp || errorsPrintsHelp) {
    // There is an error
    // show HelpGeneral
    qDebug() << QApplication::applicationVersion() << "\n";
    parser.showHelp(1);
  }

  if (options.verbose) {
    uint64_t temp = static_cast<uint64_t>(Status::MaxValue) - 1;

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
static inline void setStatus(CmdParseResult::Status &status,
                             const CmdParseResult::Status statToSet) {
  status = status | statToSet;
}
static inline bool isStatusSet(CmdParseResult::Status stat,
                               const CmdParseResult::Status &statToCheck) {
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
          CmdParseResult::Status::MaxValue) -
      1);
}
static inline bool ipValidate(const QString &ipStr) {
  QHostAddress address(ipStr);
  return address.protocol() !=
         QAbstractSocket::NetworkLayerProtocol::UnknownNetworkLayerProtocol;
}

// std::underlying_type_t<CmdParseResult::Status>
CmdParseResult::Status operator|(CmdParseResult::Status &lhs,
                                 CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(static_cast<std::uint64_t>(lhs) |
                                             static_cast<std::uint64_t>(rhs));
}

CmdParseResult::Status operator&(CmdParseResult::Status &lhs,
                                 CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(static_cast<std::uint64_t>(lhs) &
                                             static_cast<std::uint64_t>(rhs));
}

CmdParseResult::Status operator^(CmdParseResult::Status &lhs,
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
