###############################################
# CMakeLists.txt file for building GaudiUtils
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(uuid)
GAUDI_USE_PACKAGE(XercesC)

#---Linker Library
GAUDI_LINKER_LIBRARY( GaudiUtilsLib 
                      Lib/*.cpp 
                      GaudiKernel Hist)

#---Component Library
GAUDI_COMPONENT_LIBRARY( GaudiUtils 
                         component/*.cpp 
                         GaudiUtilsLib ${XercesC_LIBRARIES} ${uuid_LIBRARIES})


#---Installation
GAUDI_INSTALL_HEADERS(GaudiUtils)


