#include "cmd.h"
#include "defines.h"
#include "dialog.h"

#include <QHostAddress>
#include <QMetaEnum>

using Status = CmdParseResult::Status;

// const messages
#define filenameMessage(cmdParseResult)                                        \
  QString("Filename:  %1").arg(cmdParseResult.options.filename)
#define bindRequiredMessage(ipPort)                                            \
  QString("I am only taking these info: %1:%2").arg(ipPort.at(0), ipPort.at(1))
static const QString verboseMessage = "Verbose Mode Selected.";
static const QString debugMessage = "Debug Mode Selected.";
static const QString guiMessage = "Gui Mode Selected.";
static const QString filenameErrorMessage =
    QString("Filename not specified: Default: %1").arg(defaultFilename);
static const QString numberErrorMessage =
    QString("number parameter cannot parsed. Default: %1").arg(defaultNumber);
static const QString usageErrorMessage =
    QString("Usage Error. Default: %1").arg(defaultUsage);
static const QString clientMessage("Client Mod Selected."),
    serverMessage("Server Mod Selected.");
static const QString connectionTypeErrorMessage =
    QString("Connection type error. Default:  %1").arg(defaultConnectionType);
static const QString tcpMessage("Tcp Mod Selected."),
    udpMessage("Udp Mod Selected.");
static const QString bindErrorMessage =
    QString("You have entered ip and port in a wrong way. check it out! || "
            "<ip:port> || Default: %1")
        .arg(defaultBind); // qCritical();
static const QString ipErrorMessage =
    QString("Unknown Network Layer Protocol. Please enter a valide ip "
            "adress.  || <ip:port> || Default: %1")
        .arg(defaultBind); // qCritical();
static const QString incomePortErrorMessage =
    QString("Please enter valid income port number. 0-1024 reserved, please "
            "select "
            "1024>\"Income Port\"<65535. Please enter a valide port "
            "adress.  || <ip:port> || Default: %1")
        .arg(defaultBind); // qCritical();
static const QString outcomePortErrorMessage =
    QString("Please enter valid income port number. 0-1024 reserved, please "
            "select "
            "1024>\"Income Port\"<65535. Please enter a valide port "
            "adress.  || <ip:port> || Default: %1")
        .arg(defaultBind); // qCritical();

void setApplicationSettings() {
  QApplication::setApplicationName(APPLICATIONNAME);
  QApplication::setApplicationVersion(VERSION);
  QApplication::setApplicationDisplayName(APPLICATIONNAME);
}

// -*-*-*-*-* CmdParseResult *-*-*-*-*-
inline void TagCmdParseResult::insertWithUniqueValue(const Status &key,
                                                     const QString &value) {
  // Check if the key already exists in the map
  if (status.values().contains(value)) {
    // qDebug() << "Key" << &key << "already exists with value" <<
    // myMap.value(key);
    return;
  }

  // Insert the key-value pair into the map
  status.insert(key, value);
  // qDebug() << "Inserted key" << &key << "with value" << value;
}

bool CmdParseResult::isSet(Status stat, const Status &statToCheck) const {
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

CmdParseResult::Status CmdParseResult::getMaximumStatus() {
  /*
  QMetaEnum statMetaEnum = QMetaEnum::fromType<TagCmdParseResult::Status>();
uint64_t statMetaEnumCount = statMetaEnum.keyCount();
return statMetaEnum.value(statMetaEnumCount);
*/

  return static_cast<TagCmdParseResult::Status>(
      std::underlying_type_t<TagCmdParseResult::Status>(
          TagCmdParseResult::Status::MaxValue) -
      1);
}

// -*-*-*-*-* Cmd *-*-*-*-*-
Cmd::Cmd() {
  init();
  setup();
  load();
}
void Cmd::init() {
  parser.addPositionalArgument("subcommand", "client\n"
                                             "server\n");
  // QStringList cmdOptionNames = parser.optionNames();
  parser.addOptions(optionList);
}
void Cmd::setup() {
  parser.setOptionsAfterPositionalArgumentsMode(
      QCommandLineParser::ParseAsOptions);
  parser.setApplicationDescription(QString(HelpMain));
}
void Cmd::load() {}

inline bool Cmd::ipValidate(const QString &ipStr) const {
  QHostAddress address(ipStr);
  return address.protocol() !=
         QAbstractSocket::NetworkLayerProtocol::UnknownNetworkLayerProtocol;
}

inline void Cmd::verboseOptionCheck() {
  cmdParseResult.options.verbose = parser.isSet("verbose");

  if (cmdParseResult.options.verbose) {

    status = status | Status::VerboseRequired;
    // cmdParseResult.status[status] = verboseMessage;

    cmdParseResult.insertWithUniqueValue(status, verboseMessage);
  }
}

inline void Cmd::debugOptionCheck() {
  cmdParseResult.options.debug = parser.isSet("debug");

  if (cmdParseResult.options.debug) {
    status = status | Status::DebugRequired;
    // cmdParseResult.status[status] = debugMessage;
    cmdParseResult.insertWithUniqueValue(status, debugMessage);
  }
}

inline void Cmd::guiOptionCheck() {
  cmdParseResult.options.gui = parser.isSet("gui");

  if (cmdParseResult.options.gui) {
    status = status | Status::GuiRequired;
    // cmdParseResult.status[status] = guiMessage;
    cmdParseResult.insertWithUniqueValue(status, guiMessage);
  }
}

inline void Cmd::filenameOptionCheck() {
  cmdParseResult.options.filename = parser.value("filename").trimmed();
  cmdParseResult.options.filenameIsSet = parser.isSet("filename");

  if (cmdParseResult.options.filenameIsSet) {
    status = status | Status::FileNameRequired;
    // cmdParseResult.status[status] = filenameMessage(cmdParseResult);
    cmdParseResult.insertWithUniqueValue(status,
                                         filenameMessage(cmdParseResult));
  } else {
    status = status | Status::FileNameError;
    // cmdParseResult.status[status] = filenameErrorMessage;
    cmdParseResult.insertWithUniqueValue(status, filenameErrorMessage);
  }
}

inline void Cmd::numberOptionCheck() {
  bool numberOk;
  cmdParseResult.options.number =
      parser.value("number").trimmed().toInt(&numberOk);
  cmdParseResult.options.numberIsSet = parser.isSet("number");

  if (!numberOk) {
    status = status | Status::NumberError;
    // cmdParseResult.status[status] = numberErrorMessage;
    cmdParseResult.insertWithUniqueValue(status, numberErrorMessage);
  }
}

inline void Cmd::usageOptionCheck() {
  QStringView usage = parser.value("usage").trimmed();
  cmdParseResult.options.usageIsSet = parser.isSet("usage");
  bool usageOptCheck =
      usage.isEmpty() &&
      !usageTypes.contains(usage, Qt::CaseSensitivity::CaseInsensitive);
  QStringView command = usageOptCheck ? QString("") : usage;

  if (command.contains(usageTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    status = status | Status::UsageClient;
    // cmdParseResult.status[status] = clientMessage;
    cmdParseResult.insertWithUniqueValue(status, clientMessage);
  } else if (command.contains(usageTypes.at(3),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.server = true;
    status = status | Status::UsageServer;
    // cmdParseResult.status[status] = serverMessage;
    cmdParseResult.insertWithUniqueValue(status, serverMessage);
  } else {
    status = status | Status::Error;
    // cmdParseResult.status[status] = usageErrorMessage;
    cmdParseResult.options.usage = defaultUsage;
    cmdParseResult.insertWithUniqueValue(status, usageErrorMessage);
  }
}

inline void Cmd::connectiontypeOptionCheck() {
  QStringView connectionType = parser.value("connection-type").trimmed();
  cmdParseResult.options.connectiontypeIsSet = parser.isSet("connection-type");
  bool connectionTypeOptCheck =
      connectionType.isEmpty() &&
      !connectionTypes.contains(connectionType,
                                Qt::CaseSensitivity::CaseInsensitive);
  QStringView command = connectionTypeOptCheck ? QString("") : connectionType;

  if (!command.compare(connectionTypes.at(1),
                       Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    status = status | Status::ConnectionTypeTCP;
    // cmdParseResult.status[status] = tcpMessage;
    cmdParseResult.insertWithUniqueValue(status, tcpMessage);
  } else if (!command.compare(connectionTypes.at(3),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    status = status | Status::ConnectionTypeUDP;
    // cmdParseResult.status[status] = udpMessage;
    cmdParseResult.insertWithUniqueValue(status, udpMessage);
  } else {
    status = status | Status::Error;
    // cmdParseResult.status[status] = connectionTypeErrorMessage;
    cmdParseResult.options.usage = defaultConnectionType;
    cmdParseResult.insertWithUniqueValue(status, connectionTypeErrorMessage);
  }
}

inline void Cmd::bindOptionCheck() {
  bool portOk;
  QString bindStr = parser.value("bind").trimmed();
  cmdParseResult.options.bindInfoIsSet = parser.isSet("bind");
  QStringList ipPort = bindStr.split(defaultBindSeperator);

  if (ipPort.count() < 2) {
    status = status | Status::BindError;
    // cmdParseResult.status[status] = bindErrorMessage;
    cmdParseResult.insertWithUniqueValue(status, bindErrorMessage);
    return;
  } else {
    if (!ipValidate(ipPort.at(0))) {
      status = status | Status::IpError;
      // cmdParseResult.status[status] = ipErrorMessage;
      cmdParseResult.insertWithUniqueValue(status, ipErrorMessage);
      return;
    }
    status = status | Status::BindRequired;
    // cmdParseResult.status[status] = bindRequiredMessage(ipPort);
    cmdParseResult.insertWithUniqueValue(status, bindRequiredMessage(ipPort));
    cmdParseResult.options.bindInfo.IpStr = ipPort.at(0);

    if (ipPort.count() == 2) {
      const auto IncomePort = ipPort.at(1).toUInt(&portOk);
      if ((IncomePort > 65535) && !portOk) {
        status = status | Status::IncomePortError;
        // cmdParseResult.status[status] = incomePortErrorMessage;
        cmdParseResult.insertWithUniqueValue(status, incomePortErrorMessage);
      } else {
        cmdParseResult.options.bindInfo.IncomePort =
            static_cast<uint16_t>(ipPort.at(1).toUInt(&portOk));
      }
    }

    // !not that important!
    if (ipPort.count() > 2) {
      const auto OutcomePort = ipPort.at(2).toUInt(&portOk);
      if ((OutcomePort > 65535) && !portOk) {
        status = status | Status::OutcomePortError;
        // cmdParseResult.status[status] = outcomePortErrorMessage;
        cmdParseResult.insertWithUniqueValue(status, outcomePortErrorMessage);
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
    status = Status::Error;
    cmdParseResult.status[status] = "Missing subcommand!";
    //     cmdParseResult.status[status] = QStringLiteral("There is no
    //     positional argument implementation!");
    return;
  }

  usageSubcommand =
      positionalArguments.count() > 0 ? positionalArguments.at(0) : QString();
  connectiontypeSubcommand =
      positionalArguments.count() > 1 ? positionalArguments.at(1) : QString();

  // usage as subcommand
  if (!usageSubcommand.compare(usageTypes.at(1),
                               Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.connectiontypeIsSet = true;
    cmdParseResult.options.client = true;
    cmdParseResult.options.server = false;
    status = status | Status::UsageClient;
    // cmdParseResult.status[status] = clientMessage;
    cmdParseResult.insertWithUniqueValue(status, clientMessage);
  } else if (!usageSubcommand.compare(usageTypes.at(3),
                                      Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.connectiontypeIsSet = true;
    cmdParseResult.options.client = false;
    cmdParseResult.options.server = true;
    status = status | Status::UsageServer;
    // cmdParseResult.status[status] = serverMessage;
    cmdParseResult.insertWithUniqueValue(status, serverMessage);
  }
  /*
  else {
    status = status | Status::Error;
    cmdParseResult.status[status] = usageErrorMessage;
    cmdParseResult.options.usage = defaultUsage;
  }
  */

  // connection-type as subcommand
  if (!connectiontypeSubcommand.compare(connectionTypes.at(1),
                                        Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    status = status | Status::ConnectionTypeTCP;
    // cmdParseResult.status[status] = tcpMessage;
    cmdParseResult.insertWithUniqueValue(status, tcpMessage);
  } else if (!connectiontypeSubcommand.compare(
                 connectionTypes.at(3), Qt::CaseSensitivity::CaseInsensitive)) {
    cmdParseResult.options.client = true;
    status = status | Status::ConnectionTypeUDP;
    // cmdParseResult.status[status] = udpMessage;
    cmdParseResult.insertWithUniqueValue(status, udpMessage);
  }
  /*
  else {
    status = status | Status::Error;
    cmdParseResult.status[status] = connectionTypeErrorMessage;
    cmdParseResult.options.usage = defaultConnectionType;
  }
  */
}

CmdParseResult Cmd::parseCommandLine(QCommandLineParser &parser) {

  // Process the command line arguments
  // exit immediately

  QStringList arguments = QApplication::arguments();
  bool parseResult = !parser.parse(arguments);
  QStringList optionNames = parser.optionNames();
  QStringList unknownOptions = parser.unknownOptionNames();

  if (parseResult) {
    // status = Status::Error;
    // cmdParseResult.status[status] = parser.errorText();
    cmdParseResult.insertWithUniqueValue(status, parser.errorText());
    return {cmdParseResult.status, cmdParseResult.options};
  } else if (!unknownOptions.isEmpty()) {
    // status = Status::Error;
    // cmdParseResult.status[status] = QString("%1 %2").arg(
    //     "There are unknown options: ", unknownOptions.join(" "));
    cmdParseResult.insertWithUniqueValue(
        status, QString("%1 %2").arg("There are unknown options: ",
                                     unknownOptions.join(" ")));
    return {cmdParseResult.status, cmdParseResult.options};
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
    status = Status::HelpRequested;
    // cmdParseResult.status[status] = ""; // parser.helpText();
    cmdParseResult.insertWithUniqueValue(status, "");
    return {cmdParseResult.status, cmdParseResult.options};
  }
  if (parser.isSet(versionOption)) {
    // parser.showVersion(); // closes the application
    status = Status::VersionRequested;
    // cmdParseResult.status[status] = ""; // VERSION;
    cmdParseResult.insertWithUniqueValue(status, "");
    return {cmdParseResult.status, cmdParseResult.options};
  }

  return cmdParseResult;
}

void Cmd::evalCmd() {
  setApplicationSettings();

  Status statusCode;
  QStringView message;
  const CmdParseResult parseResult = parseCommandLine(parser);
  const QMap<Status, QString> &statusTemp = parseResult.status;
  const CmdOptions &cmdOptions = parseResult.options;

  bool anySet = cmdOptions.usageIsSet || cmdOptions.connectiontypeIsSet ||
                cmdOptions.bindInfoIsSet || cmdOptions.filenameIsSet ||
                cmdOptions.numberIsSet || cmdOptions.verbose ||
                cmdOptions.verbose || cmdOptions.gui;

  QMapIterator<Status, QString> idx(statusTemp);
  while (idx.hasNext()) {
    idx.next();
    statusCode = idx.key();
    message = idx.value();
    // errorMessage = errorMessage.isEmpty() ? "" : QString("Unknown error");

    bool allErrorCheck =
        parseResult.isSet(statusCode, Status::Error) &&
        parseResult.isSet(statusCode, Status::UsageError) &&
        parseResult.isSet(statusCode, Status::FileNameError) &&
        parseResult.isSet(statusCode, Status::BindError) &&
        parseResult.isSet(statusCode, Status::IpError) &&
        parseResult.isSet(statusCode, Status::IncomePortError) &&
        parseResult.isSet(statusCode, Status::OutcomePortError) &&
        parseResult.isSet(statusCode, Status::ConnectionTypeError) &&
        parseResult.isSet(statusCode, Status::NumberError);

    // !!! ERROR !!!
    // Ok-Version-Help

    if (!message.isEmpty()) {
      qWarning() << message;
    }

    if (parseResult.isSet(statusCode, Status::VersionRequested)) {
      parser.showVersion(); // closes the application
    }
    if (parseResult.isSet(statusCode, Status::HelpRequested)) {
      // parser.showHelp(0);
      qDebug() << QApplication::applicationVersion();
      if (cmdParseResult.options.client) {
        qDebug() << "HelpClient";
        qDebug() << HelpClient;
      } else if (cmdParseResult.options.server) {
        qDebug() << "HelpServer";
        qDebug() << HelpServer;
      } else {
        qDebug() << "HelpMain";
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
        if (parseResult.isSet(statusCode, Status::VerboseRequired)) {
        }

        // !!! -*-*-* Evaluation Start *-*-*- !!!
        // OnlyIp-OnlyPort
        if (parseResult.isSet(statusCode, Status::OnlyIpRequested)) {
        } else if (parseResult.isSet(statusCode, Status::OnlyPortRequested)) {
        }
    */

    // !!! Client - Server Usage !!!
    if (parseResult.isSet(statusCode, Status::UsageClient)) {
      MainClient();
    } else if (parseResult.isSet(statusCode, Status::UsageServer)) {
      MainServer();
    }

    // Gui
    if (parseResult.isSet(statusCode, Status::GuiRequired)) {
      Dialog w;
      w.show();
    }
  }
}

void Cmd::MainClient() { qDebug() << Q_FUNC_INFO; }

void Cmd::MainServer() { qDebug() << Q_FUNC_INFO; }
