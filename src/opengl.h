#pragma once
#include <glbinding/gl32core/gl.h>
#include <fmt/printf.h>
#include <memory>
#include <string_view>
#include <array>
#include <type_traits>
namespace opengl {
template <void (*func)(gl::GLuint)> struct deleter {
    struct pointer {
        gl::GLuint x;
        pointer(std::nullptr_t = nullptr) : x(0) {}
        pointer(gl::GLuint x) : x(x) {}
        explicit operator gl::GLuint() const { return x; }
        explicit operator bool() const { return x; }
        friend bool operator==(pointer x, pointer y) { return x.x == y.x; }
        friend bool operator!=(pointer x, pointer y) { return x.x != y.x; }
    };
    void operator()(pointer p) const { func(static_cast<gl::GLuint>(p)); }
};
template <void (*func)(gl::GLsizei, gl::GLuint const *)> struct deleter1 {
    struct pointer {
        gl::GLuint x;
        pointer(std::nullptr_t = nullptr) : x(0) {}
        pointer(gl::GLuint x) : x(x) {}
        explicit operator gl::GLuint() const { return x; }
        explicit operator gl::GLuint const *() const { return &x; }
        explicit operator bool() const { return x; }
        friend bool operator==(pointer x, pointer y) { return x.x == y.x; }
        friend bool operator!=(pointer x, pointer y) { return x.x != y.x; }
    };
    void operator()(pointer p) const
    {
        func(1, static_cast<gl::GLuint const *>(p));
    }
};
using frame_buffer_handle =
    std::unique_ptr<void, deleter1<gl::glDeleteFramebuffers>>;
inline auto frame_buffer()
{
    gl::GLuint handle{};
    gl::glGenFramebuffers(1, &handle);
    return frame_buffer_handle(handle);
}
using vertex_array_handle =
    std::unique_ptr<void, deleter1<gl::glDeleteVertexArrays>>;
inline auto vertex_array()
{
    gl::GLuint handle{};
    gl::glGenVertexArrays(1, &handle);
    return vertex_array_handle(handle);
}
using buffer_handle = std::unique_ptr<void, deleter1<gl::glDeleteBuffers>>;
inline auto buffer()
{
    gl::GLuint handle{};
    gl::glGenBuffers(1, &handle);
    return buffer_handle(handle);
}
using texture_handle = std::unique_ptr<void, deleter1<gl::glDeleteTextures>>;
inline auto texture()
{
    gl::GLuint handle{};
    gl::glGenTextures(1, &handle);
    return texture_handle(handle);
}
using sampler_handle = std::unique_ptr<void, deleter1<gl::glDeleteSamplers>>;
inline auto sampler()
{
    gl::GLuint handle{};
    gl::glGenSamplers(1, &handle);
    return sampler_handle(handle);
}
using shader_handle = std::unique_ptr<void, deleter<gl::glDeleteShader>>;
inline auto shader(gl::GLenum type)
{
    return shader_handle(gl::glCreateShader(type));
}
using program_handle = std::unique_ptr<void, deleter<gl::glDeleteProgram>>;
inline auto program() { return program_handle(gl::glCreateProgram()); }
template <gl::GLenum Type> struct shader_source {
    std::string_view source;
};
using vertex_source = shader_source<gl::GL_VERTEX_SHADER>;
using fragment_source = shader_source<gl::GL_FRAGMENT_SHADER>;
template <typename T> inline gl::GLuint gluint(T const &obj)
{
    return static_cast<gl::GLuint>(obj.get());
}
template <gl::GLenum Type> auto compile(shader_source<Type> src)
{
    auto s = shader(Type);
    if (s) {
        auto handle = gluint(s);
        gl::GLint len[1] = {static_cast<gl::GLint>(src.source.size())};
        auto ptr = src.source.data();
        gl::glShaderSource(handle, 1, &ptr, len);
        gl::glCompileShader(handle);
        gl::GLint success;
        gl::glGetShaderiv(handle, gl::GL_COMPILE_STATUS, &success);
        if (!success) {
            char infoLog[512];
            gl::glGetShaderInfoLog(handle, sizeof(infoLog), NULL, infoLog);
            fmt::print("compile error:{}\n", infoLog);
        };
    }
    return s;
}
namespace details {
template <class T, class... Ts>
struct are_same : std::conjunction<std::is_same<T, Ts>...> {};
} // namespace details
template <typename Shader, typename... Shaders,
          typename = details::are_same<Shader, Shaders...>>
auto link(Shader &&shader, Shaders &&... shaders)
{
    auto p = program();
    if (p) {
        auto handle = gluint(p);
        gl::glAttachShader(handle, gluint(shader));
        (gl::glAttachShader(handle, gluint(shaders)), ...);
        gl::glLinkProgram(handle);
        gl::GLint success;
        gl::glGetProgramiv(handle, gl::GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            gl::glGetProgramInfoLog(handle, sizeof(infoLog), NULL, &infoLog[0]);
            fmt::print("link error:{}\n", infoLog);
        }
        gl::glDetachShader(handle, gluint(shader));
        (gl::glDetachShader(handle, gluint(shaders)), ...);
    }
    return p;
}
} // namespace opengl
