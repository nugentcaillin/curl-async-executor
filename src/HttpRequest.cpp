#include "curl-async-executor/HttpRequest.hpp"
#include <iostream>
namespace curl_async_executor
{



HttpRequestBuilder&& HttpRequestBuilder::set_url(const std::string& url)
{
    curl_easy_setopt(request_.handle_, CURLOPT_URL, url.c_str());
    return std::move(*this);
}

HttpRequestBuilder&& HttpRequestBuilder::set_method(HttpMethod method)
{
    method_ = method;
    return std::move(*this);
}


HttpRequestBuilder&& HttpRequestBuilder::set_body(std::string body)
{
    request_.body_ = std::move(body);
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

// deal with method specific logic, return finished HttpRequest
HttpRequest HttpRequestBuilder::build() &&
{
    switch (method_)
    {
    case HttpMethod::GET:        
        curl_easy_setopt(request_.handle_, CURLOPT_HTTPGET, 1L);
        break;
    case HttpMethod::POST:
        curl_easy_setopt(request_.handle_, CURLOPT_POST, 1L);
        curl_easy_setopt(request_.handle_, CURLOPT_POSTFIELDS, request_.body_.c_str());
        curl_easy_setopt(request_.handle_, CURLOPT_POSTFIELDSIZE, request_.body_.length());
        break;
    }

    // handle body writing
    curl_easy_setopt(request_.handle_, CURLOPT_WRITEFUNCTION, HttpRequest::curl_body_write_callback);
    curl_easy_setopt(request_.handle_, CURLOPT_WRITEDATA, &request_.body_data_);

    return std::move(request_);
}
size_t HttpRequest::curl_body_write_callback(char *data, size_t size, size_t nmemb, void *clientp)
{
    std::string body_data = *(std::string*)clientp;
    if (nmemb > 0) body_data += data;
    return size * nmemb;
}

} 