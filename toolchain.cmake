# Special wrapper to load the declared version of the heptools toolchain.
set(heptools_version 66)

# this check is needed because the toolchain is called when checking the
# compiler (without the proper cache)
if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeTmp")
  set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_PREFIX_PATH})
  find_file(use_heptools_module UseHEPTools.cmake)
  if(use_heptools_module)
    include(${use_heptools_module})
    use_heptools(${heptools_version})
  else()
    message(FATAL_ERROR "Cannot find UseHEPTools.cmake")
  endif()
endif()
