#pragma once

#include <boost/asio/awaitable.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace gateway::net {

class HttpServer {
  public:
    HttpServer(boost::asio::io_context &ioc, const boost::asio::ip::tcp::endpoint &endpoint);

    void start();

  private:
    boost::asio::awaitable<void> accept_loop();

    boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace gateway::net