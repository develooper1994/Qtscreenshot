#include "commanlineoption.h"
#include "dialog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QStringLiteral>

using namespace std::string_literals;

void setApplicationSettings() {
  QApplication::setApplicationName("QtScreenShoot");
  QApplication::setApplicationVersion("1.0");
  QApplication::setApplicationDisplayName("ScreenShoot");
}

int main(int argc, char *argv[]) {

  using Status = CommandLineParseResult::Status;

  QApplication app(argc, argv);
  setApplicationSettings();

  QCommandLineParser parser;
  Options options;
  QString errorMessage;
  CommandLineParseResult parseResult = parseCommandLine(parser, options);
  switch (parseResult.statusCode) {
  case Status::Ok:
    break;
  case Status::Error:
    errorMessage = parseResult.errorString.isEmpty()
                       ? parseResult.errorString
                       : QString("Unknown error occurred");
    fputs(qPrintable(errorMessage), stderr);
    fputs("\n\n", stderr);
    fputs(qPrintable(parser.helpText()), stderr);
    break;
  case Status::VersionRequested:
    parser.showVersion();
    break;
  case Status::HelpRequested:
    parser.showHelp();
    break;
  default:
    break;
  }

  Dialog w;
  // w.show();
  return app.exec();
}
