#include "net/http_server.hpp"

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <exception>
#include <fmt/format.h>
#include <string>
#include <string_view>

namespace gateway::net {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = asio::ip::tcp;

namespace {

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;

constexpr auto use_tuple = asio::as_tuple(asio::use_awaitable);

std::string_view sv(beast::string_view s) { return {s.data(), s.size()}; };

void log_exception(std::exception_ptr error) {
    if (!error)
        return;
    try {
        std::rethrow_exception(error);
    } catch (const std::exception &e) {
        fmt::print(stderr, "coroutine failed: {}\n", e.what());
    } catch (...) {
        fmt::print(stderr, "coroutine failed: unknown exception\n");
    }
}

Response make_response(const Request &req, std::string_view client) {
    Response res{http::status::ok, req.version()};

    res.set(http::field::server, "course_work_gateway");
    res.set(http::field::content_type, "text/plain; charset=utf-8");

    res.keep_alive(req.keep_alive());

    res.body() = fmt::format("gateway: {} {} from {}\n", sv(req.method_string()), sv(req.target()),
                             client);

    res.prepare_payload();
    return res;
}

asio::awaitable<void> run_session(beast::tcp_stream stream) {
    beast::error_code ec;

    const tcp::endpoint peer = stream.socket().remote_endpoint(ec);
    const std::string client = ec ? "unknown" : peer.address().to_string();
    fmt::print("[{}] connected\n", client);

    beast::flat_buffer buffer;

    for (;;) {
        Request req;

        auto [read_ec, read_bytes] = co_await http::async_read(stream, buffer, req, use_tuple);

        if (read_ec == http::error::end_of_stream)
            break;
        if (read_ec) { // invalid HTTP request
            fmt::print("[{}] read error: {}\n", client, read_ec.message());
            break;
        }

        Response res = make_response(req, client);
        fmt::print("[{}] {} {} -> {}\n", client, sv(req.method_string()), sv(req.target()),
                   res.result_int());

        auto [write_ec, written] = co_await http::async_write(stream, res, use_tuple);
        if (write_ec) {
            fmt::print("[{}] write error: {}\n", client, write_ec.message());
            break;
        }

        if (!res.keep_alive())
            break;
    }

    stream.socket().shutdown(tcp::socket::shutdown_send, ec);
    fmt::print("[{}] disconnected\n", client);
}

} // namespace

HttpServer::HttpServer(asio::io_context &ioc, const tcp::endpoint &endpoint)
    : acceptor_(ioc, endpoint){};

void HttpServer::start() {
    fmt::print("listening on {}:{}\n", acceptor_.local_endpoint().address().to_string(),
               acceptor_.local_endpoint().port());

    asio::co_spawn(acceptor_.get_executor(), accept_loop(), log_exception);
}

asio::awaitable<void> HttpServer::accept_loop() {
    for (;;) {
        auto [ec, socket] = co_await acceptor_.async_accept(use_tuple);

        // operation_aborted - acceptor (has been closed), shutdown server;
        if (ec == asio::error::operation_aborted)
            co_return;
        if (ec) {
            fmt::print(stderr, "accept error: {}\n", ec.message());
            continue;
        }

        // for each connection we create a new coroutine-session
        // beast::tcp_stream wraps a basic socket and adds timeout support.
        asio::co_spawn(acceptor_.get_executor(), run_session(beast::tcp_stream(std::move(socket))),
                       log_exception);

        // Do not wait for the session to finish (no co_await); immediately accept the next client.
    }
}

} // namespace gateway::net
