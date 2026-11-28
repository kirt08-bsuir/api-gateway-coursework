#pragma once

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include "core/upstream.hpp"

namespace gateway::core {

// fields from .yaml config
struct Route {
    std::string name;
    std::string prefix;

    std::vector<std::string> methods; // allowed methods
    std::string strip_prefix; // what we should cut from original prefix before sending to upstream

    Upstream upstream; // here will be list, when we will implement round-robin

    // Empty list (methods) - all methods are allowed
    bool allows_method(std::string_view method) const {
        return methods.empty() || std::ranges::find(methods, method) != methods.end();
    }
};

} // namespace gateway::core