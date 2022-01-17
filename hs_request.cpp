#include <cstring>

#include "hs_request.hpp"

HSRequest parse_request(const QByteArray &request_ba)
{
    HSRequest request(request_ba);
    request.method = parse_request_method(request);
    request.route = parse_request_route(request);
    return request;
}

HSRequestMethod parse_request_method(const HSRequest &request)
{
    const char *request_data = request.ba.data();
    if (std::memcmp(request_data, "GET",    3) == 0) return HSRequestMethod::GET;
    if (std::memcmp(request_data, "POST",   4) == 0) return HSRequestMethod::POST;
    if (std::memcmp(request_data, "PUT",    3) == 0) return HSRequestMethod::PUT;
    if (std::memcmp(request_data, "DELETE", 6) == 0) return HSRequestMethod::DELETE;
    return HSRequestMethod::UNKNOWN;
}

char* parse_request_route(const HSRequest &request)
{
    auto parse_route_after_method = [=](int offset) -> char* {
        char *route = new char[HS_MAX_ROUTE_SIZE]();
        char *tmp = route;
        const char *request_data = request.ba.data();
        request_data += offset + 1;
        while (*request_data != ' ')
            *tmp++ = *request_data++;
        return route;
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
    return 0;
}
