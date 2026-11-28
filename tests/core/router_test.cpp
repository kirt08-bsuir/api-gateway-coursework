#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "core/router.hpp"

namespace gateway::core {

namespace {

Route make_route(
    std::string name,
    std::string prefix,
    std::vector<std::string> methods = {},
    std::string strip_prefix = ""
) {
    return Route{
        .name = std::move(name),
        .prefix = std::move(prefix),
        .methods = std::move(methods),
        .strip_prefix = std::move(strip_prefix),
        .upstream = {.host = "127.0.0.1", .port = 9000},
    };
}

RequestInfo make_request(std::string method, std::string path, std::string query = "") {
    return {
        .method = std::move(method),
        .path = std::move(path),
        .query = std::move(query),
        .client_ip = "127.0.0.1",
    };
}

TEST(RouterTest, MatchesExactPrefix) {
    Router router({make_route("users", "/api/users")});
    const RouteResult result = router.match(make_request("GET", "/api/users"));

    ASSERT_EQ(result.status, MatchStatus::Matched);
    EXPECT_EQ(result.route->name, "users");
}

TEST(RouterTest, MatchesSubPath) {
    Router router({make_route("users", "/api/users")});
    const RouteResult result = router.match(make_request("GET", "/api/users/42"));

    EXPECT_EQ(result.status, MatchStatus::Matched);
}

TEST(RouterTest, ReturnsNotFoundOnPartialSegment) {
    Router router({make_route("users", "/api/users")});
    const RouteResult result = router.match(make_request("GET", "/api/users_service"));

    EXPECT_EQ(result.status, MatchStatus::NotFound);
}

TEST(RouterTest, PrefersLongestPrefix) {
    Router router({make_route("api", "/api"), make_route("users", "/api/users")});
    const RouteResult result = router.match(make_request("GET", "/api/users/42"));

    ASSERT_EQ(result.status, MatchStatus::Matched);
    EXPECT_EQ(result.route->name, "users");
}

TEST(RouterTest, FallsBackToShorterPrefix) {
    Router router({make_route("api", "/api"), make_route("users", "/api/users")});
    const RouteResult result = router.match(make_request("GET", "/api/orders"));

    ASSERT_EQ(result.status, MatchStatus::Matched);
    EXPECT_EQ(result.route->name, "api");
}

TEST(RouterTest, RootPrefixMatchesEverything) {
    Router router({make_route("frontend", "/")});
    const RouteResult result = router.match(make_request("GET", "/any/path"));

    EXPECT_EQ(result.status, MatchStatus::Matched);
}

TEST(RouterTest, ReturnsNotFoundWhenNoRouteMatches) {
    Router router({make_route("users", "/api/users")});
    const RouteResult result = router.match(make_request("GET", "/other"));

    EXPECT_EQ(result.status, MatchStatus::NotFound);
    EXPECT_EQ(result.route, nullptr);
}

TEST(RouterTest, ReturnsMethodNotAllowedWithRoute) {
    Router router({make_route("users", "/api/users", {"GET", "POST"})});
    const RouteResult result = router.match(make_request("DELETE", "/api/users/42"));

    ASSERT_EQ(result.status, MatchStatus::MethodNotAllowed);
    ASSERT_NE(result.route, nullptr);
    EXPECT_EQ(result.route->methods, (std::vector<std::string>{"GET", "POST"}));
}

TEST(RouterTest, StripsPrefix) {
    Router router({make_route("users", "/api/users", {}, "/api")});
    const RouteResult result = router.match(make_request("GET", "/api/users/42"));

    EXPECT_EQ(result.upstream_target, "/users/42");
}

TEST(RouterTest, StripToEmptyBecomesRoot) {
    Router router({make_route("svc", "/api/svc", {}, "/api/svc")});
    const RouteResult result = router.match(make_request("GET", "/api/svc"));

    EXPECT_EQ(result.upstream_target, "/");
}

TEST(RouterTest, KeepsQueryString) {
    Router router({make_route("users", "/api/users", {}, "/api")});
    const RouteResult result = router.match(make_request("GET", "/api/users", "page=2"));

    EXPECT_EQ(result.upstream_target, "/users?page=2");
}

} // namespace

} // namespace gateway::core