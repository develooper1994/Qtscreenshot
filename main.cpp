#include "dialog.h"

#include <QApplication>

void setApplicationSettings(const QApplication &a) {
  QApplication::setApplicationName("QtScreenShoot");
  QApplication::setApplicationVersion("1.0");
  QApplication::setApplicationDisplayName("ScreenShoot");
}

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  setApplicationSettings(app);

  Dialog w;
  w.show();
  return app.exec();
}
