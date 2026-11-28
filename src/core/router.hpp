#pragma once

#include <string>
#include <vector>

#include "core/request_info.hpp"
#include "core/route.hpp"

namespace gateway::core {

enum class MatchStatus { Matched, NotFound, MethodNotAllowed };

// router's answer, network layer will forward the requests.
struct RouteResult {
    MatchStatus status = MatchStatus::NotFound;

    const Route *route = nullptr;

    std::string upstream_target{}; // empty string
};

class Router {
public:
    explicit Router(std::vector<Route> routes);

    RouteResult match(const RequestInfo &request) const;

private:
    std::vector<Route> routes_; // vector sorted by prefix length, from long to short
};

} // namespace gateway::core