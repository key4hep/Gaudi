CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

# Declare project name and version
GAUDI_PROJECT(Gaudi v22r1)

#---Tests environment-------------------------------------------
if( BUILD_TESTS )
  GAUDI_USE_PACKAGE(QMtest)
  GAUDI_USE_PACKAGE(pytools)
  GAUDI_USE_PACKAGE(RELAX)
endif()
