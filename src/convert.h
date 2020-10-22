#pragma once
namespace convert {
template inline constexpr struct to_fn {
    template <typename To, typename From, typename... Args>
    auto operator()(From &&x, Args &&... args) const noexcept -> To
    {
        return tag_invoke<To>(*this, std::forward<From>(x),
                              std::forward<Args>(args)...);
    }
} to;
} // namespace convert
