CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

#---Do not know how to do this yet------------------------------
find_package(GaudiProject)
#---------------------------------------------------------------

GAUDI_PROJECT(Gaudi v22r1)

#---Common includes---------------------------------------------
# TODO: these calls are valid for every Gaudi-based project, so they be part
# of the GAUDI_PROJECT macro.
INCLUDE(GaudiPolicy)

GAUDI_GET_PACKAGES(packages)
GAUDI_SORT_PACKAGES(packages ${packages})

foreach(package ${packages})
  add_subdirectory(${package})
endforeach()

#---Global actions for the project------------------------------
GAUDI_PROJECT_VERSION_HEADER()
GAUDI_BUILD_PROJECT_SETUP()
GAUDI_MERGE_CONF_USER_DB()

#---Tests environment-------------------------------------------
if( BUILD_TESTS )
  GAUDI_USE_PACKAGE(QMtest)
  GAUDI_USE_PACKAGE(pytools)
  GAUDI_USE_PACKAGE(RELAX)
endif()


include(CPack)
set(CPACK_GENERATOR RPM)


