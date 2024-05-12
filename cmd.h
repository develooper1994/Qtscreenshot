#ifndef CMD_H
#define CMD_H

#include "defines.h"
#include "dialog.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QObject>

void setApplicationSettings();

// -*-*-*-*-* BindInfo *-*-*-*-*-
typedef struct TagBindInfo {
  /*
enum class BindInfoStatus : uint64_t {
  Ok = 0,
  Error = 1 << 0,
  BindError = 1 << 1,
  IpError = 1 << 2,
  IncomePortError = 1 << 3,
  OutcomePortError = 1 << 4,
  PortError = 1 << 5
};
*/
  QString IpStr = defaultIp, ConnectionType = defaultConnectionType;
  uint16_t IncomePort = defaultIncomePort;
  uint16_t OutcomePort = defaultOutcomePort;
} BindInfo;

// -*-*-*-*-* CmdOptions *-*-*-*-*-
typedef struct TagCmdOptions {
  bool help, gui, verbose, debug, client, server;
  int number = 1;
  QString filename = defaultFilename, usage = defaultUsage;
  BindInfo bindInfo;
  // IsSet;
  bool numberIsSet, filenameIsSet, usageIsSet, bindInfoIsSet,
      connectiontypeIsSet;
} CmdOptions;

// -*-*-*-*-* CmdParseResult *-*-*-*-*-

typedef struct TagCmdParseResult {
  // Max not combined states : 64 - 1(MaxValue) = 63
  enum class Status : std::uint64_t {
    // 0b0000000000000010 // third way
    Ok = 0,                    // 0000 0000 0000 0000 // MinValue
    VersionRequested = 1 << 0, // 0000 0000 0000 0010
    HelpRequested = 1 << 1,    // 0000 0000 0000 0001
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
    MaxValue = UINT64_MAX
  };
  typedef QMap<Status, QString> StatMsgType;

  // Status statusCode = Status::Ok;
  // my compiler is not that advanced
  // std::optional<QString> status = std::nullopt;
  StatMsgType statusMessage = StatMsgType();
  CmdOptions options = CmdOptions();
  Status status = Status::Ok;

public:
  void insertUniqueValue(const Status statToSet = Status::Ok);
  void insertUniqueValue(Status &key);
  void insertUniqueValue(Status &key, const Status statToSet = Status::Ok);
  void insertUniqueValue(const Status &key, const QString &value);
  void insertUniqueValue(Status &key, const QString &value,
                             const Status statToSet = Status::Ok);
} CmdParseResult;

// use "static_cast" if compiler doesn't support "underlying_type_t"
// !!!compile error!!!
/*
template <class T> inline T operator~(T &lhs) {
  return static_cast<T>(~std::underlying_type_t<T>(lhs));
}
*/

inline CmdParseResult::Status operator|(CmdParseResult::Status &lhs,
                                        CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(
      std::underlying_type_t<CmdParseResult::Status>(lhs) |
      std::underlying_type_t<CmdParseResult::Status>(rhs));
}
inline CmdParseResult::Status operator&(CmdParseResult::Status &lhs,
                                        CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(
      std::underlying_type_t<CmdParseResult::Status>(lhs) &
      std::underlying_type_t<CmdParseResult::Status>(rhs));
}
inline CmdParseResult::Status operator^(CmdParseResult::Status &lhs,
                                        CmdParseResult::Status rhs) {
  return static_cast<CmdParseResult::Status>(
      std::underlying_type_t<CmdParseResult::Status>(lhs) |
      std::underlying_type_t<CmdParseResult::Status>(rhs));
}
void setStatus(
    CmdParseResult::Status &stat,
    const CmdParseResult::Status statToSet = CmdParseResult::Status::Ok);
bool isStatusSet(
    CmdParseResult::Status stat = CmdParseResult::Status::Ok,
    const CmdParseResult::Status &statToCheck = CmdParseResult::Status::Ok);
QString getStatusMessage(
    const CmdParseResult::Status &statToCheck = CmdParseResult::Status::Ok);
CmdParseResult::Status getMaximumStatus();

// -*-*-*-*-* Cmd *-*-*-*-*-
class Cmd {
public:
  // constructors
  explicit Cmd();
  void setup();
  void init();
  void load();
  void evalCmd();

private:
  // CmdParseResult::Status status = CmdParseResult::Status::Ok;
  Dialog w;
  CmdParseResult cmdParseResult;
  QCommandLineParser parser;
  // Help, Version and other options
  const QCommandLineOption helpOption = parser.addHelpOption();
  const QCommandLineOption versionOption = parser.addVersionOption();
  const QList<QCommandLineOption> optionList = {
      {{"V", "verbose"}, QApplication::translate("main", __SECTION_Verbose)},
      {{"d", "debug"}, QApplication::translate("main", __SECTION_Debug)},
      {{"g", "gui"}, QApplication::translate("main", __SECTION_Gui)},
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
  bool ipValidate(const QString &ipStr) const;

  void verboseOptionCheck();
  void debugOptionCheck();
  void guiOptionCheck();
  void filenameOptionCheck();
  void numberOptionCheck();
  void bindOptionCheck();
  void usageOptionCheck();
  void connectiontypeOptionCheck();
  void optionChecks();
  void subcommandCheck();

  // Eval Client/Server
  void MainClient();
  void MainServer();
};

#endif // CMD_H
