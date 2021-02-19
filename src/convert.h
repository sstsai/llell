#pragma once
#include <tl/expected.hpp>
#include <type_traits>
#include <chrono>
#include <algorithm>
#include <system_error>
#include <array>
#if __has_include(<bit>)
#include <bit>
#endif
namespace convert {
enum class error {
    buffer_too_small = 1,
};
} // namespace convert
namespace std {
template <> struct is_error_code_enum<convert::error> : true_type {};
} // namespace std
namespace convert {
namespace detail {
class error_category : public std::error_category {
public:
    // Return a short descriptive name for the category
    virtual const char *name() const noexcept override final
    {
        return "convert_error";
    }
    // Return what each enum means in text
    virtual std::string message(int c) const override final
    {
        switch (static_cast<error>(c)) {
        case error::buffer_too_small:
            return "buffer too small";
        default:
            return "unknown";
        }
    }
};
error_category &err_category()
{
    static error_category c;
    return c;
}
} // namespace detail
inline std::error_code make_error_code(error e)
{
    return {static_cast<int>(e), detail::err_category()};
}
} // namespace convert
namespace convert {
template <typename T> using result_t = tl::expected<T, std::error_code>;
inline namespace type {
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
constexpr auto tag_invoke(to_fn<To> const &, From const &from) noexcept
    -> std::enable_if_t<std::is_constructible_v<To, From>, To>
{
    return To(from);
}
template <typename To, typename... Args>
constexpr auto to(Args &&...args) noexcept(noexcept(
    tag_invoke(std::declval<to_fn<To> &&>(), std::declval<Args &&>()...))) -> To
{
    return tag_invoke(to_fn<To>{}, std::forward<Args>(args)...);
}
} // namespace type
inline namespace ser {
template <std::endian Endian> struct serialize_fn {
    template <typename Iter, typename From>
    friend auto tag_invoke(serialize_fn<Endian> const &, Iter begin, Iter end,
                           From const &from) noexcept
        -> std::enable_if_t<std::is_arithmetic_v<From>, result_t<Iter>>
    {
        auto size = std::distance(begin, end);
        if (size < sizeof(From))
            return tl::make_unexpected(error::buffer_too_small);
        auto const *byte_ptr = reinterpret_cast<uint8_t const *>(&from);
        auto const *end_byte = byte_ptr + sizeof(From);
        if constexpr (Endian == std::endian::native) {
            return std::copy(byte_ptr, end_byte, begin);
        } else {
            if constexpr (sizeof(From) == 1) {
                return std::copy(byte_ptr, end_byte, begin);
            } else if constexpr (sizeof(From) == 2) {
                uint16_t x;
                auto *x_ptr = reinterpret_cast<uint8_t *>(&x);
                auto end_x = std::copy(byte_ptr, end_byte, x_ptr);
                x = (x << 8) | (x >> 8);
                return std::copy(x_ptr, end_x, begin);
            } else if constexpr (sizeof(From) == 4) {
                uint32_t x;
                auto *x_ptr = reinterpret_cast<uint8_t *>(&x);
                auto end_x = std::copy(byte_ptr, end_byte, x_ptr);
                x = x << 16 | x >> 16;
                x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff);
                return std::copy(x_ptr, end_x, begin);
            } else if constexpr (sizeof(From) == 8) {
                uint64_t x;
                auto *x_ptr = reinterpret_cast<uint8_t *>(&x);
                auto end_x = std::copy(byte_ptr, end_byte, x_ptr);
                x = x << 32 | x >> 32;
                x = (x & 0x0000FFFF0000FFFFULL) << 16 |
                    (x & 0xFFFF0000FFFF0000ULL) >> 16;
                x = (x & 0x00FF00FF00FF00FFULL) << 8 |
                    (x & 0xFF00FF00FF00FF00ULL) >> 8;
                return std::copy(x_ptr, end_x, begin);
            }
        }
        return end;
    }
    template <typename Iter, typename T, std::size_t N>
    friend auto tag_invoke(serialize_fn<Endian> const &, Iter begin, Iter end,
                           std::array<T, N> const &from) noexcept
        -> result_t<Iter>
    {
        auto size = std::distance(begin, end);
        if (size < N * sizeof(T))
            return tl::make_unexpected(error::buffer_too_small);
        for (auto const &value : from) {
            auto result = tag_invoke(serialize_fn<Endian>{}, begin, end, value);
            if (!result)
                return tl::make_unexpected(result.error());
            begin = result.value();
        }
        return begin;
    }
    template <typename... Args>
    constexpr auto operator()(Args &&...args) const
        noexcept(noexcept(tag_invoke(std::declval<serialize_fn<Endian> &&>(),
                                     std::declval<Args &&>()...)))
    {
        return tag_invoke(*this, std::forward<Args>(args)...);
    }
};
inline constexpr serialize_fn<std::endian::little> serialize;
inline constexpr serialize_fn<std::endian::big> serialize_be;
} // namespace ser
inline namespace des {
template <std::endian Endian> struct deserialize_fn {
    template <typename Iter, typename To>
    friend auto tag_invoke(deserialize_fn<Endian> const &, Iter begin, Iter end,
                           To &to) noexcept
        -> std::enable_if_t<std::is_arithmetic_v<To>, result_t<Iter>>
    {
        auto size = std::distance(begin, end);
        if (size < sizeof(To))
            return tl::make_unexpected(error::buffer_too_small);
        end = std::next(begin, sizeof(To));
        auto *byte_ptr = reinterpret_cast<uint8_t *>(&to);
        if constexpr (Endian == std::endian::native) {
            std::copy(begin, end, byte_ptr);
        } else {
            if constexpr (sizeof(To) == 1) {
                std::copy(begin, end, byte_ptr);
            } else if constexpr (sizeof(To) == 2) {
                uint16_t x;
                auto *x_ptr = reinterpret_cast<uint8_t *>(&x);
                std::copy(begin, end, x_ptr);
                x = (x << 8) | (x >> 8);
                std::copy(x_ptr, std::next(x_ptr, sizeof(x)), byte_ptr);
            } else if constexpr (sizeof(To) == 4) {
                uint32_t x;
                auto *x_ptr = reinterpret_cast<uint8_t *>(&x);
                std::copy(begin, end, x_ptr);
                x = x << 16 | x >> 16;
                x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff);
                std::copy(x_ptr, std::next(x_ptr, sizeof(x)), byte_ptr);
            } else if constexpr (sizeof(To) == 8) {
                uint64_t x;
                auto *x_ptr = reinterpret_cast<uint8_t *>(&x);
                std::copy(begin, end, x_ptr);
                x = x << 32 | x >> 32;
                x = (x & 0x0000FFFF0000FFFFULL) << 16 |
                    (x & 0xFFFF0000FFFF0000ULL) >> 16;
                x = (x & 0x00FF00FF00FF00FFULL) << 8 |
                    (x & 0xFF00FF00FF00FF00ULL) >> 8;
                std::copy(x_ptr, std::next(x_ptr, sizeof(x)), byte_ptr);
            }
        }
        return end;
    }
    template <typename Iter, typename T, std::size_t N>
    friend auto tag_invoke(deserialize_fn<Endian> const &, Iter begin, Iter end,
                           std::array<T, N> &from) noexcept -> result_t<Iter>
    {
        auto size = std::distance(begin, end);
        if (size < N * sizeof(T))
            return tl::make_unexpected(error::buffer_too_small);
        for (auto &value : from) {
            auto result =
                tag_invoke(deserialize_fn<Endian>{}, begin, end, value);
            if (!result)
                return tl::make_unexpected(result.error());
            begin = result.value();
        }
        return begin;
    }
    template <typename... Args>
    constexpr auto operator()(Args &&...args) const
        noexcept(noexcept(tag_invoke(std::declval<deserialize_fn<Endian> &&>(),
                                     std::declval<Args &&>()...)))
    {
        return tag_invoke(*this, std::forward<Args>(args)...);
    }
};
inline constexpr deserialize_fn<std::endian::little> deserialize;
inline constexpr deserialize_fn<std::endian::big> deserialize_be;
} // namespace des
} // namespace convert
#if __has_include(<doctest/doctest.h>)
#include <doctest/doctest.h>
TEST_CASE("convert")
{
    SUBCASE("to")
    {
        using convert::to;
        using namespace std::chrono;
        static_assert(to<int>(5) == 5, "int == int");
        REQUIRE_EQ(to<int>(5.2), 5);
        REQUIRE_EQ(to<seconds>(1999ms), 1s);
    }
    SUBCASE("serialize")
    {
        using convert::serialize;
        using convert::serialize_be;
        std::array<uint8_t, 128> arr;
        SUBCASE("bool")
        {
            auto result = serialize(arr.begin(), arr.end(), bool(true));
            REQUIRE(result);
            REQUIRE_EQ(std::distance(arr.begin(), result.value()), 1);
            REQUIRE_EQ(arr[0], 1);
        }
        SUBCASE("uint16_t")
        {
            auto result = serialize(arr.begin(), arr.end(), uint16_t(0x0102));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 2);
            REQUIRE_EQ(arr[1], 1);
            result = serialize_be(arr.begin(), arr.end(), uint16_t(0x0102));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 1);
            REQUIRE_EQ(arr[1], 2);
        }
        SUBCASE("uint32_t")
        {
            auto result =
                serialize(arr.begin(), arr.end(), uint32_t(0x01020304));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 4);
            REQUIRE_EQ(arr[1], 3);
            REQUIRE_EQ(arr[2], 2);
            REQUIRE_EQ(arr[3], 1);
            result = serialize_be(arr.begin(), arr.end(), uint32_t(0x01020304));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 1);
            REQUIRE_EQ(arr[1], 2);
            REQUIRE_EQ(arr[2], 3);
            REQUIRE_EQ(arr[3], 4);
        }
        SUBCASE("uint64_t")
        {
            auto result =
                serialize(arr.begin(), arr.end(), uint64_t(0x0102030405060708));
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 8);
            REQUIRE_EQ(arr[1], 7);
            REQUIRE_EQ(arr[2], 6);
            REQUIRE_EQ(arr[3], 5);
            REQUIRE_EQ(arr[4], 4);
            REQUIRE_EQ(arr[5], 3);
            REQUIRE_EQ(arr[6], 2);
            REQUIRE_EQ(arr[7], 1);
            result = serialize_be(arr.begin(), arr.end(),
                                  uint64_t(0x0102030405060708));
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
        SUBCASE("float")
        {
            auto result = serialize(arr.begin(), arr.end(), 1.0f);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 0x00);
            REQUIRE_EQ(arr[1], 0x00);
            REQUIRE_EQ(arr[2], 0x80);
            REQUIRE_EQ(arr[3], 0x3f);
            result = serialize_be(arr.begin(), arr.end(), 1.0f);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 0x3f);
            REQUIRE_EQ(arr[1], 0x80);
            REQUIRE_EQ(arr[2], 0x00);
            REQUIRE_EQ(arr[3], 0x00);
        }
        SUBCASE("double")
        {
            auto result = serialize(arr.begin(), arr.end(), 1.0);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 0x00);
            REQUIRE_EQ(arr[1], 0x00);
            REQUIRE_EQ(arr[2], 0x00);
            REQUIRE_EQ(arr[3], 0x00);
            REQUIRE_EQ(arr[4], 0x00);
            REQUIRE_EQ(arr[5], 0x00);
            REQUIRE_EQ(arr[6], 0xf0);
            REQUIRE_EQ(arr[7], 0x3f);
            result = serialize_be(arr.begin(), arr.end(), 1.0);
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
        SUBCASE("array")
        {
            std::array<uint8_t, 4> data;
            data = {1, 2, 3, 4};
            auto result = serialize(arr.begin(), arr.end(), data);
            REQUIRE(result);
            REQUIRE_EQ(std::distance(arr.begin(), result.value()), 4);
            REQUIRE_EQ(arr[0], 1);
            REQUIRE_EQ(arr[1], 2);
            REQUIRE_EQ(arr[2], 3);
            REQUIRE_EQ(arr[3], 4);
            result = serialize_be(arr.begin(), arr.end(), data);
            REQUIRE(result);
            REQUIRE_EQ(arr[0], 1);
            REQUIRE_EQ(arr[1], 2);
            REQUIRE_EQ(arr[2], 3);
            REQUIRE_EQ(arr[3], 4);
        }
    }
    SUBCASE("deserialize")
    {
        using convert::deserialize;
        using convert::deserialize_be;
        std::array<uint8_t, 128> arr{1, 2, 3, 4, 5, 6, 7, 8};
        SUBCASE("bool")
        {
            bool value;
            auto result = deserialize(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, true);
        }
        SUBCASE("uint16_t")
        {
            uint16_t value;
            auto result = deserialize(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0201);
            result = deserialize_be(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0102);
        }
        SUBCASE("uint32_t")
        {
            uint32_t value;
            auto result = deserialize(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x04030201);
            result = deserialize_be(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x01020304);
        }
        SUBCASE("uint32_t")
        {
            uint64_t value;
            auto result = deserialize(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0807060504030201);
            result = deserialize_be(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 0x0102030405060708);
        }
        SUBCASE("float")
        {
            arr = {0x00, 0x00, 0x80, 0x3f};
            float value;
            auto result = deserialize(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0f);
            arr = {0x3f, 0x80, 0x00, 0x00};
            result = deserialize_be(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0f);
        }
        SUBCASE("double")
        {
            arr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x3f};
            double value;
            auto result = deserialize(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0);
            arr = {0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            result = deserialize_be(arr.begin(), arr.end(), value);
            REQUIRE(result);
            REQUIRE_EQ(value, 1.0);
        }
        SUBCASE("array")
        {
            std::array<uint8_t, 4> data;
            arr = {1, 2, 3, 4};
            auto result = deserialize(arr.begin(), arr.end(), data);
            REQUIRE(result);
            REQUIRE_EQ(std::distance(arr.begin(), result.value()), 4);
            REQUIRE_EQ(data[0], 1);
            REQUIRE_EQ(data[1], 2);
            REQUIRE_EQ(data[2], 3);
            REQUIRE_EQ(data[3], 4);
            result = deserialize_be(arr.begin(), arr.end(), data);
            REQUIRE(result);
            REQUIRE_EQ(data[0], 1);
            REQUIRE_EQ(data[1], 2);
            REQUIRE_EQ(data[2], 3);
            REQUIRE_EQ(data[3], 4);
        }
    }
}
#endif
