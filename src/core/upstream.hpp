#pragma once

#include <cstdint>
#include <string>

namespace gateway::core {
// struct with information how to connect to upstream
struct Upstream {
    std::string host;
    std::uint16_t port = 80;

    std::string address() const { return host + ":" + std::to_string(port); }
};
} // namespace gateway::core