add_executable(tests "")
target_sources(tests
  PRIVATE
    "main.cpp"
)
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(tests PUBLIC _USE_MATH_DEFINES
                                        GLFW_INCLUDE_NONE
)
target_compile_features(tests PRIVATE cxx_std_23)
target_link_libraries(tests PRIVATE doctest::doctest Boost::boost OpenSSL::SSL
                                    OpenSSL::Crypto imgui::imgui tl::expected
                                    nlohmann_json fmt::fmt-header-only
                                    implot::implot glbinding::glbinding
                                    unofficial::nativefiledialog::nfd
)
if (WIN32)
  target_compile_definitions(tests
    PUBLIC _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601
  )
  target_link_libraries(tests PRIVATE OpenSSL::applink crypt32.lib)
endif()

include(doctest)
doctest_discover_tests(tests)
