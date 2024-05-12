#ifndef HELP_H
#define HELP_H

#define HelpHeader                                                                 \
  "-*-*-*-*-* Header *-*-*-*-*-\n"                                                 \
  "This application takes screenshot and saves the image or send to \n"            \
  "another machine if switches activated. \n"                                      \
  "* Komut satırından çalıştığında isteğe bağlı olarak gui açılacak " \
  "veya sadece komut satırından çalıştırılabilir olacak. \n"                \
  "\n\n"                                                                           \
  "* (bool) = Kullanılmazsa false, kullanılırsa true \n"                        \
  "* (string) = String olarak ifade edilecek. Kullanılmazsa default olan\n"       \
  "kabul edilecektir. \n"                                                          \
  "* (number) = Sayı. Kullanılmazsa default olan kabul edilecektir. \n\n"        \
  "* (positional) = '-' veya '--' şeklinde ifade edilmeyenler. Kullanılmazsa "   \
  "default olan kabul edilecektir. \n"                                             \
  "* (NI) = \"not implemented\". Daha bu özellik programa eklenmedi.\n"           \
  "\n\n"                                                                           \
  "* Komut satırı seçenekleri: \n"                                              \
  "\n"

#define FuturePlan                                                               \
  "\n\n-*-*-*-*-* Future Plan *-*-*-*-*-\n"                                      \
  "* Gelecekte: Canlı ekran görüntüsü gönderiyormuş hissi vermek üzere " \
  "saniyede kaç kare göndermek istendiği komut satırından alınabilecektir.\n"

#define HelpFooter FuturePlan "\n" Author

#define __SECTION_SubCommand                                                       \
  "* (NI) (positional) : kullanım türü(client/server).\n"                       \
  "\tDefault : " defaultUsage "\n"                                                 \
  "\tBelirtilmezse client kabul edilir. Eğer client modunda server gibi, \n"      \
  "\tserver modunda client gibi kullanılırsa hata verir. Client ve Server \n"    \
  "\tmodundaki seçenekler birbirlerinden farklıdır. Eğer client hakkında \n"  \
  "\tyardım almak istiyorsanız <programadı> client --help || client -h || "     \
  "--help client || -h client , Eğer server \n"                                   \
  "\thakkında yardım almak istiyorsanız <programadı> server --help || server " \
  "-h || --help server || -h server \n"

#define __SECTION_Help                                                         \
  "* -h, --help(bool) : Help \n"                                               \
  "\tDefault : " defaultHelp "\n"                                              \
  "* client -h || -h client : Client Help \n"                                  \
  "\tDefault : " defaultClientHelp "\n"                                        \
  "* server -h || -h server : Server Help \n"                                  \
  "\tDefault : " defaultServerHelp "\n"

#define __SECTION_Version                                                      \
  "* -v, --version(bool) : Versiyon \n"                                        \
  "\tDefault : " defaultVersion "\n"

#define __SECTION_Verbose                                                      \
  "* -V, --verbose(bool) : Verbose \n"                                         \
  "\tDefault : " defaultVerbose "\n"

#define __SECTION_Debug                                                        \
  "* -d, --debug(bool) : Debug \n"                                             \
  "\tDefault : " defaultDebug "\n"

#define __SECTION_Usage                                                           \
  "* (NI) -u, --usage(string) : kullanım "                                       \
  "türü(client/server).\n"                                                      \
  "\tDefault : " defaultUsage "\n"                                                \
  "\tBelirtilmezse client kabul edilir. Eğer client modunda server gibi, \n"     \
  "\tserver modunda client gibi kullanılırsa hata verir. Client ve Server \n"   \
  "\tmodundaki seçenekler birbirlerinden farklıdır. Eğer client hakkında \n" \
  "\tyardım almak istiyorsanız <programadı> client --help, Eğer server \n"    \
  "\thakkında yardım almak istiyorsanız <programadı> server --help \n"

#define __SECTION_Gui                                                          \
  "* (NI)  -g, --gui(bool) : Gui istiyor musun? \n"                            \
  "\tResim ekranda gösterilsin mi? \n"                                        \
  "\tDefault : " defaultGui "\n"

#define __SECTION_Filename                                                     \
  "* -f, --filename(string): Çekilen ekran görüntüsü nereye "             \
  "kaydedilecek.\n"                                                            \
  "\tDefault: \"" defaultFilename "\"\n"                                       \
  "\tTam yol belirtilmeli, göreli yol ifadesi kabul olmayacak! \n"            \
  "\t?port belirtilmişse uyarı verecek? \n"

#define __SECTION_Number                                                       \
  "* (NI) -n, --number(number) : Çekilecek ekran "                            \
  "görüntü sayısı\n"                                                      \
  "\tDefault : " defaultNumber "\n"

#define __SECTION_Bind                                                         \
  "* (NI) -b, --bind(string) : Gönderilmek istenen ip adresi ve port. \n"     \
  "\tDefault<ip:port> : " defaultBind "\n"

#define __SECTION_ConnectionType                                               \
  "* (NI) -c, --conection-type(string) : Veriyi hangi "                        \
  "protokol ile iletmek istiyorsun?(TCP/UDP)"                                  \
  "\tDefault :  " defaultConnectionType "\n"

#define __SECTION_Main                                                         \
  __SECTION_Help __SECTION_Version __SECTION_Verbose __SECTION_Verbose         \
      __SECTION_Gui __SECTION_Filename __SECTION_Number __SECTION_Bind         \
          __SECTION_Subcommand

#define HelpMain                                                               \
  HelpHeader __SECTION_Help __SECTION_Version __SECTION_Verbose HelpFooter

// HelpHeader HelpGeneral HelpFooter

#define __SECTION_Client ""
#define __SECTION_Server ""

#define HelpClient HelpMain

#define HelpServer HelpHeader __SECTION_Server HelpFooter

#endif // HELP_H
