#include "curl-async-executor/HttpRequest.hpp"

namespace curl_async_executor
{



HttpRequestBuilder&& HttpRequestBuilder::set_url(std::string url)
{
    curl_easy_setopt(request_.handle_, CURLOPT_URL, url.c_str());
    return std::move(*this);
}

HttpRequestBuilder&& HttpRequestBuilder::set_method(HttpMethod method)
{
    (void)method;
    return std::move(*this);
}

HttpRequestBuilder&& HttpRequestBuilder::add_header(std::string key, std::string value)
{
    (void)key;
    (void) value;
    return std::move(*this);
}

HttpRequest HttpRequestBuilder::build() &&
{
    return std::move(request_);
}

} 