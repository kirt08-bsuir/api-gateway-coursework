#pragma once

#include <string>

namespace gateway::core {

struct RequestInfo {
    std::string method;
    std::string path;
    std::string query;
    std::string client_ip;
};

} // namespace gateway::core