add_executable(tests "")
target_sources(tests
  PRIVATE
    "main.cpp"
)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_compile_definitions(tests PUBLIC _USE_MATH_DEFINES)
target_compile_features(tests PRIVATE cxx_std_20)
target_link_libraries(tests PRIVATE doctest::doctest OpenSSL::SSL
                                    OpenSSL::Crypto
)
if (WIN32)
  target_compile_definitions(tests
    PUBLIC _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601
  )
  target_link_libraries(tests PRIVATE OpenSSL::applink crypt32.lib)
endif()

include(doctest)
doctest_discover_tests(tests)