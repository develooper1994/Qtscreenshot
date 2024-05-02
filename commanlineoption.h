#ifndef COMMANLINEOPTION_H
#define COMMANLINEOPTION_H

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QObject>

class CommanlineOption : public QObject {
  Q_OBJECT
public:
  // option flags
  // constructors
  explicit CommanlineOption(QObject *parent = nullptr);
  void init();
  void load();
  void setup();
  void parse();

signals:
private:
  QCommandLineParser parser;
};

#endif // COMMANLINEOPTION_H
