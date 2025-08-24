#include "curl-async-executor/HttpRequest.hpp"

namespace curl_async_executor
{



HttpRequestBuilder&& HttpRequestBuilder::set_url(const std::string& url)
{
    curl_easy_setopt(request_.handle_, CURLOPT_URL, url.c_str());
    return std::move(*this);
}

HttpRequestBuilder&& HttpRequestBuilder::set_method(const HttpMethod& method)
{
    (void)method;
    return std::move(*this);
}

HttpRequestBuilder&& HttpRequestBuilder::add_header(const std::string& key, const std::string& value)
{
    std::string header_string {};
    if (value != "") header_string = key + ": " + value; 
    else header_string = key + ":";
    request_.headers_ = curl_slist_append(request_.headers_, header_string.c_str());
    return std::move(*this);
}

HttpRequest HttpRequestBuilder::build() &&
{
    return std::move(request_);
}

} 