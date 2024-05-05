#include "commanlineoption.h"
#include <QApplication>

using Status = CommandLineParseResult::Status;

CommandLineParseResult parseCommandLine(QCommandLineParser &parser,
                                        Options &commandOptions) {

  parser.setApplicationDescription(helpString);

  // Help and Version option
  const QCommandLineOption helpOption = parser.addHelpOption();
  const QCommandLineOption versionOption = parser.addVersionOption();

  QCommandLineOption guiOption(QStringList() << "g" << "gui",
                               QApplication::translate("main", "Gui mode."));
  parser.addOption(guiOption);

  // Filename option
  QCommandLineOption filenameOption(
      QStringList() << "f" << "filename",
      QApplication::translate("main",
                              "Specify the filename of the screenshot."),
      QApplication::translate("main", "filename"), "untitled.png");
  parser.addOption(filenameOption);

  // IP option
  QCommandLineOption ipOption(
      QStringList() << "ip",
      QApplication::translate("main", "Specify the IP address to send to."),
      QApplication::translate("main", "ip"));
  parser.addOption(ipOption);

  // Port option
  QCommandLineOption portOption(
      QStringList() << "p" << "port",
      QApplication::translate("main",
                              "Specify the port number for broadcasting."),
      QApplication::translate("main", "port"), "8000");
  parser.addOption(portOption);

  QStringList usageTypes;
  usageTypes << "client" << "server";
  QCommandLineOption usageTypeOption(
      QStringList() << "u" << "usage-type",
      QApplication::translate("main", "Specify usage type (client/server)"),
      "usage-type", "client");
  parser.addOption(usageTypeOption);

  // Process the command line arguments
  if (!parser.parse(QApplication::arguments()))
    return {Status::Error, parser.errorText()};

  // Retrieve option values
  commandOptions.help = parser.isSet(helpOption);
  commandOptions.version = parser.isSet(versionOption);
  commandOptions.gui = parser.isSet(guiOption);
  commandOptions.filename = parser.value(filenameOption);
  commandOptions.ip = parser.value(ipOption);
  commandOptions.port = parser.value(portOption);
  commandOptions.usage = parser.value(usageTypeOption);
  if (!usageTypes.contains(commandOptions.usage)) {
    return {Status::Error,
            QString("You can only choose between client and server!")};
  }

  if (parser.isSet(versionOption))
    return {Status::VersionRequested};
  if (parser.isSet(helpOption))
    return {Status::HelpRequested};

  return {Status::Ok};
}

CommandOptionParser::CommandOptionParser(QObject *parent) : QObject{parent} {
  init();
  parseCommandLine(parser, commandOptions);
}

void CommandOptionParser::init() {
  // Help and Version option
  const QCommandLineOption helpOption = parser.addHelpOption();
  const QCommandLineOption versionOption = parser.addVersionOption();

  QCommandLineOption guiOption(QStringList() << "g" << "gui",
                               QApplication::translate("main", "Gui mode."));
  parser.addOption(guiOption);

  // Filename option
  QCommandLineOption filenameOption(
      QStringList() << "f" << "filename",
      QApplication::translate("main",
                              "Specify the filename of the screenshot."),
      QApplication::translate("main", "filename"), "untitled.png");
  parser.addOption(filenameOption);

  // IP option
  QCommandLineOption ipOption(
      QStringList() << "ip",
      QApplication::translate("main", "Specify the IP address to send to."),
      QApplication::translate("main", "ip"));
  parser.addOption(ipOption);

  // Port option
  QCommandLineOption portOption(
      QStringList() << "p" << "port",
      QApplication::translate("main",
                              "Specify the port number for broadcasting."),
      QApplication::translate("main", "port"), "8000");
  parser.addOption(portOption);

  QStringList usageTypes;
  usageTypes << "client" << "server";
  QCommandLineOption usageTypeOption(
      QStringList() << "u" << "usage-type",
      QApplication::translate("main", "Specify usage type (client/server)"),
      "usage-type", "client");
  parser.addOption(usageTypeOption);
}

void CommandOptionParser::load() {}

void CommandOptionParser::setup() {}

void CommandOptionParser::parse() {
  // Process the command line arguments
  if (!parser.parse(QApplication::arguments()))
    return {Status::Error, parser.errorText()};
}
