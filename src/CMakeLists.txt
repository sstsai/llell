add_executable(llell "")
target_sources(llell
  PRIVATE
    "main.cpp"
  PUBLIC
    "convert.h"
    "http.h"
    "glfw.h"
    "imgui_.h"
    "imgui_widgets.h"
    "opengl.h"
    "timeout.h"
    "filedialog.h"
    "image.h"
    "uri.h")
target_include_directories(llell
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(llell PRIVATE doctest::doctest Boost::boost OpenSSL::SSL
                                    OpenSSL::Crypto tl::expected imgui::imgui
                                    nlohmann_json fmt::fmt-header-only
                                    implot::implot JPEG::JPEG
                                    unofficial::nativefiledialog::nfd
)
target_compile_definitions(llell PUBLIC _USE_MATH_DEFINES
                                        BOOST_BEAST_USE_STD_STRING_VIEW
                                        DOCTEST_CONFIG_DISABLE
                                        GLFW_INCLUDE_NONE
)
target_compile_features(llell PUBLIC cxx_std_20)

if (WIN32)
  target_compile_definitions(llell
    PUBLIC _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601
  )
  target_link_libraries(llell PRIVATE OpenSSL::applink crypt32.lib)
endif ()
if (UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(llell PRIVATE Threads::Threads)
endif ()
