set(CCACHE_VERSION 4.8)

find_program(CCACHE_PROGRAM ccache)
if (NOT CCACHE_PROGRAM)
  message(STATUS "Downloading ccache")
  if (WIN32)
    set(OS windows-i686.zip)
  elseif(UNIX AND NOT APPLE)
    set(OS linux-x86_64.tar.xz)
  elseif(APPLE)
    set(OS darwin.tar.gz)
  endif()
  include(get_cpm)
  CPMAddPackage(
    NAME ccache
    URL https://github.com/ccache/ccache/releases/download/v${CCACHE_VERSION}/ccache-${CCACHE_VERSION}-${OS}
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/external/ccache
    DOWNLOAD_ONLY YES
  )
  find_program(CCACHE_PROGRAM ccache PATHS ${ccache_SOURCE_DIR})
endif()
if(CCACHE_PROGRAM)
  set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}" CACHE INTERNAL "")
  set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE INTERNAL "")
endif()
