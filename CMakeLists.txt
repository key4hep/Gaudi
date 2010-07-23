#
# Define project and CMake requirements
#
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.6)
PROJECT(GAUDI)

# Define the version of the project - can be used to generate sources,
SET(GAUDI_VERSION "v21r7")
SET(GAUDI_VERSION_MAJOR "21")
SET(GAUDI_VERSION_MINOR "7")
SET(GAUDI_VERSION_PATCH "0")

#
# Project Options
#
OPTION(BUILD_SHARED_LIBS "Set to OFF to build static libraries" ON)

#
# Set up path to internal set of CMake modules.
#
set(CMAKE_MODULE_PATH
    ${GAUDI_SOURCE_DIR}/cmake
    ${CMAKE_MODULE_PATH})

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/InstallArea )
#
# Common includes
#
INCLUDE(GaudiPolicy)

# Add the Project Packages to process the CMakeLists.txt there
#
ADD_SUBDIRECTORY(GaudiKernel)
ADD_SUBDIRECTORY(GaudiSvc)
ADD_SUBDIRECTORY(GaudiAud)
ADD_SUBDIRECTORY(GaudiUtils)
ADD_SUBDIRECTORY(GaudiAlg)
ADD_SUBDIRECTORY(GaudiGSL)
ADD_SUBDIRECTORY(GaudiMonitor)
ADD_SUBDIRECTORY(GaudiPoolDb)
ADD_SUBDIRECTORY(PartPropSvc)
ADD_SUBDIRECTORY(RootHistCnv)
ADD_SUBDIRECTORY(GaudiPython)
ADD_SUBDIRECTORY(GaudiExamples)
ADD_SUBDIRECTORY(Gaudi)

# Global actions for the project
GAUDI_USE_PACKAGE(QMtest)
GAUDI_USE_PACKAGE(pytools)
GAUDI_USE_PACKAGE(RELAX)
SET( QMtest_environment ${QMtest_environment} QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes )
GAUDI_PROJECT_VERSION_HEADER()
GAUDI_BUILD_PROJECT_SETUP()

