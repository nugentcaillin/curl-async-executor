#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>

namespace curl_async_executor
{

class HttpResponse
{
    std::string body;
    int http_status;
    int curl_status;
};





} // namespace curl_async_executor

#endif // HTTP_RESPONSE_H