###############################################
# CMakeLists.txt file for building GaudiAlg
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiUtils)

#---Linker Library
GAUDI_LINKER_LIBRARY( GaudiAlgLib 
                      lib/*.cpp 
                      GaudiKernel GaudiUtilsLib )

#---Component Library
GAUDI_COMPONENT_LIBRARY( GaudiAlg 
                         components/*.cpp 
                         GaudiAlgLib )

#---Installation
GAUDI_INSTALL_HEADERS(GaudiAlg)


