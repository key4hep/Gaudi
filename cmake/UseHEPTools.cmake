
include(${CMAKE_CURRENT_LIST_DIR}/HEPToolsMacros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/BinaryTagUtils.cmake)

# use_heptools(heptools_version)
#
# Look for the required version of the HEPTools toolchain and replace the
# current toolchain file with it.
#
# WARNING: this macro must be called from a toolchain file
#
macro(use_heptools heptools_version)
  # get_host_binary_tag tries to find PythonInterp if PYTHON_EXECUTABLE
  # is not set, but here it's too early because we didn't set CMAKE_PREFIX_PATH yet
  set(PYTHON_EXECUTABLE /usr/bin/env python)
  get_host_binary_tag(LCG_system)
  set(PYTHON_EXECUTABLE)
  parse_binary_tag()
  compatible_binary_tags(_compatible_tags)

  set(_info_names)
  foreach(_tag ${_compatible_tags})
    list(APPEND _info_names LCG_externals_${_tag}.txt)
  endforeach()
  if(LCG_SYSTEM)
    list(APPEND _info_names LCG_externals_${LCG_SYSTEM}-${BINARY_TAG_TYPE}.txt
                            LCG_externals_${LCG_SYSTEM}-opt.txt)
  endif()

  # Find the toolchain description
  find_file(LCG_TOOLCHAIN_INFO
            NAMES ${_info_names}
            HINTS ENV CMTPROJECTPATH
            PATH_SUFFIXES LCG_${heptools_version})

  if(LCG_TOOLCHAIN_INFO)
    message(STATUS "Using heptools ${heptools_version} from ${LCG_TOOLCHAIN_INFO}")

    get_filename_component(LCG_releases ${LCG_TOOLCHAIN_INFO} PATH CACHE)
    set(LCG_external ${LCG_releases})
    if(LCG_releases MATCHES "LCG_${heptools_version}\$")
      get_filename_component(LCG_releases_base ${LCG_releases} PATH)
    else()
      set(LCG_releases_base ${LCG_releases})
    endif()
    set(ENV{LCG_releases_base} ${LCG_releases_base})

    file(STRINGS ${LCG_TOOLCHAIN_INFO} _lcg_infos)

    if(LCG_FORCE_COMPILER)
      string(REPLACE "-" ";" _l "${LCG_FORCE_COMPILER}")
      lcg_set_compiler(${_l})
    endif()

    foreach(_l ${_lcg_infos})
      if(_l MATCHES "^COMPILER:")
        string(REPLACE "COMPILER:" "" _l "${_l}")
        string(STRIP _l "${_l}")
        string(REGEX REPLACE " +" ";" _l "${_l}")
        if (NOT LCG_FORCE_COMPILER)
          lcg_set_compiler(${_l})
        endif()
        # We add the compiler to the path
        lcg_set_lcg_system_compiler_path(${_l})
      else()
         if(NOT _l MATCHES "^((PLATFORM|VERSION|COMPILER):|#)")
           string(REGEX REPLACE "; *" ";" _l "${_l}")
           lcg_set_external(${_l})
         endif()
      endif()
    endforeach()


    # Enable the right compiler (needs LCG_external)
    lcg_common_compilers_definitions()

    lcg_prepare_paths()

    lcg_find_common_tools()

    # Reset the cache variable to have proper documentation.
    #set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST}
    #    CACHE FILEPATH "The CMake toolchain file" FORCE)

  else()
    message(FATAL_ERROR "Cannot find heptools ${heptools_version}.")
  endif()

endmacro()
