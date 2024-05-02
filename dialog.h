#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QPixmap>
#include <QScreen>

QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

QPixmap captureScreen(int screenNumber);
void save(QString imagename, QPixmap shoot);

class Dialog : public QDialog {
  Q_OBJECT

public:
  Dialog(QWidget *parent = nullptr);
  ~Dialog();

private:
  Ui::Dialog *ui;
  QPixmap m_image;
};
#endif // DIALOG_H
