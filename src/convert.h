#pragma once
#include <asio/buffer.hpp>
#include <asio/error.hpp>
#include <tl/expected.hpp>
#include <type_traits>
#include <chrono>
#if __has_include(<bit>)
#include <bit>
#endif
namespace convert {
inline namespace type {
template <typename T> struct to_fn {};
template <typename To>
constexpr auto tag_invoke(to_fn<To> const &, To const &from) noexcept -> To {
    return from;
}
template <class ToDuration, class Rep, class Period>
constexpr auto
tag_invoke(to_fn<ToDuration> const &,
           std::chrono::duration<Rep, Period> const &from) noexcept
    -> ToDuration {
    return std::chrono::duration_cast<ToDuration>(from);
}
template <typename To, typename From>
constexpr auto tag_invoke(to_fn<To> const &, From const &from) noexcept
    -> std::enable_if_t<std::is_constructible_v<To, From>, To> {
    return To(from);
}
template <typename To, typename... Args>
constexpr auto to(Args &&... args) noexcept(noexcept(tag_invoke(
    std::declval<to_fn<To> &&>(), std::declval<Args &&>()...))) -> To {
    return tag_invoke(to_fn<To>{}, std::forward<Args>(args)...);
}
} // namespace type
inline namespace ser {
template <typename T> using result = tl::expected<T, std::error_code>;
template <std::endian Endian> struct serialize_fn {
    template <typename From>
    friend auto tag_invoke(serialize_fn<Endian> const &,
                           asio::mutable_buffer to, From const &from) noexcept
        -> std::enable_if_t<std::is_arithmetic_v<From>,
                            result<asio::mutable_buffer>> {
        if (to.size() < sizeof(From))
            return tl::make_unexpected(asio::error::no_buffer_space);
        if constexpr (sizeof(From) == 1) {
            asio::buffer_copy(to, asio::buffer(&from, sizeof(From)));
        } else if constexpr (Endian == std::endian::native) {
            asio::buffer_copy(to, asio::buffer(&from, sizeof(From)));
        } else {
            if constexpr (sizeof(From) == 2) {
                uint16_t x;
                asio::buffer_copy(asio::buffer(&x, sizeof(x)),
                                  asio::buffer(&from, sizeof(From)));
                x = (x << 8) | (x >> 8);
                asio::buffer_copy(to, asio::buffer(&x, sizeof(x)));
            } else if constexpr (sizeof(From) == 4) {
                uint32_t x;
                asio::buffer_copy(asio::buffer(&x, sizeof(x)),
                                  asio::buffer(&from, sizeof(From)));
                x = x << 16 | x >> 16;
                x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff);
                asio::buffer_copy(to, asio::buffer(&x, sizeof(x)));
            } else if constexpr (sizeof(From) == 8) {
                uint64_t x;
                asio::buffer_copy(asio::buffer(&x, sizeof(x)),
                                  asio::buffer(&from, sizeof(From)));
                x = x << 32 | x >> 32;
                x = (x & 0x0000FFFF0000FFFFULL) << 16 |
                    (x & 0xFFFF0000FFFF0000ULL) >> 16;
                x = (x & 0x00FF00FF00FF00FFULL) << 8 |
                    (x & 0xFF00FF00FF00FF00ULL) >> 8;
                asio::buffer_copy(to, asio::buffer(&x, sizeof(x)));
            }
        }
        return to + sizeof(From);
    }
    template <typename... Args>
    constexpr auto operator()(Args &&... args) const
        noexcept(noexcept(tag_invoke(std::declval<serialize_fn<Endian> &&>(),
                                     std::declval<Args &&>()...))) {
        return tag_invoke(*this, std::forward<Args>(args)...);
    }
};
inline constexpr serialize_fn<std::endian::little> serialize;
inline constexpr serialize_fn<std::endian::big>    serialize_be;
struct is_serializable : std::false_type {};

} // namespace ser
inline namespace des {
template <typename T> using result = tl::expected<T, std::error_code>;
template <std::endian Endian> struct deserialize_fn {
    template <typename To>
    friend auto tag_invoke(deserialize_fn<Endian> const &,
                           asio::const_buffer from, To &to) noexcept
        -> std::enable_if_t<std::is_arithmetic_v<To>,
                            result<asio::const_buffer>> {
        if (from.size() < sizeof(To))
            return tl::make_unexpected(asio::error::no_buffer_space);
        if constexpr (sizeof(To) == 1) {
            asio::buffer_copy(asio::buffer(&to, sizeof(To)), from);
        } else if constexpr (Endian == std::endian::native) {
            asio::buffer_copy(asio::buffer(&to, sizeof(To)), from);
        } else {
            if constexpr (sizeof(To) == 2) {
                uint16_t x;
                asio::buffer_copy(asio::buffer(&x, sizeof(x)), from);
                x = (x << 8) | (x >> 8);
                asio::buffer_copy(asio::buffer(&to, sizeof(To)),
                                  asio::buffer(&x, sizeof(x)));
            } else if constexpr (sizeof(To) == 4) {
                uint32_t x;
                asio::buffer_copy(asio::buffer(&x, sizeof(x)), from);
                x = x << 16 | x >> 16;
                x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff);
                asio::buffer_copy(asio::buffer(&to, sizeof(To)),
                                  asio::buffer(&x, sizeof(x)));
            } else if constexpr (sizeof(To) == 8) {
                uint64_t x;
                asio::buffer_copy(asio::buffer(&x, sizeof(x)), from);
                x = x << 32 | x >> 32;
                x = (x & 0x0000FFFF0000FFFFULL) << 16 |
                    (x & 0xFFFF0000FFFF0000ULL) >> 16;
                x = (x & 0x00FF00FF00FF00FFULL) << 8 |
                    (x & 0xFF00FF00FF00FF00ULL) >> 8;
                asio::buffer_copy(asio::buffer(&to, sizeof(To)),
                                  asio::buffer(&x, sizeof(x)));
            }
        }
        return from + sizeof(To);
    }
    template <typename... Args>
    constexpr auto operator()(Args &&... args) const
        noexcept(noexcept(tag_invoke(std::declval<deserialize_fn<Endian> &&>(),
                                     std::declval<Args &&>()...))) {
        return tag_invoke(*this, std::forward<Args>(args)...);
    }
};
inline constexpr deserialize_fn<std::endian::little> deserialize;
inline constexpr deserialize_fn<std::endian::big>    deserialize_be;
} // namespace des
} // namespace convert
#if __has_include(<doctest/doctest.h>)
#include <doctest/doctest.h>
TEST_CASE("convert") {
    SUBCASE("to") {
        using convert::to;
        using namespace std::chrono;
        static_assert(to<int>(5) == 5, "int == int");
        REQUIRE_EQ(to<int>(5.2), 5);
        REQUIRE_EQ(to<seconds>(1999ms), 1s);
    }
    SUBCASE("serialize") {
        using convert::serialize;
        using convert::serialize_be;
        std::array<uint8_t, 128> arr;
        auto                     buf = asio::buffer(arr);
        SUBCASE("bool") {
            auto result = serialize(buf, bool(true));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 1);
        }
        SUBCASE("uint16_t") {
            auto result = serialize(buf, uint16_t(0x0102));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 2);
            REQUIRE_EQ(arr[1], 1);
            result = serialize_be(buf, uint16_t(0x0102));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 1);
            REQUIRE_EQ(arr[1], 2);
        }
        SUBCASE("uint32_t") {
            auto result = serialize(buf, uint32_t(0x01020304));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 4);
            REQUIRE_EQ(arr[1], 3);
            REQUIRE_EQ(arr[2], 2);
            REQUIRE_EQ(arr[3], 1);
            result = serialize_be(buf, uint32_t(0x01020304));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 1);
            REQUIRE_EQ(arr[1], 2);
            REQUIRE_EQ(arr[2], 3);
            REQUIRE_EQ(arr[3], 4);
        }
        SUBCASE("uint64_t") {
            auto result = serialize(buf, uint64_t(0x0102030405060708));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 8);
            REQUIRE_EQ(arr[1], 7);
            REQUIRE_EQ(arr[2], 6);
            REQUIRE_EQ(arr[3], 5);
            REQUIRE_EQ(arr[4], 4);
            REQUIRE_EQ(arr[5], 3);
            REQUIRE_EQ(arr[6], 2);
            REQUIRE_EQ(arr[7], 1);
            result = serialize_be(buf, uint64_t(0x0102030405060708));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 1);
            REQUIRE_EQ(arr[1], 2);
            REQUIRE_EQ(arr[2], 3);
            REQUIRE_EQ(arr[3], 4);
            REQUIRE_EQ(arr[4], 5);
            REQUIRE_EQ(arr[5], 6);
            REQUIRE_EQ(arr[6], 7);
            REQUIRE_EQ(arr[7], 8);
        }
        SUBCASE("float") {
            auto result = serialize(buf, 1.0f);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 0x00);
            REQUIRE_EQ(arr[1], 0x00);
            REQUIRE_EQ(arr[2], 0x80);
            REQUIRE_EQ(arr[3], 0x3f);
            result = serialize_be(buf, 1.0f);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 0x3f);
            REQUIRE_EQ(arr[1], 0x80);
            REQUIRE_EQ(arr[2], 0x00);
            REQUIRE_EQ(arr[3], 0x00);
        }
        SUBCASE("double") {
            auto result = serialize(buf, 1.0);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 0x00);
            REQUIRE_EQ(arr[1], 0x00);
            REQUIRE_EQ(arr[2], 0x00);
            REQUIRE_EQ(arr[3], 0x00);
            REQUIRE_EQ(arr[4], 0x00);
            REQUIRE_EQ(arr[5], 0x00);
            REQUIRE_EQ(arr[6], 0xf0);
            REQUIRE_EQ(arr[7], 0x3f);
            result = serialize_be(buf, 1.0);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 0x3f);
            REQUIRE_EQ(arr[1], 0xf0);
            REQUIRE_EQ(arr[2], 0x00);
            REQUIRE_EQ(arr[3], 0x00);
            REQUIRE_EQ(arr[4], 0x00);
            REQUIRE_EQ(arr[5], 0x00);
            REQUIRE_EQ(arr[6], 0x00);
            REQUIRE_EQ(arr[7], 0x00);
        }
    }
    SUBCASE("deserialize") {
        using convert::deserialize;
        using convert::deserialize_be;
        std::array<uint8_t, 128> arr{1, 2, 3, 4, 5, 6, 7, 8};
        auto                     buf = asio::buffer(arr);
        SUBCASE("bool") {
            bool value;
            auto result = deserialize(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, true);
        }
        SUBCASE("uint16_t") {
            uint16_t value;
            auto     result = deserialize(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0201);
            result = deserialize_be(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0102);
        }
        SUBCASE("uint32_t") {
            uint32_t value;
            auto     result = deserialize(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x04030201);
            result = deserialize_be(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x01020304);
        }
        SUBCASE("uint32_t") {
            uint64_t value;
            auto     result = deserialize(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0807060504030201);
            result = deserialize_be(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0102030405060708);
        }
        SUBCASE("float") {
            arr = {0x00, 0x00, 0x80, 0x3f};
            float value;
            auto  result = deserialize(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0f);
            arr = {0x3f, 0x80, 0x00, 0x00};
            result = deserialize_be(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0f);
        }
        SUBCASE("double") {
            arr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f};
            double value;
            auto  result = deserialize(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0);
            arr = {0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            result = deserialize_be(buf, value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0);
        }
    }
}
#endif
