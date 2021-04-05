#pragma once
#include "http.h"
#include <string_view>
#include <utility>
#include <optional>
namespace uri {
struct parts_get {
    std::string_view scheme;
    std::string_view userinfo;
    std::string_view host;
    std::string_view port;
    std::string_view path;
};
struct parts {
    std::string_view scheme;
    std::string_view userinfo;
    std::string_view host;
    std::string_view port;
    std::string_view path;
    std::string_view query;
    std::string_view fragment;
};
inline constexpr auto parse_get(std::string_view uri,
                                std::string_view::size_type pos = 0)
    -> parts_get
{
    auto epos = uri.find(':', pos);
    if (epos == std::string_view::npos)
        return {};
    parts_get p;
    p.scheme = uri.substr(pos, epos - pos);
    pos = epos + 1;
    if (uri.compare(pos, 2, "//") == 0) {
        pos += 2;
        epos = uri.find('@', pos);
        if (epos != std::string_view::npos) {
            p.userinfo = uri.substr(pos, epos - pos);
            ++pos;
        }
        epos = uri.find_first_of(":/?#", pos);
        if (epos == std::string_view::npos) {
            p.host = uri.substr(pos);
            return p;
        }
        p.host = uri.substr(pos, epos - pos);
        if (uri[epos] == ':') {
            pos = epos + 1;
            epos = uri.find_first_of("/?#", pos);
            if (epos == std::string_view::npos) {
                p.port = uri.substr(pos);
                return p;
            }
            p.port = uri.substr(pos, epos - pos);
        }
    } else {
        epos = uri.find_first_of("/?#", pos);
        if (epos == std::string_view::npos)
            return p;
    }
    p.path = uri.substr(epos);
    return p;
}
inline constexpr auto parse(std::string_view uri,
                            std::string_view::size_type pos = 0) -> parts
{
    auto epos = uri.find(':', pos);
    if (epos == std::string_view::npos)
        return {};
    parts p;
    p.scheme = uri.substr(pos, epos - pos);
    pos = epos + 1;
    if (uri.compare(pos, 2, "//") == 0) {
        pos += 2;
        epos = uri.find('@', pos);
        if (epos != std::string_view::npos) {
            p.userinfo = uri.substr(pos, epos - pos);
            ++pos;
        }
        epos = uri.find_first_of(":/?#", pos);
        if (epos == std::string_view::npos) {
            p.host = uri.substr(pos);
            return p;
        }
        p.host = uri.substr(pos, epos - pos);
        if (uri[epos] == ':') {
            pos = epos + 1;
            epos = uri.find_first_of("/?#", pos);
            if (epos == std::string_view::npos) {
                p.port = uri.substr(pos);
                return p;
            }
            p.port = uri.substr(pos, epos - pos);
        }
    } else {
        epos = uri.find_first_of("/?#", pos);
        if (epos == std::string_view::npos)
            return p;
    }
    if (uri[epos] == '/') {
        pos = epos;
        epos = uri.find_first_of("?#", pos);
        if (epos == std::string_view::npos) {
            p.path = uri.substr(pos);
            return p;
        }
        p.path = uri.substr(pos, epos - pos);
    }
    if (uri[epos] == '?') {
        pos = epos + 1;
        epos = uri.find_first_of("#", pos);
        if (epos == std::string_view::npos) {
            p.query = uri.substr(pos);
            return p;
        }
        p.query = uri.substr(pos, epos - pos);
    }
    if (uri[epos] == '#') {
        pos = epos + 1;
        p.fragment = uri.substr(pos);
    }
    return p;
}
inline auto get(std::string_view resource) -> std::optional<
    boost::beast::http::response<boost::beast::http::string_body>>
{
    using namespace std::literals;
    constexpr auto http = "http"sv;
    constexpr auto https = "https"sv;
    if (auto [scheme, userinfo, host, port, path] = uri::parse_get(resource);
        scheme.size()) {
        if (scheme == http) {
            namespace net = boost::asio;
            net::io_context ioc;
            auto stream = http::make_stream(ioc);
            return http::get(stream, host, path, port.size() ? port : "80");
        } else if (scheme == https) {
            namespace net = boost::asio;
            net::io_context ioc;
            auto stream = https::make_stream(ioc);
            return https::get(stream, host, path, port.size() ? port : "443");
        }
    }
    return {};
}
} // namespace uri
#if __has_include(<doctest/doctest.h>)
#include <doctest/doctest.h>
TEST_CASE("uri parser")
{
    using namespace std::literals;
    constexpr auto view = "http://www.google.com:80/hello?query#fragment"sv;
    constexpr auto parts = uri::parse(view);
    REQUIRE_EQ(parts.scheme, "http");
    REQUIRE_EQ(parts.userinfo, "");
    REQUIRE_EQ(parts.host, "www.google.com");
    REQUIRE_EQ(parts.port, "80");
    REQUIRE_EQ(parts.path, "/hello");
    REQUIRE_EQ(parts.query, "query");
    REQUIRE_EQ(parts.fragment, "fragment");
}
#endif