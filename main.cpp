#include "cmd.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QStringLiteral>

using namespace std::string_literals;
using Status = CmdParseResult::Status;

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  Cmd cmd;
  cmd.evalCmd();

  return app.exec();
}
