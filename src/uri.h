#pragma once
#include <string_view>
#include <utility>
namespace uri {
struct parts {
    std::string_view scheme;
    std::string_view userinfo;
    std::string_view host;
    std::string_view port;
    std::string_view path;
    std::string_view query;
    std::string_view fragment;
};
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