###############################################
# CMakeLists.txt file for building GaudiAlg
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiUtils)

#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS} ) 
#LINK_DIRECTORIES(${GAUDI_LIBRARY_DIRS} )

#---Linker Library
GAUDI_LINKER_LIBRARY( GaudiAlgLib 
                      lib/*.cpp 
                      GaudiKernel GaudiUtilsLib )

#---Component Library
GAUDI_COMPONENT_LIBRARY( GaudiAlg 
                         components/*.cpp 
                         GaudiAlgLib )


#---Set variables for clients!!
#SET( GAUDI_INCLUDE_DIRS  ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS}  PARENT_SCOPE )
#SET( GAUDI_LIBRARY_DIRS ${ROOT_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} PARENT_SCOPE )

#---Installation
GAUDI_INSTALL_HEADERS(GaudiAlg)


