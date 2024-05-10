#include "commanlineoption.h"
#include "defines.h"
#include "dialog.h"

#include <QHostAddress>
#include <QMetaEnum>

using Status = CmdParseResult::Status;

void setApplicationSettings() {
  QApplication::setApplicationName(APPLICATIONNAME);
  QApplication::setApplicationVersion(VERSION);
  QApplication::setApplicationDisplayName(APPLICATIONNAME);
}

// -*-*-*-*-* CmdParseResult *-*-*-*-*-
bool CmdParseResult::isSet(Status stat, const Status &statToCheck) const {
  /*
   * 1011 & 0010 == 0010 => True
   * 0000 & 0000 == 0000 => True
   * stat & statToCheck == stat
   */
  auto a = std::underlying_type_t<Status>(stat);
  auto b = std::underlying_type_t<Status>(statToCheck);
  auto result = ((a & b) == a);
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
  setup();
  init();
  load();
}
void Cmd::setup() {
  parser.setApplicationDescription(QString(HelpMain));
  // QStringList cmdOptionNames = parser.optionNames();
  parser.addOptions(optionList);
}
void Cmd::init() {}
void Cmd::load() {}

inline bool Cmd::ipValidate(const QString &ipStr) const {
  QHostAddress address(ipStr);
  return address.protocol() !=
         QAbstractSocket::NetworkLayerProtocol::UnknownNetworkLayerProtocol;
}

inline void Cmd::guiOptionCheck() {
  cmdOptions.gui = parser.isSet("gui");
  if (cmdOptions.gui) {
    status = status | Status::GuiRequired;
    errorMessages[status] = "";
  }
}

inline void Cmd::numberOptionCheck() {
  bool numberOk;
  cmdOptions.number = parser.value("number").trimmed().toInt(&numberOk);
  cmdOptions.numberIsSet = parser.isSet("number");
  if (!numberOk) {
    status = status | Status::NumberError;
    errorMessages[status] = QString("%1 %2").arg(
        "number parameter cannot parsed. Default:", defaultNumber);
  }
}

inline void Cmd::bindOptionCheck() {
  bool portOk;
  QString bindStr = parser.value("bind").trimmed();
  cmdOptions.bindInfoIsSet = parser.isSet("bind");
  QStringList ipPort = bindStr.split(defaultBindSeperator);
  if (ipPort.count() < 2) {
    status = status | Status::BindError;
    errorMessages[status] =
        QString("%1 %2").arg("You have entered ip and port in a wrong way. "
                             "check it out! || <ip:port>",
                             defaultNumber); // qCritical()
  } else {
    if (!ipValidate(ipPort.at(0))) {
      status = status | Status::IpError;
      errorMessages[status] = QString("%1 %2").arg(
          "Unknown Network Layer Protocol. Please enter a valide ip adress.",
          defaultNumber); // qCritical()
    }
    cmdOptions.bindInfo = {ipPort.at(0),
                           static_cast<uint16_t>(ipPort.at(1).toUInt(&portOk))};
  }
}

inline void Cmd::usageOptionCheck() {
  QStringView usage = parser.value("usage").trimmed();
  cmdOptions.usageIsSet = parser.isSet("number");
  auto usageOptCheck =
      usage.isEmpty() &&
      !usageTypes.contains(usage, Qt::CaseSensitivity::CaseInsensitive);
  QStringView command = usageOptCheck ? QString("") : usage;

  if (usageOptCheck) {
    status = status | Status::UsageError;
    errorMessages[status] =
        QString("%1 %2").arg("Usage Error. Default:", defaultUsage);
    cmdOptions.usage = defaultUsage;
  } else if (command.contains(QStringLiteral("client"),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    status = status | Status::UsageClient;
    errorMessages[status] = "";
  } else if (command.contains(QStringLiteral("server"),
                              Qt::CaseSensitivity::CaseInsensitive)) {
    status = status | Status::UsageServer;
    errorMessages[status] = "";
  } else {
    status = status | Status::Error;
    errorMessages[status] = QString("%1 %2").arg("Unkown ");
  }
}

CmdParseResult Cmd::parseCommandLine(QCommandLineParser &parser) {

  qDebug() << "QApplication::arguments(): " << QApplication::arguments();
  // Process the command line arguments
  // exit immediately
  if (!parser.parse(QApplication::arguments())) {
    status = Status::Error;
    errorMessages[status] = parser.errorText();
    return {errorMessages, cmdOptions};
  }
  if (parser.positionalArguments().count() > MaxPositionalArgs) {
    status = Status::Error;
    errorMessages[status] =
        QStringLiteral("There is no positional argument implementation!");
    return {errorMessages, cmdOptions};
  }
  if (!parser.unknownOptionNames().isEmpty()) {
    status = Status::Error;
    errorMessages[status] = QString("%1").arg("There are unknown parameters: ");
    return {errorMessages, cmdOptions};
  }
  if (parser.isSet(helpOption)) {
    status = Status::HelpRequested;
    errorMessages[status] = "";
    return {errorMessages, cmdOptions};
  }
  if (parser.isSet(versionOption)) {
    status = Status::VersionRequested;
    errorMessages[status] = "";
    return {errorMessages, cmdOptions};
  }

  // Retrieve option values
  cmdOptions.filename = parser.value("filename").trimmed();
  cmdOptions.filenameIsSet = parser.isSet("filename");

  // Gui option check
  guiOptionCheck();

  // Number option check
  numberOptionCheck();

  // Bind option check
  bindOptionCheck();

  // Usage option check
  usageOptionCheck();

  return {errorMessages, cmdOptions};
}

void Cmd::evalCmd() {
  setApplicationSettings();

  QTextStream cout(stdout), cerr(stderr);

  Status statusCode;
  QStringView errorMessage;
  const auto parseResult = parseCommandLine(parser);
  const QMap<Status, QString> &statusTemp = parseResult.status;
  cmdOptions = parseResult.options;

  bool isSet = cmdOptions.usageIsSet || cmdOptions.bindInfoIsSet ||
               cmdOptions.filenameIsSet || cmdOptions.numberIsSet;

  QMapIterator<Status, QString> idx(statusTemp);
  while (idx.hasNext()) {
    idx.next();
    statusCode = idx.key();
    errorMessage = idx.value();
    // errorMessage = errorMessage.isEmpty() ? "" : QString("Unknown error");

    bool errorCheck = parseResult.isSet(statusCode, Status::Error) ||
                      parseResult.isSet(statusCode, Status::UsageError) ||
                      parseResult.isSet(statusCode, Status::FileNameError) ||
                      parseResult.isSet(statusCode, Status::BindError) ||
                      parseResult.isSet(statusCode, Status::IpError) ||
                      parseResult.isSet(statusCode, Status::NumberError);

    // !!! ERROR !!!
    // Ok-Version-Help
    if (parseResult.isSet(statusCode, Status::VersionRequested)) {
      parser.showVersion(); // closes the application
    } else if (parseResult.isSet(statusCode, Status::HelpRequested)) {
      parser.showHelp(0); // closes the application
    }
    // Error - UsageError-FileNameError-BindError-IpError-NumberError
    else if (!isSet || errorCheck) {
      cerr << errorMessage;
      cerr << "\n\n";
      cout << QApplication::applicationVersion();
      cout << parser.helpText();
      cerr.flush();
      cout.flush();
      exit(1);
    }

    // !!! -*-*-* Evaluation Start *-*-*- !!!
    // OnlyIp-OnlyPort
    if (parseResult.isSet(statusCode, Status::OnlyIpRequested)) {
    } else if (parseResult.isSet(statusCode, Status::OnlyPortRequested)) {
    }

    // !!! Client - Server Usage !!!
    if (parseResult.isSet(statusCode, Status::UsageClient)) {
      evalCmdClient();
    } else if (parseResult.isSet(statusCode, Status::UsageServer)) {
      evalCmdServer();
    }

    // Gui
    if (parseResult.isSet(statusCode, Status::GuiRequired)) {
      Dialog w;
      // w.show();
    }
  }
}

void Cmd::evalCmdClient() {}

void Cmd::evalCmdServer() {}
