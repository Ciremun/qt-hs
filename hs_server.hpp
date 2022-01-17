#ifndef HS_SERVER_HPP
#define HS_SERVER_HPP

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>

#include "hs_request.hpp"

class TCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TCPServer(QObject *parent = 0);

private:
    void incomingConnection(qintptr socketDescriptor) override;
    HS_REQUEST_METHOD parse_request_method(const QByteArray& request);
};

#endif // HS_SERVER_HPP
