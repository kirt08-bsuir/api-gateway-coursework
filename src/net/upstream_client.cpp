#include "net/upstream_client.hpp"

// tcp::resolver help us to convert domain like google.com into list of IP
// in your case, we can put inside config information like 'fastapi:8000'
// and it will be changed for example into 127.0.0.1:800
#include <boost/asio/ip/tcp.hpp> // tcp::resolver

// asio::this_coro::executor allow us to get event loop inside wich your couritine was started
#include <boost/asio/this_coro.hpp> // asio::this_coro::executor
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

namespace gateway::net {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

using tcp = asio::ip::tcp;

asio::awaitable<HttpResponse> UpstreamClient::forward(
    const core::Upstream &upstream,
    HttpRequest request
) {
    auto executor = co_await asio::this_coro::executor;

    // here can be additional request to DNS
    tcp::resolver resolver(executor);
    const auto endpoints = co_await resolver.async_resolve(
        upstream.host,
        std::to_string(upstream.port),
        asio::use_awaitable
    );

    // create tcp socket
    beast::tcp_stream stream(executor);
    co_await stream.async_connect(endpoints, asio::use_awaitable);

    // prepare request for sending
    // change Host into address of our service
    request.set(http::field::host, upstream.address());

    request.keep_alive(false);

    // recalculate Content-Length header
    request.prepare_payload();

    co_await http::async_write(stream, request, asio::use_awaitable);

    // getting response
    beast::flat_buffer buffer;
    http::response_parser<http::string_body> parser;

    if (request.method() == http::verb::head) {
        parser.skip(true);
    }
    co_await http::async_read(stream, buffer, parser, asio::use_awaitable);

    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // release() - get done message from parser

    co_return parser.release();
}

} // namespace gateway::net