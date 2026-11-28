#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

#include "core/upstream.hpp"

namespace gateway::net {

using HttpRequest = boost::beast::http::request<boost::beast::http::string_body>;
using HttpResponse = boost::beast::http::response<boost::beast::http::string_body>;

class UpstreamClient {
public:
    boost::asio::awaitable<HttpResponse> forward(
        const core::Upstream &upstream,
        HttpRequest request
    );
};

} // namespace gateway::net