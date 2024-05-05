#ifndef COMMANLINEOPTION_H
#define COMMANLINEOPTION_H

constexpr const char *helpString =
    "This application takes screenshot and saves the image or send to "
    "another machine if switches activated."
    "   * Komut satırından çalıştığında isteğe bağlı olarak gui açılacak"
    "* veya sadece komut satırından çalıştırılabilir olacak."
    "*"
    "* (bool) = Kullanılmazsa false, kullanılırsa true"
    "* (string) = String olarak ifade edilecek."
    "* (number) ="
    "*"
    "* Komut satırı seçenekleri:"
    "*"
    "* -h, --help(bool) : help"
    "Default : Program seçeneksiz kullanılırsa -h seçeneği kullanılacak."
    "* -v, --version(bool) : versiyon"
    "versiyonu help ile birlikte yaz!"
    "Default : False"
    "* -g, --gui : Gui istiyor musun? Resim ekranda gösterilsin mi?"
    "Default : False"
    "* -f, --filename(string): Çekilen ekran görüntüsü nereye kaydedilecek."
    "Default: \"untitled.png\""
    "Tam yol belirtilmeli, göreli yol ifadesi kabul olmayacak!"
    "?port belirtilmişse uyarı verecek?"
    "* --ip(string) : gönderilmek istenen ip adresi."
    "Belirtilmezse broadcast yapılır."
    "Default : Empty"
    "* -p, --port(number) : yayın yapılmak istenen port numarası."
    "Default : 8000"
    "*"
    "* Gelecekte:"
    "* client ve server modda çalışacaktır. seçenekler de client ve server "
    "için"
    "düzenlenecektir."
    "* Canlı ekran görüntüsü gönderiyormuş hissi vermek üzere saniyede kaç "
    "kare"
    "göndermek istendiği komut satırından alınabilecektir.";
/*
   *
   *
   * Komut satırından çalıştığında isteğe bağlı olarak gui açılacak
   * veya sadece komut satırından çalıştırılabilir olacak.
   *
   * (bool) = Kullanılmazsa false, kullanılırsa true
   * (string) = String olarak ifade edilecek.
   * (number) =
   *
   * Komut satırı seçenekleri:
   *
   * -h, --help(bool) : help
        Default : Program seçeneksiz kullanılırsa "-h" seçeneği kullanılacak.
   * -v, --version(bool) : versiyon
        versiyonu help ile birlikte yaz!
        Default : False
   * -g, --gui : Gui istiyor musun? Resim ekranda gösterilsin mi?
        Default : False
   * -f, --filename(string): Çekilen ekran görüntüsü nereye kaydedilecek.
        Default: "untitled.png"
        Tam yol belirtilmeli, göreli yol ifadesi kabul olmayacak!
        ?port belirtilmişse uyarı verecek?
   * --ip(string) : gönderilmek istenen ip adresi.
        Belirtilmezse broadcast yapılır.
        Default : Empty
   * -p, --port(number) : yayın yapılmak istenen port numarası.
        Default : 8000
   *
   * Gelecekte:
   * client ve server modda çalışacaktır. seçenekler de client ve server için
   düzenlenecektir.
   * Canlı ekran görüntüsü gönderiyormuş hissi vermek üzere saniyede kaç kare
   göndermek istendiği komut satırından alınabilecektir.
   *
   *
   *
   *
   */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QObject>

typedef struct Tag_Options {
  bool help, version, gui;
  QString filename, ip, port, usage;
} Options;

struct CommandLineParseResult {
  enum class Status {
    Ok,
    Error,
    VersionRequested,
    HelpRequested,
    GuiRequested,
    FileNameRequested,
    IpRequested,
    PortRequested,
    UsageRequested,
  };
  Status statusCode = Status::Ok;
  // my compiler is not that advanced
  // std::optional<QString> errorString = std::nullopt;
  QString errorString = QString();
  Options options = Options();
};

CommandLineParseResult parseCommandLine(QCommandLineParser &parser,
                                        Options &commandOptions);

class CommandOptionParser : public QObject {
  Q_OBJECT
public:
  // option flags
  // constructors
  explicit CommandOptionParser(QObject *parent = nullptr);

  void init();
  void load();
  void setup();
  void parse();

signals:
private:
  // QList<QCommandLineOption> commandLineOptions; // help, version, gui,
  // filename, ip, port, usage
  Options commandOptions;
  QCommandLineParser parser;
};

#endif // COMMANLINEOPTION_H
