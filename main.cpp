#include "dialog.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QStringLiteral>

using namespace std::string_literals;
using Status = CmdParseResult::Status;

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  Dialog dialog;

  return app.exec();
}
