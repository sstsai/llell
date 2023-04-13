set(VCPKG_VERSION 2023.02.24)

if(NOT DEFINED $ENV{VCPKG_ROOT})
  message(STATUS "Downloading vcpkg")
  include(get_cpm)
  CPMAddPackage(
    NAME vcpkg
    URL https://github.com/microsoft/vcpkg/archive/refs/tags/${VCPKG_VERSION}.tar.gz
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/vcpkg
    DOWNLOAD_ONLY YES
  )
  include(${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake)
else()
  include($ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()
