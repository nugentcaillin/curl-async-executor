#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <curl/curl.h>
#include <algorithm>
#include <stdexcept>

class HttpRequestTest;

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
friend class ::HttpRequestTest;
public:
    HttpRequest(const HttpRequest& other) = delete;
    HttpRequest& operator=(const HttpRequest& other) = delete;

    HttpRequest(HttpRequest&& other)
    : handle_(nullptr)
    , headers_(nullptr)
    , body_()
    {
        std::swap(handle_, other.handle_);
        std::swap(headers_, other.headers_);
        std::swap(body_, other.body_);
    }

    HttpRequest& operator=(HttpRequest&& other)
    {
        if (&other == this) return *this;
        if (headers_) curl_slist_free_all(headers_);
        if (handle_) curl_easy_cleanup(handle_);
        headers_ = nullptr;
        handle_ = nullptr;
        body_ = "";

        std::swap(headers_, other.headers_);
        std::swap(handle_, other.handle_);
        std::swap(body_, other.body_);

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
    std::string body_;

    HttpRequest()
    : handle_(curl_easy_init())
    , headers_(nullptr)
    , body_()
    {
        if (!handle_) throw std::runtime_error("Unable to create curl easy handle"); 
    };
};

class HttpRequestBuilder
{
friend class ::HttpRequestTest;
public:
    HttpRequestBuilder()
    : request_()
    , method_(HttpMethod::GET)
    {};

    HttpRequestBuilder&& set_url(const std::string& url);
    HttpRequestBuilder&& set_method(HttpMethod method);
    HttpRequestBuilder&& set_body(std::string body);
    HttpRequestBuilder&& add_header(const std::string& key, const std::string& value);
    HttpRequest build() &&;

    HttpRequestBuilder(const HttpRequestBuilder& other) = delete;
    HttpRequestBuilder& operator=(const HttpRequestBuilder& other) = delete;
    HttpRequestBuilder(HttpRequestBuilder&& other) = default;
    HttpRequestBuilder& operator=(HttpRequestBuilder&& other) = default;
    ~HttpRequestBuilder() = default;

private:
    HttpRequest request_;
    HttpMethod method_;
};

} // namespace curl_async_executor

#endif // HTTP_REQUEST_H