CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

# Defined the wished LCG configuration version (must be present only in Gaudi)
set(LCG_version 60b)

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

# Declare project name and version
GAUDI_PROJECT(Gaudi v22r2)

#---Tests environment-------------------------------------------
if( BUILD_TESTS )
  GAUDI_USE_PACKAGE(QMtest)
  GAUDI_USE_PACKAGE(pytools)
  GAUDI_USE_PACKAGE(RELAX)
endif()
