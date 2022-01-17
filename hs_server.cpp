#include <cstring>

#include "hs_server.hpp"

TCPServer::TCPServer(QObject *parent)
    : QTcpServer(parent)
{
    if(this->listen(QHostAddress::LocalHost, 1234))
        qDebug() << "Server started http://127.0.0.1:1234";
}

void TCPServer::incomingConnection(qintptr socket_descriptor)
{
    QTcpSocket *socket = new QTcpSocket();
    socket->setSocketDescriptor(socket_descriptor);
    socket->waitForReadyRead(1000);

    QByteArray request_ba = socket->readAll();
    if (!request_ba.isEmpty())
    {
        HSRequest request = parse_request(request_ba);
        QByteArray response;
        if (request.status == HSStatus::OK)
        {
            printf("status ok\n");
            if (std::memcmp(request.route, "/api", 4) == 0)
            {
                printf("api matched\n");
                response.insert(0, "api route");
            }
            if (std::memcmp(request.route, "/test", 5) == 0)
            {
                printf("test matched\n");
                response.insert(0, "test route");
            }
            printf("response: %s\n", response.data());
        }
        write_status(socket, request);
        if (!response.isEmpty())
            socket->write(response);
    }
    else
    {
        socket->write("HTTP/1.1 200 OK\r\n\r\n");
    }

    socket->disconnectFromHost();
    socket->deleteLater();
}

void TCPServer::write_status(QTcpSocket *socket, const HSRequest &request)
{
    switch (request.status)
    {
        case HSStatus::OK:
            socket->write("HTTP/1.1 200 OK\r\n\r\n");
            break;
        case HSStatus::BAD_REQUEST:
            socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
            break;
        case HSStatus::NOT_FOUND:
            socket->write("HTTP/1.1 404 Not Found\r\n\r\n");
            break;
        case HSStatus::INTERNAL_SERVER_ERROR:
            socket->write("HTTP/1.1 500 Internal Server Error\r\n\r\n");
            break;
        default:
            qDebug() << "Error: unknown request status: " << (int)request.status;
            break;
    }
}
