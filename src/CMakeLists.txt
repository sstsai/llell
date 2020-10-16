add_executable(llell "")
target_sources(llell
  PRIVATE
    "main.cpp"
)
target_include_directories(llell PRIVATE ${BOOST_BEAST_INCLUDE_DIRS})
target_include_directories(llell
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_compile_definitions(llell
  PUBLIC _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS _WIN32_WINNT=0x0601
)
target_compile_features(llell PUBLIC cxx_std_20)
if (UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(llell PRIVATE Threads::Threads)
endif ()
