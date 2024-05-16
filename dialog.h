#ifndef DIALOG_H
#define DIALOG_H

#include "cmd.h"
#include "fbcat.h"
#include <QDialog>
#include <QPixmap>
#include <QScreen>

QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

QPixmap captureScreen(const ScreenInfo &si);
void save(QString imagename, QPixmap shoot);

class Dialog : public QDialog {
  Q_OBJECT

public:
  Dialog(QWidget *parent = nullptr);
  ~Dialog();

private:
  Ui::Dialog *ui;
  QPixmap m_image;
  Cmd cmd;
  FrameBuffer frameBuffer;
  void init();
  void setup();
  void load();
};
#endif // DIALOG_H
