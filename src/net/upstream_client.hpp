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
    /**
     * @brief Forwards HTTP request to specified upstream server and returns its response
     *
     * @param upstream Target backend service (host and port)
     * @param request Incoming request (client -> gateway)
     * @return Awaitable resolving (result) to the HTTP response received from the upstream (backend
     *service)
     * @throws boost::system::system_error on DNS resolution, connecting to upstream, or network I/O
     *errors
     */
    boost::asio::awaitable<HttpResponse> forward(
        const core::Upstream &upstream,
        HttpRequest request
    );
};

} // namespace gateway::net