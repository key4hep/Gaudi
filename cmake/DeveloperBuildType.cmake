#[========================================================================[.rst:
Build type: Developer
---------------------

When included, this file adds a build type called "Developer" to the
CMake configuration.

This build type is a debug and warnings build type for C++.

#]========================================================================]

include_guard(GLOBAL) # can only be included once per configuration

set(_config Developer)
string(TOUPPER ${_config} _CONFIG)

# Set compile flags
set(_flags "-g -Wall -Wextra -pedantic -Wnon-virtual-dtor -Werror=return-type -Wwrite-strings -Wpointer-arith -Woverloaded-virtual")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # only for gcc because this flag is not supported by clang yet
    set(_flags "${_flags} -Wsuggest-override")
endif()
# WARNING: The default value of a cached variable cannot be modified after it has been set
set(CMAKE_CXX_FLAGS_${_CONFIG} "${_flags}"
    CACHE INTERNAL "Compile flags for ${_config} build type")

# Set linker flags
foreach(bin_type IN ITEMS EXE STATIC SHARED MODULE)
    set(CMAKE_${bin_type}_LINKER_FLAGS_${_CONFIG} "-Wl,--no-undefined -Wl,-z,max-page-size=0x1000"
        CACHE INTERNAL "Linker flags for ${bin_type} with ${_config} build type")
endforeach()

# Add the build types to the possible ones
if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND CMAKE_CONFIGURATION_TYPES ${_config})
  set(CMAKE_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} PARENT_SCOPE) # if build full stack
endif()
