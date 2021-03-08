#include "filedialog.h"
#include "image.h"
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

    auto url = std::array<char, 256>();
    auto texture = opengl::texture();

    while (!glfwWindowShouldClose(main_win.get())) {
        gl::glClear(gl::GL_COLOR_BUFFER_BIT);
        imgui_ctx.render([&]() {
            ImGui::GetBackgroundDrawList(ImGui::GetMainViewport())
                ->AddImage((void *)(intptr_t)(gl::GLuint)texture.get(),
                           ImGui::GetMainViewport()->WorkPos,
                           ImVec2(ImGui::GetMainViewport()->WorkPos.x +
                                      ImGui::GetMainViewport()->WorkSize.x,
                                  ImGui::GetMainViewport()->WorkPos.y +
                                      ImGui::GetMainViewport()->WorkSize.y));
            ImGui::InputText("url", url.data(), url.size());
            if (ImGui::Button("open")) {
                auto img = image::read_jpeg(std::string_view(url.data()));
                if (img) {
                    gl::glBindTexture(gl::GL_TEXTURE_2D,
                                      (gl::GLuint)texture.get());
                    image::glimage(boost::gil::view(*img));
                }
            }
            static bool show = false;
            if (show) {
                ImGui::ShowDemoWindow(&show);
                ImPlot::ShowDemoWindow(&show);
            }
        });
        glfwPollEvents();
    }
}
