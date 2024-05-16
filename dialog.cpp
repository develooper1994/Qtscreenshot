#include "dialog.h"
#include "ui_dialog.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog) {
  init();
  setup();
  load();

  const CmdOptions &options = cmd.getCmdParseResult().options;

  options.gui ? this->show() : this->hide();

  m_image = captureScreen(options.screen);
  save(options.filename, m_image);
}

Dialog::~Dialog() { delete ui; }

void Dialog::init() { cmd.evalCmd(); }
void Dialog::setup() { ui->setupUi(this); }
void Dialog::load() {}

QPixmap captureScreen(int screenNumber) {
  QScreen *screen = QGuiApplication::screens().at(screenNumber);
  if (!screen) {
    qCritical() << "Error: Couldn't get screen";
    return QPixmap();
  }
  QPixmap image = screen->grabWindow(0); // 0-> entire screen
  if (image.isNull()) {
    qCritical() << "Error: Failed to grab screen";
    return QPixmap();
  }
  return image;
}

void save(QString imagename, QPixmap shoot) {
  /*
    QString initialPath = QStandardPaths::writableLocation(
        QStandardPaths::StandardLocation::PicturesLocation);
    if (initialPath.isEmpty()) {
      initialPath = QDir::currentPath();
    }
  */
  QString initialPath = QDir::currentPath();
  if (imagename.isEmpty()) {
    imagename = defaultFilename;
  }
  QString fullname =
      QString("%1%2%3").arg(initialPath).arg(QDir::separator()).arg(imagename);
  if (!shoot.save(fullname)) {
    qDebug() << "Save Error: "
             << QString("Image couldn't save to \"%1\"").arg(imagename);
  }
}
