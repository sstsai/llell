add_executable(tests "")
target_sources(tests
  PRIVATE
    "main.cpp"
)
target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_compile_definitions(llell
  PUBLIC _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601
)
target_compile_features(tests PRIVATE cxx_std_20)
target_link_libraries(tests
  PRIVATE doctest::doctest
          ${BOOST_BEAST_LIB_DIRS}/lib/libssl.lib
          ${BOOST_BEAST_LIB_DIRS}/lib/libcrypto.lib
          Crypt32.lib
)
include(doctest)
doctest_discover_tests(tests)
