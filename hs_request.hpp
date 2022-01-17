#ifndef HS_REQUEST_HPP
#define HS_REQUEST_HPP

#include <QByteArray>

#define HS_MAX_ROUTE_SIZE 2048

enum class HSRequestMethod
{
    UNKNOWN = 0,
    GET,
    POST,
    PUT,
    DELETE,
};

enum class HSStatus
{
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500,
};

struct HSRequest
{
    HSRequest(const QByteArray &request_ba);
    ~HSRequest();

    char *route = nullptr;
    const QByteArray &ba;
    HSRequestMethod method = HSRequestMethod::UNKNOWN;
    HSStatus status = HSStatus::INTERNAL_SERVER_ERROR;
};

HSRequest parse_request(const QByteArray &request_ba);
bool parse_request_method(HSRequest &request);
bool parse_request_route(HSRequest &request);

#endif // HS_REQUEST_HPP
