CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)

#---Do not know how to do this yet----------------------------
INCLUDE(/build/mato/GAUDI/GAUDI_cmake/cmake/GaudiProject.cmake)
#-------------------------------------------------------------

GAUDI_PROJECT(GAUDI v21r10p1)

#
# Common includes
#
INCLUDE(GaudiPolicy)

GAUDI_GET_PACKAGES(packages)
GAUDI_SORT_PACKAGES( packages ${packages})

foreach( package ${packages})
  add_subdirectory(${package})
endforeach()

INSTALL(DIRECTORY cmake/ DESTINATION cmake)

# Global actions for the project
GAUDI_USE_PACKAGE(QMtest)
GAUDI_USE_PACKAGE(pytools)
GAUDI_USE_PACKAGE(RELAX)
SET( QMtest_environment ${QMtest_environment} QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes )
GAUDI_PROJECT_VERSION_HEADER()
GAUDI_BUILD_PROJECT_SETUP()

