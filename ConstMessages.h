#ifndef CONSTMESSAGES_H
#define CONSTMESSAGES_H

/*
{
    // runtime
    // {Status::FileNameRequired, __FilenameMessage(cmdParseResult)},
    // {Status::BindRequired, __BindRequiredMessage(ipPort)},
    // {Status::Error, __ParserError},
    // compiletime
    {Status::PositionalArgumentsError, __PositionalArgumentsErrorMessage},
    {Status::HelpRequested, ""},
    {Status::VersionRequested, ""},
    {Status::VerboseRequired, __VerboseMessage},
    {Status::DebugRequired, __DebugMessage},
    {Status::GuiRequired, __GuiMessage},
    {Status::FileNameError, __FilenameErrorMessage},
    {Status::BindError, __BindErrorMessage},
    {Status::NumberError, __NumberErrorMessage},
    {Status::UsageError, __UsageErrorMessage},
    {Status::UsageClient, __ClientMessage},
    {Status::UsageServer, __ServerMessage},
    {Status::ConnectionTypeError, __ConnectionTypeErrorMessage},
    {Status::ConnectionTypeTCP, __TcpMessage},
    {Status::ConnectionTypeUDP, __UdpMessage},
    {Status::IpError, __IpErrorMessage},
    {Status::IncomePortError, __IncomePortErrorMessage},
    {Status::OutcomePortError, __OutcomePortErrorMessage},
}
*/

// const messages
#define __PositionalArgumentsErrorMessage "Missing subcommand!"
// #define __PositionalArgumentsErrorMessage                                      \
//  "There is no positional argument implementation !"
#define __ParserError parser.errorText()
#define __VerboseMessage "Verbose Mode Selected."
#define __DebugMessage "Debug Mode Selected."
#define __GuiMessage "Gui Mode Selected."
#define __FilenameMessage(cmdParseResult)                                      \
  QString("Filename:  %1").arg(cmdParseResult.options.filename)
#define __FilenameErrorMessage                                                 \
  "Filename not specified: Default: " defaultFilename
#define __BindRequiredMessage(ipPort)                                          \
  QString("I am only taking these info: %1:%2").arg(ipPort.at(0), ipPort.at(1))
#define __BindErrorMessage                                                     \
  "You have entered ip and port in a wrong way. check it out! || <ip:port> "   \
  "|| Default: " defaultBind // qCritical();
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
#define __Unkown "Unkown"

#endif // CONSTMESSAGES_H
