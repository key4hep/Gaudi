# Special wrapper to load the declared version of the heptools toolchain.
set(heptools_version 70root6)

if(DEFINED ENV{HEPTOOLS_VERSION})
  set(heptools_version $ENV{HEPTOOLS_VERSION})
endif()

# this check is needed because the toolchain is called when checking the
# compiler (without the proper cache)
if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeTmp")

  find_file(default_toolchain NAMES GaudiDefaultToolchain.cmake
            HINTS ${CMAKE_SOURCE_DIR}/cmake)
  if(default_toolchain)
    include(${default_toolchain})
  else()
    message(FATAL_ERROR "Cannot find GaudiDefaultToolchain.cmake")
  endif()

endif()
