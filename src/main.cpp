#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>
#include <csignal>
#include <fmt/core.h>

#include "core/router.hpp"
#include "net/http_server.hpp"
#include "net/upstream_client.hpp"

namespace asio = boost::asio;

int main() {
    asio::io_context ioc; // event loop

    const gateway::core::Upstream fastapi{.host = "127.0.0.1", .port = 9000};

    const gateway::core::Router router({
        {
            .name = "fastapi",
            .prefix = "/api/fastapi",
            .methods = {"GET"},
            .strip_prefix = "/api/fastapi",
            .upstream = fastapi,
        },
        {
            .name = "fastapi-echo",
            .prefix = "/api/fastapi/echo",
            .methods = {"POST"},
            .strip_prefix = "/api/fastapi",
            .upstream = fastapi,
        },
    });

    gateway::net::UpstreamClient upstream_client;

    const asio::ip::tcp::endpoint listen_endpoint{asio::ip::make_address("0.0.0.0"), 8000};
    gateway::net::HttpServer server(ioc, listen_endpoint, upstream_client, router);
    server.start();

    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](const boost::system::error_code &, int sig) {
        fmt::print("\nsignal {}, stopping\n", sig);
        ioc.stop();
    });

    ioc.run();
    return 0;
}