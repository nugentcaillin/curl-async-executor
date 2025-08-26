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

// Class to represent a Http request given to HttpExecutor to complete
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
        std::swap(body_data_, other.body_data_);
    }

    HttpRequest& operator=(HttpRequest&& other)
    {
        if (&other == this) return *this;
        if (headers_) curl_slist_free_all(headers_);
        if (handle_) curl_easy_cleanup(handle_);
        headers_ = nullptr;
        handle_ = nullptr;
        body_ = "";
        body_data_ = "";

        std::swap(headers_, other.headers_);
        std::swap(handle_, other.handle_);
        std::swap(body_, other.body_);
        std::swap(body_data_, other.body_data_);

        return *this;
    }
    ~HttpRequest()
    {
        if (headers_) curl_slist_free_all(headers_);
        if (handle_) curl_easy_cleanup(handle_);
    }

    // internal use only, do not call curl_easy_cleanup on handle
    CURL* get_handle() const 
    {
        return handle_;
    }

    // internal use only, used to get body data out of completed request
    std::string get_body_data()
    {
        return std::move(body_data_);
    }

private:
    CURL* handle_;
    struct curl_slist *headers_;
    std::string body_;

    // body to be written to by curl
    std::string body_data_;

    HttpRequest()
    : handle_(curl_easy_init())
    , headers_(nullptr)
    , body_()
    , body_data_()
    {
        if (!handle_) throw std::runtime_error("Unable to create curl easy handle"); 
    };
    static size_t curl_body_write_callback(char *data, size_t size, size_t nmemb, void *clientp);
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