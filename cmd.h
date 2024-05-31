#ifndef CMD_H
#define CMD_H

#include "defines.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QPixmap>

void setApplicationSettings();

// -*-*-*-*-* Conn *-*-*-*-*-
typedef struct TagConn {
  /*
    enum class ConnStatus : uint64_t {
      Ok = 0,
      Error = 1 << 0,
      ConnError = 1 << 1,
      IpError = 1 << 2,
      IncomePortError = 1 << 3,
      OutcomePortError = 1 << 4,
      PortError = 1 << 5,
      TypeError = 1 << 6
    };
  */
  QString IpStr = defaultIp;
  uint16_t IncomePort = defaultIncomePort;
  uint16_t OutcomePort = defaultOutcomePort;
} Conn;

typedef struct TagScreenInfo {
  enum class ScreenType : uint8_t {
    Ok = 0,
    ScreenNumber = 1,
    Framebuffer = 2,
    X11 = 3,     // !! Not IMplemented Yet !!
    Wayland = 4, // !! Not IMplemented Yet !!
    NotSupported = 5,
    ScreenTypeError = 6,
    Max
  };

  QString screen = defaultScreen; // defaultFbdev
  ScreenType screenType = defaultScreenType;
  ImageColor imageColor = defaultImageColor;

  // let the methods guide your way
  TagScreenInfo();
  TagScreenInfo(const QString &screen);
  TagScreenInfo(const QString &screen, ImageColor imageColor);
  // You are on your own
  TagScreenInfo(const QString &screen, ScreenType screenType);
  TagScreenInfo(const QString &screen, ScreenType screenType,
                ImageColor imageColor);

  inline void printLastScreenTypeMessage();
  inline void printLastImageColorMessage();
  inline void printLastMessage();

  QPixmap captureScreen(ScreenType screenType, ImageColor imageColor);
  void saveImage(QString &imagename, const QPixmap &shoot);
  QPixmap convertToGrayscale(const QPixmap &pixmap);

private:
  inline void create();
  inline void init();
  inline void extractScreen();
  inline void extractScreenParameters();
} ScreenInfo;

// -*-*-*-*-* CmdOptions *-*-*-*-*-
typedef struct TagCmdOptions {
  bool help, gui, verbose, debug, client, server;
  int number = 1;
  QString filename = defaultFilename, usage = defaultUsage,
          connectiontype = defaultConnectionType;
  static inline ScreenInfo screen;
  static inline Conn conn;

  inline void printMessages();
} CmdOptions;

// -*-*-*-*-* CmdParseResult *-*-*-*-*-
typedef struct TagCmdParseResult {
  // Max not combined states : 64 - 1(MaxValue) = 63
  enum class Status : std::uint64_t {
    // 0b0000000000000010 // third way
    Ok = 0,                    // 0000 0000 0000 0000 // MinValue
    VersionRequested = 1 << 0, // 0000 0000 0000 0010
    HelpRequested = 1 << 1,    // 0000 0000 0000 0100
    Error = 1 << 2,            // ...
    SubcommandError = 1 << 3,
    PositionalArgumentsError = 1 << 4,
    ParseError = 1 << 5,
    ServerHelpRequested = 1 << 6,
    ClientHelpRequested = 1 << 7,
    UsageError = 1 << 8,
    UsageClient = 1 << 9,
    UsageServer = 1 << 10,
    FileNameError = 1 << 11,
    FileNameRequired = 1 << 12,
    OnlyIpRequested = 1 << 13,
    OnlyPortRequested = 1 << 14,
    BindRequired = 1 << 15,
    BindError = 1 << 16,
    IpError = 1 << 17,
    IncomePortError = 1 << 18,
    OutcomePortError = 1 << 19,
    ConnectionTypeRequired = 1 << 20,
    ConnectionTypeError = 1 << 21,
    ConnectionTypeTCP = 1 << 22,
    ConnectionTypeUDP = 1 << 23,
    GuiRequired = 1 << 24,
    VerboseRequired = 1 << 25,
    DebugRequired = 1 << 26,
    NumberError = 1 << 27,
    Max
  };
  // Status statusCode = Status::Ok;
  // my compiler is not that advanced
  // std::optional<QString> status = std::nullopt;
  static inline CmdOptions options = CmdOptions();
  static inline Status status = Status::Ok;

  inline void printMessages();

} CmdParseResult;

using StatMsgType = QMap<CmdParseResult::Status, QString>;

// use "static_cast" if compiler doesn't support "underlying_type_t"
// !!!compile error!!!
/*
template <class T> inline T operator~(T &lhs) {
  return static_cast<T>(~std::underlying_type_t<T>(lhs));
}
*/

static inline CmdParseResult::Status operator|(CmdParseResult::Status &lhs,
                                               CmdParseResult::Status rhs);
static inline CmdParseResult::Status operator&(CmdParseResult::Status &lhs,
                                               CmdParseResult::Status rhs);
static inline CmdParseResult::Status operator^(CmdParseResult::Status &lhs,
                                               CmdParseResult::Status rhs);
static inline void
setStatus(CmdParseResult::Status &stat,
          const CmdParseResult::Status statToSet = CmdParseResult::Status::Ok);

static bool isStatusSet(
    CmdParseResult::Status stat = CmdParseResult::Status::Ok,
    const CmdParseResult::Status &statToCheck = CmdParseResult::Status::Ok);

static inline bool isStatusEqualAny(
    const StatMsgType &statMessage,
    const CmdParseResult::Status &statToCheck = CmdParseResult::Status::Ok);
static inline QString getStatusMessage(
    const CmdParseResult::Status &statToCheck = CmdParseResult::Status::Ok);
static inline CmdParseResult::Status getMaximumStatus();
static inline bool ipValidate(const QString &ipStr);
static inline bool isFramebufferInUse();
static inline bool isFramebufferExists();

// -*-*-*-*-* Cmd *-*-*-*-*-
class Cmd {
public:
  // constructors
  explicit Cmd();
  void evalCmd();

  // get-set
  CmdParseResult getCmdParseResult() const;

private:
  inline void setup();
  inline void init();
  inline void load();
  inline void printMessages();
  // CmdParseResult::Status status = CmdParseResult::Status::Ok;
  StatMsgType statusMessage = StatMsgType();
  CmdParseResult parseResult;
  static inline QCommandLineParser parser;
  // Help, Version and other options
  static inline const QCommandLineOption helpOption = parser.addHelpOption();
  static inline const QCommandLineOption versionOption =
      parser.addVersionOption();
  static inline const QList<QCommandLineOption> optionList = {
      {{"V", "verbose"}, QApplication::translate("main", __SECTION_Verbose)},
      {{"d", "debug"}, QApplication::translate("main", __SECTION_Debug)},
      {{"g", "gui"}, QApplication::translate("main", __SECTION_Gui)},
      {{"s", "screen"},
       QApplication::translate("main", __SECTION_Screen),
       QApplication::translate("main", "screen"),
       defaultScreen},
      {{"f", "filename"},
       QApplication::translate("main", __SECTION_Filename),
       QApplication::translate("main", "filename"),
       defaultFilename},
      {{"n", "number"},
       QApplication::translate("main", __SECTION_Number),
       QApplication::translate("main", "number"),
       defaultNumber},
      /*
      {{"u", "usage"},
       QApplication::translate("main", __SECTION_Usage),
       QApplication::translate("main", "usage(client/server)"),
       defaultUsage},
      */
      {{"c", "ct", "connection-type"},
       QApplication::translate("main", __SECTION_ConnectionType),
       QApplication::translate("main", "usage(TCP/UDP)"),
       defaultConnectionType},
      {{"b", "bind"},
       QApplication::translate("main", __SECTION_Bind),
       QApplication::translate("main", "bind"),
       defaultBind}};

  // private methods
  void parseCommandLine();

  void verboseOptionCheck();
  void debugOptionCheck();
  void guiOptionCheck();
  void filenameOptionCheck();
  void screenOptionCheck();
  void numberOptionCheck();
  void bindOptionCheck();
  void usageOptionCheck();
  void connectiontypeOptionCheck();
  void optionChecks();
  void subcommandChecks();
};

#endif // CMD_H
