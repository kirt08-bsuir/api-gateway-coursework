#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/signal_set.hpp>
#include <csignal>
#include <fmt/core.h>

#include "net/http_server.hpp"

namespace asio = boost::asio;

int main() {
    asio::io_context ioc; // event loop

    gateway::net::HttpServer server(ioc, {asio::ip::make_address("0.0.0.0"), 8000});
    server.start();

    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](const boost::system::error_code &, int sig) {
        fmt::print("\nsignal {}, stopping\n", sig);
        ioc.stop();
    });

    ioc.run();
    return 0;
}