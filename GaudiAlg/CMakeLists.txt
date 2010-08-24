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
GAUDI_LINKER_LIBRARY(GaudiAlgLib lib/*.cpp GaudiUtilsLib)
GAUDI_COMPONENT_LIBRARY(GaudiAlg components/*.cpp GaudiAlgLib GaudiKernel ${ROOT_LIBRARIES} RIO)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiAlg)


