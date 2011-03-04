CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

#---Do not know how to do this yet------------------------------
INCLUDE($ENV{CMAKEGAUDIPROJECT})
#---------------------------------------------------------------

GAUDI_PROJECT(GAUDI v22r1)

#---Common includes---------------------------------------------
INCLUDE(GaudiPolicy)

GAUDI_GET_PACKAGES(packages)
GAUDI_SORT_PACKAGES( packages ${packages})

foreach( package ${packages})
  add_subdirectory(${package})
endforeach()

#---Tests environment-------------------------------------------
if( BUILD_TESTS )
  GAUDI_USE_PACKAGE(QMtest)
  GAUDI_USE_PACKAGE(pytools)
  GAUDI_USE_PACKAGE(RELAX)
endif()

#---Global actions for the project------------------------------
GAUDI_PROJECT_VERSION_HEADER()
GAUDI_BUILD_PROJECT_SETUP()


include(CPack)
set(CPACK_GENERATOR RPM)


