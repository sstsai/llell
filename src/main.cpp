#include "imgui_.h"
#include "glfw.h"
#include "opengl.h"
#include <glbinding/glbinding.h>

int main(int, char *av[])
{
    auto main_win = glfw::make_window(1280, 720, "Main", nullptr, nullptr,
                                      glfw::attrib::context_version_major{3},
                                      glfw::attrib::context_version_minor{2},
                                      glfw::attrib::opengl_profile{
                                          glfw::attrib::opengl_profiles::core},
                                      glfw::attrib::opengl_forward_compat{true})
                        .value();
    glfwMakeContextCurrent(main_win.get());
    glbinding::initialize(glfwGetProcAddress);
    using namespace imgui;
    auto imgui_ctx = glfw_opengl(main_win.get());
    gl::glClearColor(.7f, .3f, .5f, 1.f);
    while (!glfwWindowShouldClose(main_win.get())) {
        gl::glClear(gl::GL_COLOR_BUFFER_BIT);
        imgui_ctx.render([]() {
            static bool show = true;
            ImGui::ShowDemoWindow(&show);
        });
        glfwPollEvents();
    }
}
