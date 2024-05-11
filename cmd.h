#ifndef CMD_H
#define CMD_H

#include "defines.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QObject>

void setApplicationSettings();

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

typedef struct TagCmdOptions {
  bool help, gui, verbose, debug, client, server;
  int number = 1;
  QString filename = defaultFilename, usage = defaultUsage;
  BindInfo bindInfo;
  // IsSet;
  bool numberIsSet, filenameIsSet, usageIsSet, bindInfoIsSet,
      connectiontypeIsSet;
} CmdOptions;

typedef struct TagCmdParseResult {
  // Max states : 64 - 1(MaxValue) = 63
  enum class Status : std::uint64_t {
    // 0b0000000000000010 // third way
    Ok = 0,                    // 0000 0000 0000 0000 // MinValue
    Error = 1 << 0,            // 0000 0000 0000 0001
    VersionRequested = 1 << 1, // 0000 0000 0000 0010
    HelpRequested = 1 << 2,    // ...
    ServerHelpRequested = 1 << 3,
    ClientHelpRequested = 1 << 4,
    UsageError = 1 << 5,
    UsageClient = 1 << 6,
    UsageServer = 1 << 7,
    FileNameError = 1 << 8,
    FileNameRequired = 1 << 9,
    OnlyIpRequested = 1 << 10,
    OnlyPortRequested = 1 << 11,
    BindRequired = 1 << 12,
    BindError = 1 << 13,
    IpError = 1 << 14,
    IncomePortError = 1 << 15,
    OutcomePortError = 1 << 16,
    ConnectionTypeRequired = 1 << 17,
    ConnectionTypeError = 1 << 18,
    ConnectionTypeTCP = 1 << 19,
    ConnectionTypeUDP = 1 << 20,
    GuiRequired = 1 << 21,
    VerboseRequired = 1 << 22,
    DebugRequired = 1 << 23,
    NumberError = 1 << 24,
    MaxValue
  };

  // Status statusCode = Status::Ok;
  // my compiler is not that advanced
  // std::optional<QString> status = std::nullopt;
  typedef QMap<Status, QString> StatusType;
  StatusType status = StatusType();
  CmdOptions options = CmdOptions();

public:
  void insertWithUniqueValue(const Status &key, const QString &value);
  bool isSet(Status stat = Status::Ok,
             const Status &statToCheck = Status::Ok) const;
  Status getMaximumStatus();
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

class Cmd {
public:
  // constructors
  explicit Cmd();
  void setup();
  void init();
  void load();
  void evalCmd();

private:
  CmdParseResult::Status status = CmdParseResult::Status::Ok;
  // QMap<CmdParseResult::Status, QString> messages;
  // CmdOptions cmdOptions;
  CmdParseResult cmdParseResult;
  QCommandLineParser parser;
  // Help, Version and other options
  const QCommandLineOption helpOption = parser.addHelpOption();
  const QCommandLineOption versionOption = parser.addVersionOption();
  QString usageSubcommand, connectiontypeSubcommand;
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
  const QStringList usageTypes{"c", "client", "s", "server"};
  const QStringList connectionTypes{"t", "tcp", "u", "udp"};

  // private methods
  CmdParseResult parseCommandLine(QCommandLineParser &parser);
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
