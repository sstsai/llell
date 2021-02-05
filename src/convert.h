#pragma once
#include <type_traits>
#include <chrono>
namespace convert {
template <typename T> struct to_fn {};
template <typename To>
constexpr auto tag_invoke(to_fn<To> const &, To const &from) noexcept -> To
{
    return from;
}
template <class ToDuration, class Rep, class Period>
constexpr auto
tag_invoke(to_fn<ToDuration> const &,
           std::chrono::duration<Rep, Period> const &from) noexcept
    -> ToDuration
{
    return std::chrono::duration_cast<ToDuration>(from);
}
template <typename To, typename From>
requires(std::is_constructible_v<To, From>) constexpr auto tag_invoke(
    to_fn<To> const &, From const &from) noexcept -> To
{
    return To(from);
}
template <typename To, typename... Args>
constexpr auto to(Args &&...args) noexcept(noexcept(
    tag_invoke(std::declval<to_fn<To> &&>(), std::declval<Args &&>()...))) -> To
{
    return tag_invoke(to_fn<To>{}, std::forward<Args>(args)...);
}
} // namespace convert
#if __has_include(<doctest/doctest.h>)
#include <doctest/doctest.h>
TEST_CASE("convert")
{
    using namespace convert;
    SUBCASE("to")
    {
        using namespace std::chrono;
        static_assert(to<int>(5) == 5, "int == int");
        REQUIRE_EQ(to<int>(5.2), 5);
        REQUIRE_EQ(to<seconds>(1999ms), 1s);
    }
}
#endif
