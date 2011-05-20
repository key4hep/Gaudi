CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

# Defined the wished LCG configuration version (must be present only in Gaudi)
set(LCG_version 60b)

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

# Declare project name and version
GAUDI_PROJECT(Gaudi v22r2)

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    get_filename_component(LCG_TOOLCHAIN_PATH ${CMAKE_TOOLCHAIN_FILE} PATH)
    message(${LCG_TOOLCHAIN_PATH})
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${LCG_TOOLCHAIN_PATH})

    # Define the search paths
    include(Configuration)

    set(CMAKE_PREFIX_PATH ${CMAKE_LIBRARY_PATH} ${LCG_external}/CppUnit/${CppUnit_config_version}/${LCG_system})
endif()

#---Tests environment-------------------------------------------
if( BUILD_TESTS )
  GAUDI_USE_PACKAGE(QMtest)
  GAUDI_USE_PACKAGE(pytools)
  GAUDI_USE_PACKAGE(RELAX)
endif()
