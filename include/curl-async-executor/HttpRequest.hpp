#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <curl/curl.h>
#include <algorithm>
#include <stdexcept>

namespace curl_async_executor
{

enum HttpMethod
{
    GET,
    POST
};

// owns Curl handle, constructed with builder, move only so executor can take control of lifecycle of handle
class HttpRequest
{
    friend class HttpRequestBuilder;
public:
    HttpRequest(const HttpRequest& other) = delete;
    HttpRequest& operator=(const HttpRequest& other) = delete;

    HttpRequest(HttpRequest&& other)
    : handle_(nullptr)
    , headers_(nullptr)
    {
        std::swap(handle_, other.handle_);
        std::swap(headers_, other.headers_);
    }

    HttpRequest& operator=(HttpRequest&& other)
    {
        if (&other == this) return *this;
        if (headers_) curl_slist_free_all(headers_);
        if (handle_) curl_easy_cleanup(handle_);
        headers_ = nullptr;
        handle_ = nullptr;

        std::swap(headers_, other.headers_);
        std::swap(handle_, other.handle_);

        return *this;
    }
    ~HttpRequest()
    {
        if (headers_) curl_slist_free_all(headers_);
        if (handle_) curl_easy_cleanup(handle_);
    }

private:
    CURL* handle_;
    struct curl_slist *headers_;

    HttpRequest()
    : handle_(curl_easy_init())
    , headers_(nullptr)
    {
        if (!handle_) throw std::runtime_error("Unable to create curl easy handle"); 
    };
};

class HttpRequestBuilder
{
public:
    HttpRequestBuilder()
    : request()
    {};

    HttpRequestBuilder&& setUrl(std::string url);
    HttpRequestBuilder&& setMethod(HttpMethod method);
    HttpRequestBuilder&& addHeader(std::string key, std::string value);
    HttpRequest build() &&;

    HttpRequestBuilder(const HttpRequestBuilder& other) = delete;
    HttpRequestBuilder& operator=(const HttpRequestBuilder& other) = delete;
    HttpRequestBuilder(HttpRequestBuilder&& other) = default;
    HttpRequestBuilder& operator=(HttpRequestBuilder&& other) = default;
    ~HttpRequestBuilder() = default;

private:
    HttpRequest request;
};

} // namespace curl_async_executor

#endif // HTTP_REQUEST_H