###############################################
# CMakeLists.txt file for building GaudiUtils
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(uuid)
GAUDI_USE_PACKAGE(XercesC)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS} ${XercesC_INCLUDE_DIRS} ) 
LINK_DIRECTORIES(${GAUDI_LIBRARY_DIRS} ${XercesC_LIBRARY_DIRS} )

#---Linker Library
GAUDI_LINKER_LIBRARY( GaudiUtilsLib 
                      Lib/*.cpp 
                      GaudiKernel Hist)

#---Component Library
GAUDI_COMPONENT_LIBRARY( GaudiUtils 
                         component/*.cpp 
                         GaudiUtilsLib ${XercesC_LIBRARIES} ${uuid_LIBRARIES})


#---Set variables for clients!!
SET( GAUDI_INCLUDE_DIRS  ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS}  PARENT_SCOPE )
#SET( GAUDI_LIBRARY_DIRS ${ROOT_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} PARENT_SCOPE )

#---Installation
GAUDI_INSTALL_HEADERS(GaudiUtils)


