#include "commanlineoption.h"

CommanlineOption::CommanlineOption(QObject *parent) : QObject{parent} {
  init();
}

void CommanlineOption::init() {
  parser.setApplicationDescription(
      "This application takes screenshot and saves the image or send to "
      "another machine if switches activated.");
  parser.addHelpOption();
  parser.addVersionOption();
  // positional argument
  /*
  parser.addPositionalArgument(
      "source", QApplication::translate("main", "Source file to copy."));
  parser.addPositionalArgument(
      "destination", QApplication::translate("main", "Destination directory."));
  */

  // bool arguments
  QCommandLineOption verboseOption(QStringList() << "v"
                                                 << "verbose"
                                                 << "Print more information");
  QCommandLineOption forceOption(QStringList() << "f"
                                               << "force",
                                 "force to override");
  parser.addOption(forceOption);
}

void CommanlineOption::load() {}

void CommanlineOption::setup() {}

void CommanlineOption::parse() {}
