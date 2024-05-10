#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);

signals:
};

#endif // TCPCLIENT_H
