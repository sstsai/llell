#pragma once
#include <string_view>
#include <optional>
namespace uri {
struct state {
    std::string_view remaining;
    std::string_view parsed = std::string_view();
};
inline constexpr auto scheme(state uri) -> state
{
    constexpr auto delimiter = ':';
    if (auto end = uri.remaining.find_first_of(delimiter);
        end != std::string_view::npos) {
        uri.parsed = uri.remaining.substr(0, end);
        uri.remaining = uri.remaining.substr(end + sizeof(delimiter));
    }
    return uri;
}
inline constexpr auto userinfo(state uri) -> state
{
    constexpr auto delimiter = '@';
    if (auto end = uri.remaining.find_first_of(delimiter);
        end != std::string_view::npos) {
        uri.parsed = uri.remaining.substr(0, end);
        uri.remaining = uri.remaining.substr(end + sizeof(delimiter));
    }
    return uri;
}
inline constexpr auto host(state uri) -> state
{
    constexpr auto delimiter = ':';
    if (auto end = uri.remaining.find_first_of(delimiter);
        end != std::string_view::npos) {
        uri.parsed = uri.remaining.substr(0, end);
        uri.remaining = uri.remaining.substr(end + sizeof(delimiter));
    } else {
        uri.parsed = uri.remaining;
        uri.remaining = std::string_view();
    }
    return uri;
}
inline constexpr auto port(state uri) -> state
{
    return state{uri.remaining, std::string_view()};
}
inline constexpr auto authority(state uri) -> state
{
    constexpr auto delimiter = std::string_view("//");
    constexpr auto path_delimiter = '/';
    if (uri.remaining.size() >= delimiter.size() &&
        uri.remaining.substr(0, delimiter.size()) == delimiter) {
        auto end =
            uri.remaining.find_first_of(path_delimiter, delimiter.size());
        if (end == std::string_view::npos) {
            uri.parsed = uri.remaining.substr(delimiter.size());
            uri.remaining = std::string_view();
        } else {
            uri.parsed =
                uri.remaining.substr(delimiter.size(), end - delimiter.size());
            uri.remaining = uri.remaining.substr(end);
        }
    }
    return uri;
}
inline constexpr auto path(state uri) -> state
{
    constexpr auto delimiter = '?';
    if (auto end = uri.remaining.find_first_of(delimiter);
        end != std::string_view::npos) {
        uri.parsed = uri.remaining.substr(0, end);
        uri.remaining = uri.remaining.substr(end + sizeof(delimiter));
    } else {
        uri.parsed = uri.remaining;
        uri.remaining = std::string_view();
    }
    return uri;
}
inline constexpr auto query(state uri) -> state
{
    constexpr auto delimiter = '#';
    if (auto end = uri.remaining.find_first_of(delimiter);
        end != std::string_view::npos) {
        uri.parsed = uri.remaining.substr(0, end);
        uri.remaining = uri.remaining.substr(end + sizeof(delimiter));
    } else {
        uri.parsed = uri.remaining;
        uri.remaining = std::string_view();
    }
    return uri;
}
inline constexpr auto fragment(state uri) -> state
{
    return state{uri.remaining, std::string_view()};
}
} // namespace uri
