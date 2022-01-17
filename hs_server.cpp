#include "hs_server.hpp"

TCPServer::TCPServer(QObject *parent) :
    QTcpServer(parent)
{

    if(this->listen(QHostAddress::LocalHost, 1234))
        qDebug() << "Server started localhost:1234";

}

void TCPServer::incomingConnection(qintptr socketDescriptor)
{
    auto socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);
    socket->waitForReadyRead(1000);
    QByteArray request = socket->readAll();
    auto request_method = parse_request_method(request);
    // std::string szRequest = data.toStdString();
    QString response_string = QString("request method: ") + QString::number((int)request_method);
    QByteArray response_string_ba = response_string.toLocal8Bit();
    socket->write("HTTP/1.1 200 OK\r\n\r\n");
    socket->write(response_string_ba);
    socket->disconnectFromHost();
}

HS_REQUEST_METHOD TCPServer::parse_request_method(const QByteArray& request)
{
    const char *request_string = request.data();
    if (memcmp(request_string, "GET", 3) == 0)
        return HS_REQUEST_METHOD::GET;
    if (memcmp(request_string, "POST", 4) == 0)
        return HS_REQUEST_METHOD::POST;
    if (memcmp(request_string, "PUT", 3) == 0)
        return HS_REQUEST_METHOD::PUT;
    if (memcmp(request_string, "DELETE", 6) == 0)
        return HS_REQUEST_METHOD::DELETE;
    return HS_REQUEST_METHOD::UNKNOWN;
}
