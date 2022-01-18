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
    socket->waitForReadyRead();

    QByteArray request_ba = socket->readAll();
    qDebug() << request_ba.data() << '\n';
    if (!request_ba.isEmpty())
    {
        HSRequest request = parse_request(request_ba);
        QByteArray response;
        if (request.status == HSStatus::OK)
        {
            if (std::memcmp(request.route, "/api", 4) == 0)
                api_route(request, response);
            else if (std::memcmp(request.route, "/test", 5) == 0)
                test_route(request, response);
            else
                request.status = HSStatus::NOT_FOUND;
            printf("response: %s\n", response.data());
        }
        write_status(socket, request);
        if (!response.isEmpty())
            socket->write(response);
    }
    else
    {
        socket->write("HTTP/1.1 400 Bad Request\r\n\r\n");
    }

    socket->flush();
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

void TCPServer::api_route(HSRequest &request, QByteArray &response)
{
    auto parse_json_document = [&](QJsonDocument *out) -> bool {
        char *request_data = (char *)request.ba.data();
        int content_length = 0;
        while (1)
        {
            if (*request_data == 0) { request.status = HSStatus::BAD_REQUEST; return false; }
            if (*request_data == '\n' && std::memcmp(request_data + 1, "Content-Length: ", 16) == 0)
            {
                request_data += 17;
                QString content_length_string = "";
                while (*request_data != '\r')
                    content_length_string += *request_data++;
                content_length = content_length_string.toInt();
                break;
            }
            request_data++;
        }
        while (std::memcmp(request_data, "\r\n\r\n", 4) != 0)
        {
            if (*request_data == 0) { request.status = HSStatus::BAD_REQUEST; return false; }
            request_data++;
        }
        request_data += 4;
        QJsonParseError error;
        QJsonDocument document = QJsonDocument::fromJson(request_data, &error);
        if (error.error != QJsonParseError::NoError)
        {
            qDebug() << "Error: couldn't parse JSON: " << error.errorString();
            return false;
        }
        *out = document;
        return true;
    };
    switch (request.method)
    {
        case HSRequestMethod::GET:
        {
            response.append("api route");
        } break;
        case HSRequestMethod::POST:
        {
            QJsonDocument document;
            if (!parse_json_document(&document))
                return;
            QJsonValue value = document["value"];
            if (value.isUndefined())
            {
                response.append(JSON_OBJECT_TO_BA({{"message", "api error: no value was provided"}}));
            }
            else if (value.isString())
            {
                m_table[m_table.count()] = value.toString();
                QString entry_id_string = QString::number(m_table.count() - 1);
                response.append(JSON_OBJECT_TO_BA({{"id", entry_id_string}}));
                m_event_log.push_back("item with id " + entry_id_string + " has been added");
            }
            else
            {
                response.append(JSON_OBJECT_TO_BA({{"message", "api error: value type is not supported"}}));
            }
        } break;
        case HSRequestMethod::PUT:
        {
            QJsonDocument document;
            if (!parse_json_document(&document))
                return;
            QJsonValue id = document["id"];
            if (id.isUndefined())
                response.append(JSON_OBJECT_TO_BA({{"message", "api error: no id was provided"}}));
            else if (id.isDouble())
            {
                QJsonValue value = document["value"];
                if (value.isUndefined())
                    response.append(JSON_OBJECT_TO_BA({{"message", "api error: no value was provided"}}));
                else if (value.isString())
                {
                    int idx = id.toVariant().toInt();
                    bool key_existed = m_table.contains(idx);
                    m_table[idx] = value.toString();
                    QString entry_id_string = QString::number(idx);
                    response.append(JSON_OBJECT_TO_BA({{"id", entry_id_string}}));
                    m_event_log.push_back("item with id " + entry_id_string +
                                         (key_existed ? " has been changed" : " has been added"));
                }
                else
                    response.append(JSON_OBJECT_TO_BA({{"message", "api error: value type is not supported"}}));
            }
            else
                response.append(JSON_OBJECT_TO_BA({{"message", "api error: id type is not supported"}}));
        } break;
        case HSRequestMethod::DELETE:
        {
            QJsonDocument document;
            if (!parse_json_document(&document))
                return;
            QJsonValue id = document["id"];
            if (id.isUndefined())
                response.append(JSON_OBJECT_TO_BA({{"message", "api error: no id was provided"}}));
            else if (id.isDouble())
            {
                int idx = id.toVariant().toInt();
                if (!m_table.remove(idx))
                {
                    response.append(JSON_OBJECT_TO_BA({{"message", "api error: id doesn't exist"}}));
                    return;
                }
                QString entry_id_string = QString::number(idx);
                response.append(JSON_OBJECT_TO_BA({{"id", entry_id_string}}));
                m_event_log.push_back("item with id " + entry_id_string + " has been deleted");
            }
            else
                response.append(JSON_OBJECT_TO_BA({{"message", "api error: id type is not supported"}}));
        } break;
        default:
            qDebug() << "Warning: unknown request method: " << (int)request.method;
            break;
    }
}

void TCPServer::test_route(HSRequest &request, QByteArray &response)
{
    switch (request.method)
    {
        case HSRequestMethod::GET:
        {
            response.append("<!DOCTYPE html>\n\n<html><body>");
            if (m_table.isEmpty())
            {
                response.append("table is empty<br>");
            }
            else
            {
                response.append("<table><tr><td>ID</td><td>Value</td></tr>");
                decltype(m_table)::const_iterator i = m_table.constBegin();
                while (i != m_table.constEnd())
                {
                    QString table_entry = "<tr><td>" + QString::number(i.key()) + "</td><td>" + i.value() + "</td></tr>";
                    QByteArray table_entry_ba = table_entry.toLocal8Bit();
                    response.append(table_entry_ba);
                    ++i;
                }
                response.append("</table><br>");
            }
            if (m_event_log.isEmpty())
            {
                response.append("event log is empty");
            }
            else
            {
                for (const auto &le : m_event_log)
                {
                    QString log_entry = le + "<br>";
                    QByteArray log_entry_ba = log_entry.toLocal8Bit();
                    response.append(log_entry_ba);
                }
            }
            response.append("</body></html>");
        } break;
        case HSRequestMethod::POST:
        case HSRequestMethod::PUT:
        case HSRequestMethod::DELETE:
        {
            request.status = HSStatus::BAD_REQUEST;
        } break;
        default:
            qDebug() << "Warning: unknown request method: " << (int)request.method;
            break;
    }
}
