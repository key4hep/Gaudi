############################################################################
# CMakeLists.txt file for building GaudiAlg
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiUtils)
GAUDI_USE_PACKAGE(ROOT)
GAUDI_USE_PACKAGE(AIDA)
GAUDI_USE_PACKAGE(Boost)

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiAlgLib lib/*.cpp LIBRARIES GaudiUtilsLib)
GAUDI_COMPONENT_LIBRARY(GaudiAlg components/*.cpp LIBRARIES GaudiAlgLib GaudiKernel ${ROOT_LIBRARIES} ${ROOT_RIO_LIBRARY})

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiAlg)

#---This is needed by SET_RUNTIME_PATH() :-(-----------------------------------
SET( GAUDI_LIBRARY_DIRS ${GAUDI_LIBRARY_DIRS} ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE )



