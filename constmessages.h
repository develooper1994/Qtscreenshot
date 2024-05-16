#ifndef CONSTMESSAGES_H
#define CONSTMESSAGES_H

// const messages
#define __PositionalArgumentsErrorMessage "Missing subcommand!"
#define __ParserError parser.errorText()
#define __VerboseMessage "Verbose Mode Selected."
#define __DebugMessage "Debug Mode Selected."
#define __GuiMessage "Gui Mode Selected."
#define __FilenameMessage(filename) QString("Filename:  %1").arg(filename)
#define __FilenameErrorMessage                                                 \
  "Filename not specified: Default: " defaultFilename
#define __BindRequiredMessage(ipPort)                                          \
  QString("I am only taking these info: %1:%2").arg(ipPort.at(0), ipPort.at(1))
#define __BindErrorMessage                                                     \
  "You have entered ip and port in a wrong way. check it out! || <ip:port> "   \
  "|| Default: " defaultBind // qCritical();
#define __ScreenErrorMessage                                                   \
  "It is not a number or device doesn't have that screen number or there "     \
  "isn't any device named like that. Please enter screen a number or "         \
  "framebuffer device"                                                         \
  "Default(screen number): " defaultScreen                                     \
  " or Default(framebuffer): " defaultFbdev
#define __NumberErrorMessage                                                   \
  "number parameter cannot parsed. Default: " defaultNumber
#define __UsageErrorMessage "Usage Error. Default: " defaultUsage
#define __ClientMessage "Client Mod Selected."
#define __ServerMessage "Server Mod Selected."
#define __ConnectionTypeErrorMessage                                           \
  "Connection type error. Default: " defaultConnectionType
#define __TcpMessage "Tcp Mod Selected."
#define __UdpMessage "Udp Mod Selected."
#define __IpErrorMessage                                                       \
  "Unknown Network Layer Protocol. Please enter a valide ip adress.  || "      \
  "<ip:port> || Default: " defaultBind // qCritical();
#define __IncomePortErrorMessage                                               \
  "Please enter valid income port number. 0-1024 reserved, please "            \
  "select "                                                                    \
  "1024>\"Income Port\"<65535. Please enter a valide port "                    \
  "adress.  || <ip:port> || Default: " defaultBind // qCritical();
#define __OutcomePortErrorMessage                                              \
  "Please enter valid income port number. 0-1024 reserved, please "            \
  "select "                                                                    \
  "1024>\"Income Port\"<65535. Please enter a valide port "                    \
  "adress.  || <ip:port> || Default: " defaultBind // qCritical();
#define __UnkownError "Unkown Error"
#define __Ok "Ok"

#endif // CONSTMESSAGES_H
