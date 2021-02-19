#pragma once
#include <GLFW/glfw3.h>
#include <fmt/printf.h>
#include <tl/expected.hpp>
#include <system_error>
#include <memory>
namespace glfw {
namespace details {
struct error_category : std::error_category {
    const char *name() const noexcept override { return "glfw"; }
    std::string message(int ev) const override
    {
        switch (ev) {
        case GLFW_NO_ERROR:
            return "no error";
        case GLFW_NOT_INITIALIZED:
            return "not initialized";
        case GLFW_NO_CURRENT_CONTEXT:
            return "no current context";
        case GLFW_INVALID_ENUM:
            return "invalid enum";
        case GLFW_INVALID_VALUE:
            return "invalid value";
        case GLFW_OUT_OF_MEMORY:
            return "out of memory";
        case GLFW_API_UNAVAILABLE:
            return "api unavaliable";
        case GLFW_VERSION_UNAVAILABLE:
            return "version unavaliable";
        case GLFW_PLATFORM_ERROR:
            return "platform error";
        case GLFW_FORMAT_UNAVAILABLE:
            return "format unavailable";
        case GLFW_NO_WINDOW_CONTEXT:
            return "no window context";
        }
        return "unspecified error";
    }
};
} // namespace details
std::error_code make_error_code(int e)
{
    return {e, details::error_category()};
}
std::error_code make_error_code()
{
    return make_error_code(glfwGetError(nullptr));
}
namespace details {
struct terminate {
    struct pointer {
    private:
        bool is_active = false;

    public:
        pointer(std::nullptr_t = nullptr) {}
        pointer(bool b) : is_active(b) {}
        explicit operator bool() const { return is_active; }
        friend bool operator==(pointer l, pointer r)
        {
            return l.is_active == r.is_active;
        }
        friend bool operator!=(pointer l, pointer r)
        {
            return l.is_active != r.is_active;
        }
    };
    void operator()(pointer) const { glfwTerminate(); }
};
using context = std::unique_ptr<void, terminate>;
inline auto make_context() -> tl::expected<context, std::error_code>
{
    if (glfwInit() == GLFW_FALSE)
        return tl::make_unexpected(make_error_code());
    return context(true);
}
} // namespace details
struct destroy_window {
    void operator()(GLFWwindow *p) const { glfwDestroyWindow(p); }
};
using window = std::unique_ptr<GLFWwindow, destroy_window>;
template <int Attribute, typename T = bool> struct attribute {
    static constexpr int id = Attribute;
    T value{};
};
namespace attrib {
inline constexpr int dont_care = GLFW_DONT_CARE;
enum class client_apis {
    none = GLFW_NO_API,
    opengl = GLFW_OPENGL_API,
    opengl_es = GLFW_OPENGL_ES_API,
};
enum class robustness_strategy {
    none = GLFW_NO_ROBUSTNESS,
    no_reset_notification = GLFW_NO_RESET_NOTIFICATION,
    lose_context_on_reset = GLFW_LOSE_CONTEXT_ON_RESET,
};
enum class opengl_profiles {
    any = GLFW_OPENGL_ANY_PROFILE,
    core = GLFW_OPENGL_CORE_PROFILE,
    compat = GLFW_OPENGL_COMPAT_PROFILE,
};
enum class release_behavior {
    any = GLFW_ANY_RELEASE_BEHAVIOR,
    flush = GLFW_RELEASE_BEHAVIOR_FLUSH,
    none = GLFW_RELEASE_BEHAVIOR_NONE,
};
enum class creation_apis {
    native = GLFW_NATIVE_CONTEXT_API,
    egl = GLFW_EGL_CONTEXT_API,
    osmesa = GLFW_OSMESA_CONTEXT_API,
};
using focused = attribute<GLFW_FOCUSED, bool>;
using iconified = attribute<GLFW_ICONIFIED, bool>;
using resizeable = attribute<GLFW_RESIZABLE, bool>;
using visible = attribute<GLFW_VISIBLE, bool>;
using decorated = attribute<GLFW_DECORATED, bool>;
using auto_iconify = attribute<GLFW_AUTO_ICONIFY, bool>;
using floating = attribute<GLFW_FLOATING, bool>;
using maximized = attribute<GLFW_MAXIMIZED, bool>;
using center_cursor = attribute<GLFW_CENTER_CURSOR, bool>;
using transparent_framebuffer = attribute<GLFW_TRANSPARENT_FRAMEBUFFER, bool>;
using hovered = attribute<GLFW_HOVERED, bool>;
using focus_on_show = attribute<GLFW_FOCUS_ON_SHOW, bool>;
using red_bits = attribute<GLFW_RED_BITS, int>;
using green_bits = attribute<GLFW_GREEN_BITS, int>;
using blue_bits = attribute<GLFW_BLUE_BITS, int>;
using alpha_bits = attribute<GLFW_ALPHA_BITS, int>;
using depth_bits = attribute<GLFW_DEPTH_BITS, int>;
using stencil_bits = attribute<GLFW_STENCIL_BITS, int>;
using stereo = attribute<GLFW_STEREO, bool>;
using samples = attribute<GLFW_SAMPLES, int>;
using srgb_capable = attribute<GLFW_SRGB_CAPABLE, bool>;
using refresh_rate = attribute<GLFW_REFRESH_RATE, int>;
using double_buffer = attribute<GLFW_DOUBLEBUFFER, bool>;
using client_api = attribute<GLFW_CLIENT_API, client_apis>;
using context_version_major = attribute<GLFW_CONTEXT_VERSION_MAJOR, int>;
using context_version_minor = attribute<GLFW_CONTEXT_VERSION_MINOR, int>;
using context_revision = attribute<GLFW_CONTEXT_REVISION, int>;
using context_robustness =
    attribute<GLFW_CONTEXT_ROBUSTNESS, robustness_strategy>;
using opengl_forward_compat = attribute<GLFW_OPENGL_FORWARD_COMPAT, bool>;
using opengl_debug_context = attribute<GLFW_OPENGL_DEBUG_CONTEXT, bool>;
using opengl_profile = attribute<GLFW_OPENGL_PROFILE, opengl_profiles>;
using context_release_behavior =
    attribute<GLFW_CONTEXT_RELEASE_BEHAVIOR, release_behavior>;
using context_no_error = attribute<GLFW_CONTEXT_NO_ERROR, bool>;
using context_creation_api =
    attribute<GLFW_CONTEXT_CREATION_API, creation_apis>;
using scale_to_monitor = attribute<GLFW_SCALE_TO_MONITOR, bool>;
using cocoa_retina_framebuffer = attribute<GLFW_COCOA_RETINA_FRAMEBUFFER, bool>;
using cocoa_frame_name = attribute<GLFW_COCOA_FRAME_NAME, char const *>;
using cocoa_graphics_switching = attribute<GLFW_COCOA_GRAPHICS_SWITCHING, bool>;
using x11_class_name = attribute<GLFW_X11_CLASS_NAME, char const *>;
using x11_instance_name = attribute<GLFW_X11_INSTANCE_NAME, char const *>;
} // namespace attrib
template <int Attribute, typename T>
void hint(attribute<Attribute, T> const &attrib)
{
    glfwWindowHint(Attribute, static_cast<int>(attrib.value));
}
template <typename... Attribs>
auto make_window(int width, int height, const char *title, GLFWmonitor *monitor,
                 GLFWwindow *share, Attribs &&...attribs)
    -> tl::expected<window, std::error_code>
{
    static auto ctx = details::make_context().value();
    (hint(attribs), ...);
    auto *win = glfwCreateWindow(width, height, title, monitor, share);
    if (!win)
        return tl::make_unexpected(make_error_code());
    return window(win);
}
} // namespace glfw
