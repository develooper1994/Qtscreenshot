#ifndef COMMANLINEOPTION_H
#define COMMANLINEOPTION_H

#include "defines.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QObject>

typedef struct TagBindInfo {
  QString IpStr = defaultIp;
  uint16_t Port = defaultPort;
} BindInfo;

typedef struct TagCmdOptions {
  bool gui = false;
  int number = 1;
  QString filename = QString(defaultFilename), usage = QString(defaultUsage);
  BindInfo bindInfo;
  // IsSet;
  bool numberIsSet, filenameIsSet, usageIsSet, bindInfoIsSet;
} CmdOptions;

typedef struct TagCmdParseResult {
  enum class Status : std::uint64_t {
    // 0b0000000000000010 // third way
    Ok = 0,                     // 0000 0000 0000 0000 // MinValue
    Error = 1 << 0,             // 0000 0000 0000 0001
    VersionRequested = 1 << 1,  // 0000 0000 0000 0010
    HelpRequested = 1 << 2,     // 0000 0000 0000 0100
    UsageError = 1 << 3,        // 0000 0000 0000 1000
    UsageClient = 1 << 4,       // 0000 0000 0001 0000
    UsageServer = 1 << 5,       // 0000 0000 0010 0000
    FileNameError = 1 << 6,     // 0000 0000 0100 0000
    OnlyIpRequested = 1 << 7,   // 0000 0000 1000 0000
    OnlyPortRequested = 1 << 8, // 0000 0001 0000 0000
    BindError = 1 << 9,         // 0000 0010 0000 0000
    IpError = 1 << 10,          // 0000 0100 0000 0000
    GuiRequired = 1 << 11,      // 0000 1000 0000 0000
    NumberError = 1 << 12,      // 0001 0000 0000 0000
    MaxValue                    // 0001 0000 0000 0001
  };

  // Status statusCode = Status::Ok;
  // my compiler is not that advanced
  // std::optional<QString> status = std::nullopt;
  QMap<Status, QString> status = QMap<Status, QString>();
  CmdOptions options = CmdOptions();

public:
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

void setApplicationSettings();

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
  QMap<CmdParseResult::Status, QString> errorMessages;
  CmdOptions cmdOptions;
  CmdParseResult cmdParseResult;
  QCommandLineParser parser;
  // Help, Version and other options
  const QCommandLineOption helpOption = parser.addHelpOption();
  const QCommandLineOption versionOption = parser.addVersionOption();
  const QList<QCommandLineOption> optionList = {
      {{"g", "gui"}, QApplication::translate("main", "Gui mode.")},
      {{"f", "filename"},
       QApplication::translate("main",
                               "Specify the filename for the screenshot."),
       QApplication::translate("main", "filename"),
       defaultFilename},
      {{"n", "number"},
       QApplication::translate("main", "Specify the number of shots."),
       QApplication::translate("main", "number"),
       defaultNumber},
      {{"u", "usage"},
       QApplication::translate("main",
                               "Specify the type of usage Client/Server."),
       QApplication::translate("main", "usage(client/server)"),
       defaultUsage},
      {{"b", "bind"},
       QApplication::translate("main",
                               "Specify the IP:Port address to send to."),
       QApplication::translate("main", "bind"),
       defaultBind}};
  const QStringList usageTypes{"client", "server"};

  // private methods
  CmdParseResult parseCommandLine(QCommandLineParser &parser);
  bool ipValidate(const QString &ipStr) const;

  // Option Checks
  void guiOptionCheck();
  void numberOptionCheck();
  void bindOptionCheck();
  void usageOptionCheck();

  // Eval Client/Server
  void evalCmdClient();
  void evalCmdServer();
};

#endif // COMMANLINEOPTION_H
