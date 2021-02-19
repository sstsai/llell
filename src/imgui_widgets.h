#pragma once
#include "tag_invoke.h"
#include <imgui.h>
#include <vector>
#include <tuple>
#include <functional>
namespace imgui {
inline constexpr struct draw_fn {
    template <typename T, typename... Args>
    auto operator()(T &&x, Args &&... args) const
        noexcept(pre::is_nothrow_tag_invocable_v<draw_fn, T, Args...>)
            -> pre::tag_invoke_result_t<draw_fn, T, Args...>
    {
        return pre::tag_invoke(*this, std::forward<T>(x),
                               std::forward<Args>(args)...);
    }
} draw{};
using on_click = std::function<void()>;
inline constexpr auto no_click = []() {};
template <typename Param, typename... Children> struct widget {
    Param param;
    std::tuple<Children...> children;
    template <typename P, typename... Cs>
    widget(P &&p, Cs &&... cs)
        : param(std::forward<P>(p)), children(std::forward<Cs>(cs)...)
    {}

private:
    friend void tag_invoke(pre::tag_t<draw>,
                           widget<Param, Children...> const &w)
    {
        std::apply(
            [&w](auto &&... xs) {
                draw(w.param, std::forward<decltype(xs)>(xs)...);
            },
            w.children);
    }
};
template <typename P, typename... Cs>
widget(P p, Cs &&... cs) -> widget<P, Cs...>;
template <typename T> struct id {
    T label;
    id(T label) : label(label) {}

private:
    template <typename... Children>
    friend void tag_invoke(pre::tag_t<draw>, id const &i,
                           Children &&... children) noexcept
    {
        ImGui::PushID(i.label);
        (draw(std::forward<Children>(children)), ...);
        ImGui::PopID();
    }
};
struct separator {
private:
    friend void tag_invoke(pre::tag_t<draw>, separator const &) noexcept
    {
        ImGui::Separator();
    }
};
struct same_line {
    float offset_from_start_x = 0.0f;
    float spacing = -1.0f;

private:
    friend void tag_invoke(pre::tag_t<draw>, same_line const &s) noexcept
    {
        ImGui::SameLine(s.offset_from_start_x, s.spacing);
    }
};
struct selectable {
    char const *label = nullptr;
    on_click clicked = no_click;
    bool is_pressed = false;
    ImGuiSelectableFlags flags = ImGuiSelectableFlags_None;
    ImVec2 size = ImVec2(0, 0);

private:
    friend void tag_invoke(pre::tag_t<draw>, selectable const &s) noexcept
    {
        if (ImGui::Selectable(s.label, s.is_pressed, s.flags, s.size)) {
            s.clicked();
        }
    }
};
struct combo {
    char const *label = nullptr;
    on_click clicked = no_click;
    char const *preview = nullptr;
    ImGuiComboFlags flags = ImGuiComboFlags_None;

private:
    void select(selectable &s)
    {
        s.is_pressed = (preview == s.label);
        draw(s);
        if (s.is_pressed)
            preview = s.label;
    }

    template <typename... Children>
    friend void tag_invoke(pre::tag_t<draw>, combo &c,
                           Children &&... children) noexcept
    {
        if (ImGui::BeginCombo(c.label, c.preview, c.flags)) {
            (c.select(children), ...);
            ImGui::EndCombo();
        }
    }
};
struct menu_item {
    char const *label = nullptr;
    on_click clicked = no_click;
    char const *shortcut = nullptr;
    bool selected = false;
    bool enabled = true;

private:
    friend void tag_invoke(pre::tag_t<draw>, menu_item &m) noexcept
    {
        if (ImGui::MenuItem(m.label, m.shortcut, &m.selected, m.enabled)) {
            m.clicked();
        }
    }
};
struct menu {
    char const *label = nullptr;
    bool enabled = true;

private:
    template <typename... Children>
    friend void tag_invoke(pre::tag_t<draw>, menu const &m,
                           Children &&... children) noexcept
    {
        if (ImGui::BeginMenu(m.label, m.enabled)) {
            (draw(std::forward<Children>(children)), ...);
            ImGui::EndMenu();
        }
    };
};
struct main_menubar {
private:
    template <typename... Children>
    friend void tag_invoke(pre::tag_t<draw>, main_menubar const &m,
                           Children &&... children) noexcept
    {
        if (ImGui::BeginMainMenuBar()) {
            (draw(std::forward<Children>(children)), ...);
            ImGui::EndMainMenuBar();
        }
    };
};
struct window {
    char const *label = nullptr;
    on_click clicked = no_click;
    bool *is_opened = nullptr;
    ImGuiWindowFlags flags = ImGuiWindowFlags_None;

private:
    template <typename... Children>
    friend void tag_invoke(pre::tag_t<draw>, window const &w,
                           Children &&... children) noexcept
    {
        if (!w.is_opened || *w.is_opened) {
            if (ImGui::Begin(w.label, w.is_opened, w.flags)) {
                w.clicked();
                (draw(std::forward<Children>(children)), ...);
            }
            ImGui::End();
        }
    };
};
struct image {
    ImTextureID user_texture_id = nullptr;
    ImVec2 size = ImVec2(0, 0);
    ImVec2 uv0 = ImVec2(0, 0);
    ImVec2 uv1 = ImVec2(1, 1);
    ImVec4 tint_col = ImVec4(1, 1, 1, 1);
    ImVec4 order_col = ImVec4(0, 0, 0, 0);

private:
    friend void tag_invoke(pre::tag_t<draw>, image const &i) noexcept
    {
        ImGui::Image(i.user_texture_id, i.size, i.uv0, i.uv1, i.tint_col,
                     i.order_col);
    }
};
} // namespace imgui
