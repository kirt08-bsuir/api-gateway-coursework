#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>
#include <csignal>
#include <fmt/core.h>

#include "net/http_server.hpp"

namespace asio = boost::asio;

int main() {
    asio::io_context ioc; // event loop

    gateway::core::Upstream upstream{.host = "127.0.0.1", .port = 9000};
    gateway::net::UpstreamClient upstream_client;

    const asio::ip::tcp::endpoint listen_endpoint{asio::ip::make_address("0.0.0.0"), 8000};
    gateway::net::HttpServer server(ioc, listen_endpoint, upstream_client, upstream);
    server.start();

    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](const boost::system::error_code &, int sig) {
        fmt::print("\nsignal {}, stopping\n", sig);
        ioc.stop();
    });

    ioc.run();
    return 0;
}