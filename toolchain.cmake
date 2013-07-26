# Special wrapper to load the declared version of the heptools toolchain.
set(heptools_version 65a)

find_file(use_heptools_module UseHEPTools.cmake
          HINTS ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# this check is needed because the toolchain seem to be called a second time
# without the proper cache
if(use_heptools_module)
  include(${use_heptools_module})
  use_heptools(${heptools_version})
endif()
