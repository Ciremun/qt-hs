#ifndef HS_SERVER_HPP
#define HS_SERVER_HPP

#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QMap>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>

#include "hs_request.hpp"

static inline QByteArray JSON_OBJECT_TO_BA(std::initializer_list<QPair<QString, QJsonValue>> &&o)
{
    return QJsonDocument(QJsonObject(o)).toJson(QJsonDocument::Compact);
} 

class TCPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TCPServer(QObject *parent = 0);

private:
    QMap<int, QString> m_table;
    // NOTE(Ciremun): m_event_log memory usage can be optimized by using a custom log entry type
    QVector<QString> m_event_log;

    void write_status(QTcpSocket *socket, const HSRequest &request);
    void api_route(HSRequest &request, QByteArray &response);
    void test_route(HSRequest &request, QByteArray &response);
    void incomingConnection(qintptr socket_descriptor) override;
};

#endif // HS_SERVER_HPP
