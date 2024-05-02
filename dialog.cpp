#include "dialog.h"
#include "ui_dialog.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

QPixmap captureScreen(int screenNumber) {
  QScreen *screen;
  screen = QGuiApplication::screens().at(screenNumber);
  if (!screen) {
    qCritical() << "Error: Couldn't get screen";
    return QPixmap();
  }
  QPixmap image = screen->grabWindow(0); // 0-> entire screen
  return image;
}

void save(QString imagename, QPixmap shoot) {
  QString initialPath = QStandardPaths::writableLocation(
      QStandardPaths::StandardLocation::PicturesLocation);
  if (initialPath.isEmpty()) {
    initialPath = QDir::currentPath();
  }
  if (imagename.isEmpty()) {
    imagename = "untitled.png";
  }
  QString fullname =
      QString("%1%2%3").arg(initialPath).arg(QDir::separator()).arg(imagename);
  if (!shoot.save(fullname)) {
    qDebug() << "Save Error: "
             << QString("Image couldn't save to \"%1\"").arg(imagename);
  }
}

Dialog::Dialog(QWidget *parent) : QDialog(parent), ui(new Ui::Dialog) {
  ui->setupUi(this);
}

Dialog::~Dialog() { delete ui; }
