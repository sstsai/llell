#pragma once
#include <GLFW/glfw3.h>
#include <glbinding/gl32core/gl.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <imgui_fonts_droid_sans.h>
#include <imgui_freetype.h>
#include <implot.h>
#include <memory>
namespace imgui {
struct destroy_context {
    void operator()(ImGuiContext *c) const { ImGui::DestroyContext(c); }
};
using context = std::unique_ptr<ImGuiContext, destroy_context>;
context make_context()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    static ImFontAtlas atlas;
    auto ctx = ImGui::CreateContext(&atlas);
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable
    // Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data,
                                             droid_sans_compressed_size, 18.0f);
    ImFontConfig *font_config = (ImFontConfig *)&io.Fonts->ConfigData[0];
    strcpy(font_config->Name, "DroidSans.ttf, 18px");
    ImGuiFreeType::BuildFontAtlas(io.Fonts, 0);
    return context(ctx);
}
struct destroy_plot_context {
    void operator()(ImPlotContext *c) const { ImPlot::DestroyContext(c); }
};
using plot_context = std::unique_ptr<ImPlotContext, destroy_plot_context>;
plot_context make_plot_context()
{
    return plot_context(ImPlot::CreateContext());
}
struct glfw_opengl {
private:
    context ctx = make_context();
    plot_context pctx = make_plot_context();

public:
    glfw_opengl(GLFWwindow *window)
    {
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
    }
    ~glfw_opengl()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
    }
    template <typename Fn> void render(Fn &&fn)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        std::forward<Fn>(fn)();
        ImGui::Render();
        auto window =
            static_cast<GLFWwindow *>(ImGui::GetMainViewport()->PlatformHandle);
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        gl::glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we
        // save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call
        //  glfwMakeContextCurrent(window) directly)
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow *backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
        glfwSwapBuffers(window);
    }
};
} // namespace imgui
