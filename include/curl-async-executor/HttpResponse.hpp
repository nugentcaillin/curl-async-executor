#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <algorithm>

namespace curl_async_executor
{

// Class to represent a HttpResponse created by a curl request

class HttpResponse
{
public:
    HttpResponse(std::string body, int http_status, int curl_status)
    : body_(body)
    , http_status_(http_status)
    , curl_status_(curl_status)
    {}
    HttpResponse()
    : body_("")
    , http_status_(0)
    , curl_status_(0)
    {}

    // getters
    const std::string& get_body() { return body_; }
    int get_http_status() { return http_status_; }
    int get_curl_status() { return curl_status_; }

    // move-only
    HttpResponse(const HttpResponse& other) = delete;
    HttpResponse& operator=(const HttpResponse& other) = delete;
    HttpResponse(HttpResponse&& other)
    : body_("")
    , http_status_()
    , curl_status_()
    {
        std::swap(this->body_, other.body_);
        std::swap(this->http_status_, other.http_status_);
        std::swap(this->curl_status_, other.curl_status_);
    }
    HttpResponse& operator=(HttpResponse&& other)
    {
        if (this == &other) return *this;
        this->body_ = "";
        this->http_status_ = 0;
        this->curl_status_ = 0;

        std::swap(this->body_, other.body_);
        std::swap(this->http_status_, other.http_status_);
        std::swap(this->curl_status_, other.curl_status_);

        return *this;
    }
    ~HttpResponse() = default;

private:
    std::string body_;
    int http_status_;

    // error code if present or CURLE_OK
    int curl_status_;
};





} // namespace curl_async_executor

#endif // HTTP_RESPONSE_H