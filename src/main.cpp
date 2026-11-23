#include <boost/asio.hpp>
#include <boost/version.hpp>
#include <fmt/core.h>

int main() {
    boost::asio::io_context ioc; // event loop
    fmt::print("gateway: Boost {} подключён\n", BOOST_LIB_VERSION);
    ioc.run();
    return 0;
}