#pragma once

#include <boost/asio/ip/tcp.hpp>

#include "core/upstream.hpp"
#include "net/upstream_client.hpp"

namespace gateway::net {

class HttpServer {
public:
    HttpServer(
        boost::asio::io_context &ioc,
        const boost::asio::ip::tcp::endpoint &endpoint,
        UpstreamClient &upstream_client,
        core::Upstream upstream
    );

    void start();

private:
    boost::asio::awaitable<void> accept_loop();

    boost::asio::ip::tcp::acceptor acceptor_;

    UpstreamClient &upstream_client_;

    core::Upstream upstream_;
};

} // namespace gateway::net