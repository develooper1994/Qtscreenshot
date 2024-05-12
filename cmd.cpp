#include "cmd.h"
#include "defines.h"

#include <QHostAddress>
#include <QMetaEnum>

using Status = CmdParseResult::Status;

static const QStringList usageTypes{"c", "client", "s", "server"};
static const QStringList connectionTypes{"t", "tcp", "u", "udp"};

// lookup table
static const QMap<Status, QString> __statusToMessage({
    // runtime
    // {Status::FileNameRequired, __FilenameMessage(cmdParseResult)},
    // {Status::BindRequired, __BindRequiredMessage(ipPort)},
    // {Status::ParserError, __ParserError},
    // compiletime
    {Status::Error, __Unkown},
    {Status::PositionalArgumentsError, __PositionalArgumentsErrorMessage},
    {Status::HelpRequested, ""},
    {Status::VersionRequested, ""},
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
});

inline void setApplicationSettings() {
  QApplication::setApplicationName(APPLICATIONNAME);
  QApplication::setApplicationVersion(VERSION);
  QApplication::setApplicationDisplayName(APPLICATIONNAME);
}

// -*-*-*-*-* CmdParseResult *-*-*-*-*-
void CmdParseResult::insertUniqueValue(const Status statToSet) {
  insertUniqueValue(this->status, statToSet);
}
void CmdParseResult::insertUniqueValue(Status &key) {
  const QString &value = __statusToMessage[key];
  insertUniqueValue(key, value);
}
void CmdParseResult::insertUniqueValue(Status &key, const Status statToSet) {
  const QString &value = __statusToMessage[key];
  insertUniqueValue(key, value, statToSet);
}
inline void CmdParseResult::insertUniqueValue(const Status &key,
                                              const QString &value) {
  // Check if the key already exists in the map
  if (statusMessage.values().contains(value)) {
    // qDebug() << "Key" << &key << "already exists with value" <<
    // myMap.value(key);
    return;
  }
  // Insert the key-value pair into the map
  statusMessage.insert(key, value);
  // qDebug() << "Inserted key" << &key << "with value" << value;
}
inline void CmdParseResult::insertUniqueValue(Status &key, const QString &value,
                                              const Status statToSet) {
  // Check if the key already exists in the map
  if (statusMessage.values().contains(value)) {
    // qDebug() << "Key" << &key << "already exists with value" <<
    // myMap.value(key);
    return;
  }
  // set stat
  setStatus(key, statToSet);
  // Insert the key-value pair into the map
  statusMessage.insert(key, value);
  // qDebug() << "Inserted key" << &key << "with value" << value;
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

inline bool Cmd::ipValidate(const QString &ipStr) const {
  QHostAddress address(ipStr);
  return address.protocol() !=
         QAbstractSocket::NetworkLayerProtocol::UnknownNetworkLayerProtocol;
}

inline void Cmd::verboseOptionCheck() {
  cmdParseResult.options.verbose = parser.isSet("verbose");

  if (cmdParseResult.options.verbose) {
    // status = status | Status::VerboseRequired;
    // cmdParseResult.status[status] = verboseMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __VerboseMessage,
                                     Status::VerboseRequired);
  }
}

inline void Cmd::debugOptionCheck() {
  cmdParseResult.options.debug = parser.isSet("debug");

  if (cmdParseResult.options.debug) {
    // status = status | Status::DebugRequired;
    // cmdParseResult.status[status] = debugMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __DebugMessage,
                                     Status::DebugRequired);
  }
}

inline void Cmd::guiOptionCheck() {
  cmdParseResult.options.gui = parser.isSet("gui");

  if (cmdParseResult.options.gui) {
    // status = status | Status::GuiRequired;
    // cmdParseResult.status[status] = guiMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __GuiMessage,
                                     Status::GuiRequired);
  }
}

inline void Cmd::filenameOptionCheck() {
  cmdParseResult.options.filename = parser.value("filename").trimmed();
  cmdParseResult.options.filenameIsSet = parser.isSet("filename");

  if (cmdParseResult.options.filenameIsSet) {
    // status = status | Status::FileNameRequired;
    // cmdParseResult.status[status] = filenameMessage(cmdParseResult);
    cmdParseResult.insertUniqueValue(cmdParseResult.status,
                                     __FilenameMessage(cmdParseResult),
                                     Status::FileNameRequired);
  } else {
    // status = status | Status::FileNameError;
    // cmdParseResult.status[status] = filenameErrorMessage;
    cmdParseResult.insertUniqueValue(
        cmdParseResult.status, __FilenameErrorMessage, Status::FileNameError);
  }
}

inline void Cmd::numberOptionCheck() {
  bool numberOk;
  cmdParseResult.options.number =
      parser.value("number").trimmed().toInt(&numberOk);
  cmdParseResult.options.numberIsSet = parser.isSet("number");

  if (!numberOk) {
    // status = status | Status::NumberError;
    // cmdParseResult.status[status] = numberErrorMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status,
                                     __NumberErrorMessage, Status::NumberError);
  }
}

inline void Cmd::usageOptionCheck() {
  QStringView usage = parser.value("usage").trimmed();
  cmdParseResult.options.usageIsSet = parser.isSet("usage");
  bool usageOptCheck =
      usage.isEmpty() &&
      !usageTypes.contains(usage, Qt::CaseSensitivity::CaseInsensitive);
  QStringView command = usageOptCheck ? QString() : usage;

  if (command.contains(usageTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    // status = status | Status::UsageClient;
    // cmdParseResult.status[status] = clientMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __ClientMessage,
                                     Status::UsageClient);
  } else if (command.contains(usageTypes.at(3),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.server = true;
    // status = status | Status::UsageServer;
    // cmdParseResult.status[status] = serverMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __ServerMessage,
                                     Status::UsageServer);
  } else {
    // status = status | Status::UsageError;
    // cmdParseResult.status[status] = usageErrorMessage;
    cmdParseResult.options.usage = defaultUsage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __UsageErrorMessage,
                                     Status::UsageError);
  }
}

inline void Cmd::connectiontypeOptionCheck() {
  QStringView connectionType = parser.value("connection-type").trimmed();
  cmdParseResult.options.connectiontypeIsSet = parser.isSet("connection-type");
  bool connectionTypeOptCheck =
      connectionType.isEmpty() &&
      !connectionTypes.contains(connectionType,
                                Qt::CaseSensitivity::CaseInsensitive);
  QStringView command = connectionTypeOptCheck ? QString() : connectionType;

  if (!command.compare(connectionTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    // status = status | Status::ConnectionTypeTCP;
    // cmdParseResult.status[status] = tcpMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __TcpMessage,
                                     Status::ConnectionTypeTCP);
  } else if (!command.compare(connectionTypes.at(3),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    // status = status | Status::ConnectionTypeUDP;
    // cmdParseResult.status[status] = udpMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __UdpMessage,
                                     Status::ConnectionTypeUDP);
  } else {
    // status = status | Status::ConnectionTypeError;
    // cmdParseResult.status[status] = connectionTypeErrorMessage;
    cmdParseResult.options.usage = defaultConnectionType;
    cmdParseResult.insertUniqueValue(cmdParseResult.status,
                                     __ConnectionTypeErrorMessage,
                                     Status::ConnectionTypeError);
  }
}

inline void Cmd::bindOptionCheck() {
  bool portOk;
  QString bindStr = parser.value("bind").trimmed();
  cmdParseResult.options.bindInfoIsSet = parser.isSet("bind");
  QStringList ipPort = bindStr.split(defaultBindSeperator);

  if (ipPort.count() < 2) {
    // status = status | Status::BindError;
    // cmdParseResult.status[status] = bindErrorMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __BindErrorMessage,
                                     Status::BindError);
    return;
  } else {
    if (!ipValidate(ipPort.at(0))) {
      // status = status | Status::IpError;
      // cmdParseResult.status[status] = ipErrorMessage;
      cmdParseResult.insertUniqueValue(cmdParseResult.status, __IpErrorMessage,
                                       Status::IpError);
      return;
    }
    // status = status | Status::BindRequired;
    // cmdParseResult.status[status] = bindRequiredMessage(ipPort);
    cmdParseResult.insertUniqueValue(cmdParseResult.status,
                                     __BindRequiredMessage(ipPort),
                                     Status::BindRequired);
    cmdParseResult.options.bindInfo.IpStr = ipPort.at(0);

    if (ipPort.count() == 2) {
      const auto IncomePort = ipPort.at(1).toUInt(&portOk);
      if ((IncomePort > 65535) && !portOk) {
        // status = status | Status::IncomePortError;
        // cmdParseResult.status[status] = incomePortErrorMessage;
        cmdParseResult.insertUniqueValue(cmdParseResult.status,
                                         __IncomePortErrorMessage,
                                         Status::IncomePortError);
      } else {
        cmdParseResult.options.bindInfo.IncomePort =
            static_cast<uint16_t>(ipPort.at(1).toUInt(&portOk));
      }
    }

    // !not that important!
    if (ipPort.count() > 2) {
      const auto OutcomePort = ipPort.at(2).toUInt(&portOk);
      if ((OutcomePort > 65535) && !portOk) {
        // status = status | Status::OutcomePortError;
        // cmdParseResult.status[status] = outcomePortErrorMessage;
        cmdParseResult.insertUniqueValue(cmdParseResult.status,
                                         __OutcomePortErrorMessage,
                                         Status::OutcomePortError);
      } else {
        cmdParseResult.options.bindInfo.OutcomePort =
            static_cast<uint16_t>(ipPort.at(2).toUInt(&portOk));
      }
    }
  }
}

inline void Cmd::optionChecks() {
  // Debug option check
  debugOptionCheck();
  // Verbose option check
  verboseOptionCheck();
  // Gui option check
  guiOptionCheck();
  // Filename option check
  filenameOptionCheck();
  // Number option check
  numberOptionCheck();
  // Usage option check
  // usageOptionCheck();
  // Bind option check
  bindOptionCheck();
  // ConnectionType option check
  connectiontypeOptionCheck();
}

inline void Cmd::subcommandCheck() {
  QStringList positionalArguments = parser.positionalArguments();
  if (cmdParseResult.options.debug) {
    qDebug() << "positionalArguments: " << positionalArguments;
  }

  if (positionalArguments.isEmpty()) {
    // status = Status::PositionalArgumentsError;
    // cmdParseResult.status[status] =
    //    QStringLiteral("There is no positional argument implementation!");
    cmdParseResult.insertUniqueValue(cmdParseResult.status,
                                     __PositionalArgumentsErrorMessage,
                                     Status::PositionalArgumentsError);
    return;
  }

  QString usageSubcommand =
      positionalArguments.count() > 0 ? positionalArguments.at(0) : QString();
  QString connectiontypeSubcommand =
      positionalArguments.count() > 1 ? positionalArguments.at(1) : QString();

  // usage as subcommand
  if (!usageSubcommand.compare(usageTypes.at(1),
                               Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.connectiontypeIsSet = true;
    cmdParseResult.options.client = true;
    cmdParseResult.options.server = false;
    // status = status | Status::UsageClient;
    // cmdParseResult.status[status] = clientMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __ClientMessage,
                                     Status::UsageClient);
  } else if (!usageSubcommand.compare(usageTypes.at(3),
                                      Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.connectiontypeIsSet = true;
    cmdParseResult.options.client = false;
    cmdParseResult.options.server = true;
    // status = status | Status::UsageServer;
    // cmdParseResult.status[status] = serverMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __ServerMessage,
                                     Status::UsageServer);
  }
  /*
  else {
    // status = status | Status::UsageError;
    // cmdParseResult.status[status] = usageErrorMessage;
    cmdParseResult.insertWithUniqueValue(status, __UsageErrorMessage,
            Status::UsageError); cmdParseResult.options.usage = defaultUsage;
  }
  */

  // connection-type as subcommand
  if (!connectiontypeSubcommand.compare(connectionTypes.at(1),
                                        Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    // status = status | Status::ConnectionTypeTCP;
    // cmdParseResult.status[status] = tcpMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __TcpMessage,
                                     Status::ConnectionTypeTCP);
  } else if (!connectiontypeSubcommand.compare(
                 connectionTypes.at(3), Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    // status = status | Status::ConnectionTypeUDP;
    // cmdParseResult.status[status] = udpMessage;
    cmdParseResult.insertUniqueValue(cmdParseResult.status, __UdpMessage,
                                     Status::ConnectionTypeUDP);
  }
  /*
  else {
    // status = status | Status::ConnectionTypeError;
    // cmdParseResult.status[status] = __ConnectionTypeErrorMessage;
    cmdParseResult.options.usage = defaultConnectionType;
    cmdParseResult.insertWithUniqueValue(status, __ConnectionTypeErrorMessage,
  Status::ConnectionTypeError);
  }
  */
}

inline void Cmd::parseCommandLine() {
  // Process the command line arguments
  // exit immediately

  QStringList arguments = QApplication::arguments();
  bool parseResult = !parser.parse(arguments);
  QStringList optionNames = parser.optionNames(),
              unknownOptions = parser.unknownOptionNames();

  if (parseResult) {
    // status = Status::ParseError;
    // cmdParseResult.status[status] = __ParserError;
    cmdParseResult.insertUniqueValue(Status::ParseError, __ParserError);
    cmdParseResult = {cmdParseResult.statusMessage, cmdParseResult.options};
    return;
  } else if (!unknownOptions.isEmpty()) {
    // status = Status::ParseError;
    // cmdParseResult.status[status] = QString("%1 %2").arg(
    //     "There are unknown options: ", unknownOptions.join(" "));
    cmdParseResult.insertUniqueValue(
        Status::ParseError, QString("%1 %2").arg("There are unknown options: ",
                                                 unknownOptions.join(" ")));
    cmdParseResult = {cmdParseResult.statusMessage, cmdParseResult.options};
    return;
  }

  // Retrieve option values
  optionChecks();
  subcommandCheck();

  if (cmdParseResult.options.debug) {
    qDebug() << "arguments: " << arguments;
    qDebug() << "optionNames: " << optionNames;
    qDebug() << "unknownOption: " << unknownOptions;
  }

  if (parser.isSet(helpOption)) {
    /*
    qDebug() << QApplication::applicationVersion();
    if (cmdParseResult.options.client) {
      qDebug() << HelpClient;
    } else if (cmdParseResult.options.server) {
      qDebug() << HelpServer;
    } else {
      qDebug() << HelpMain;
    }
    exit(0);
    */
    // status = Status::HelpRequested;
    // cmdParseResult.status[status] = ""; // parser.helpText();
    cmdParseResult.insertUniqueValue(Status::HelpRequested, "");
    cmdParseResult = {cmdParseResult.statusMessage, cmdParseResult.options};
    return;
  }
  if (parser.isSet(versionOption)) {
    // parser.showVersion(); // closes the application
    // status = Status::VersionRequested;
    // cmdParseResult.status[status] = ""; // VERSION;
    cmdParseResult.insertUniqueValue(Status::VersionRequested, "");
    cmdParseResult = {cmdParseResult.statusMessage, cmdParseResult.options};
    return;
  }
}

void Cmd::evalCmd() {
  setApplicationSettings();
  if (cmdParseResult.statusMessage.isEmpty()) {
    parseCommandLine();
  }

  Status statusCode;
  QStringView message;
  const QMap<Status, QString> &statusTemp = cmdParseResult.statusMessage;
  const CmdOptions &cmdOptions = cmdParseResult.options;

  bool anySet = cmdOptions.usageIsSet || cmdOptions.connectiontypeIsSet ||
                cmdOptions.bindInfoIsSet || cmdOptions.filenameIsSet ||
                cmdOptions.numberIsSet || cmdOptions.verbose ||
                cmdOptions.verbose || cmdOptions.gui || cmdOptions.debug;

  QMapIterator<Status, QString> statusTempIterator(statusTemp);
  while (statusTempIterator.hasNext()) {
    statusTempIterator.next();
    statusCode = statusTempIterator.key();
    message = statusTempIterator.value();
    // errorMessage = errorMessage.isEmpty() ? "" : QString("Unknown error");

    bool allErrorCheck =
        isStatusSet(statusCode, Status::Error) &&
        isStatusSet(statusCode, Status::SubcommandError) &&
        isStatusSet(statusCode, Status::PositionalArgumentsError) &&
        isStatusSet(statusCode, Status::ParseError) &&
        isStatusSet(statusCode, Status::UsageError) &&
        isStatusSet(statusCode, Status::FileNameError) &&
        isStatusSet(statusCode, Status::BindError) &&
        isStatusSet(statusCode, Status::IpError) &&
        isStatusSet(statusCode, Status::IncomePortError) &&
        isStatusSet(statusCode, Status::OutcomePortError) &&
        isStatusSet(statusCode, Status::ConnectionTypeError) &&
        isStatusSet(statusCode, Status::NumberError);

    // !!! ERROR !!!
    // Ok-Version-Help

    if (!message.isEmpty()) {
      qWarning() << message;
    }

    if (isStatusSet(statusCode, Status::VersionRequested)) {
      parser.showVersion(); // closes the application
    }
    if (isStatusSet(statusCode, Status::HelpRequested)) {
      // parser.showHelp(0);
      qDebug() << QApplication::applicationVersion();
      if (cmdParseResult.options.client) {
        qDebug() << "-*-*-*-*-* HelpClient *-*-*-*-*-";
        qDebug() << HelpClient;
      } else if (cmdParseResult.options.server) {
        qDebug() << "-*-*-*-*-* HelpServer *-*-*-*-*-";
        qDebug() << HelpServer;
      } else {
        qDebug() << "-*-*-*-*-* HelpMain *-*-*-*-*-";
        qDebug() << HelpMain;
      }
      exit(0);
    }
    // Error - UsageError-FileNameError-BindError-IpError-NumberError

    if (!anySet || allErrorCheck) {
      // There is an error
      // show HelpGeneral
      qDebug() << QApplication::applicationVersion() << "\n";
      parser.showHelp(1);
    }

    /*
        // Verbose
        if (isSet(statusCode, Status::VerboseRequired)) {
        }

        // !!! -*-*-* Evaluation Start *-*-*- !!!
        // OnlyIp-OnlyPort
        if (isSet(statusCode, Status::OnlyIpRequested)) {
        } else if (isSet(statusCode, Status::OnlyPortRequested)) {
        }
    */

    // !!! Client - Server Usage !!!
    if (isStatusSet(statusCode, Status::UsageClient)) {
      MainClient();
    } else if (isStatusSet(statusCode, Status::UsageServer)) {
      MainServer();
    }

    // Gui
    if (isStatusSet(statusCode, Status::GuiRequired)) {
      w.show();
    }
  }
}

void Cmd::MainClient() { qDebug() << Q_FUNC_INFO; }

void Cmd::MainServer() { qDebug() << Q_FUNC_INFO; }

// -*-*-*-*-* Functions, Not a Class *-*-*-*-*-
inline void setStatus(CmdParseResult::Status &stat,
                      const CmdParseResult::Status statToSet) {
  stat = stat | statToSet;
}
inline bool isStatusSet(CmdParseResult::Status stat,
                        const CmdParseResult::Status &statToCheck) {
  /*
   * 1011 & 0010 == 0010 => True
   * 0000 & 0000 == 0000 => True
   * stat & statToCheck == stat
   */
  auto a = std::underlying_type_t<Status>(stat);
  auto b = std::underlying_type_t<Status>(statToCheck);
  auto result = ((a & b) == b);
  return result;
}
QString getStatusMessage(const CmdParseResult::Status &statToCheck) {
  const QString &message = __statusToMessage[statToCheck];
  return message.isEmpty() ? QString(__Unkown) : message;
}
inline CmdParseResult::Status getMaximumStatus() {
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
