#include <QDebug>
#include <cstring>

#include "hs_request.hpp"

HSRequest::HSRequest(const QByteArray &request_ba) : ba(request_ba) {}

HSRequest::~HSRequest()
{
    if (route)
    {
        delete route;
        route = nullptr;
    }
}

HSRequest parse_request(const QByteArray &request_ba)
{
    HSRequest request(request_ba);
    bool request_method = parse_request_method(request);
    bool request_route = parse_request_route(request);
    request.status = request_method && request_route ?
        HSStatus::OK : HSStatus::BAD_REQUEST;
    if (!request_method)
        qDebug() << "Warning: couldn't parse request method";
    if (!request_route)
        qDebug() << "Warning: couldn't parse request route";
    return request;
}

bool parse_request_method(HSRequest &request)
{
    if (std::memcmp(request.ba, "GET",    3) == 0) request.method = HSRequestMethod::GET;
    if (std::memcmp(request.ba, "POST",   4) == 0) request.method = HSRequestMethod::POST;
    if (std::memcmp(request.ba, "PUT",    3) == 0) request.method = HSRequestMethod::PUT;
    if (std::memcmp(request.ba, "DELETE", 6) == 0) request.method = HSRequestMethod::DELETE;
    bool success = request.method != HSRequestMethod::UNKNOWN;
    if (!success)
        qDebug() << "Warning: unknown request method";
    return success;
}

bool parse_request_route(HSRequest &request)
{
    auto parse_route_after_method = [&request](int offset) mutable -> bool {
        char *route = new char[HS_MAX_ROUTE_SIZE]();
        char *tmp = route;
        const char *request_data = request.ba.data();
        request_data += offset + 1;
        while (*request_data != ' ')
        {
            *tmp++ = *request_data++;
            if (tmp - route > HS_MAX_ROUTE_SIZE)
            {
                qDebug() << "Warning: route size exceeds HS_MAX_ROUTE_SIZE";
                delete route;
                route = nullptr;
                return false;
            }
        }
        request.route = route;
        return true;
    };
    switch (request.method)
    {
        case HSRequestMethod::GET:    return parse_route_after_method(3);
        case HSRequestMethod::POST:   return parse_route_after_method(4);
        case HSRequestMethod::PUT:    return parse_route_after_method(3);
        case HSRequestMethod::DELETE: return parse_route_after_method(6);
        default:
            break;
    }
    qDebug() << "Warning: unknown request method";
    return false;
}
