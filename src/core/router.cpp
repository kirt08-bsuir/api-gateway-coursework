#include "core/router.hpp"

#include <algorithm>
#include <string>
#include <string_view>
#include <utility>

namespace gateway::core {

namespace {

bool path_has_prefix(std::string_view path, std::string_view prefix) {
    if (prefix == "/") {
        return true;
    }

    if (!path.starts_with(prefix)) {
        return false;
    }

    // here check this one exmaple
    // path: /user_settings, prefix: /user -> incorrect
    // path: /user/settings, prefix: /user -> correct
    return path.size() == prefix.size() || path[prefix.size()] == '/';
}

std::string build_upstream_target(const Route &route, const RequestInfo &request) {
    std::string path = request.path;
    if (!route.strip_prefix.empty() && path_has_prefix(path, route.strip_prefix)) {
        path.erase(0, route.strip_prefix.size());
    }

    if (path.empty()) {
        path = "/";
    }

    if (!request.query.empty()) {
        path += "?";
        path += request.query;
    }

    return path;
}

} // namespace

Router::Router(std::vector<Route> routes) : routes_(std::move(routes)) {
    std::ranges::stable_sort(routes_, [](const Route &a, const Route &b) {
        return a.prefix.size() > b.prefix.size();
    });
}

RouteResult Router::match(const RequestInfo &request) const {
    for (const Route &route : routes_) {
        if (!path_has_prefix(request.path, route.prefix)) {
            continue;
        }

        if (!route.allows_method(request.method)) {
            return {.status = MatchStatus::MethodNotAllowed, .route = &route};
        }

        return {
            .status = MatchStatus::Matched,
            .route = &route,
            .upstream_target = build_upstream_target(route, request),
        };
    }

    return {.status = MatchStatus::NotFound};
}

} // namespace gateway::core