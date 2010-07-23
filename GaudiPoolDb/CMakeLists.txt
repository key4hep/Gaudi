###############################################
# CMakeLists.txt file for building GaudiPoolDb
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(POOL)
GAUDI_USE_PACKAGE(CORAL)
GAUDI_USE_PACKAGE(ROOT)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS} ${POOL_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} ) 
LINK_DIRECTORIES(${GAUDI_LIBRARY_DIRS} ${POOL_LIBRARY_DIRS} ${CORAL_LIBRARY_DIRS} )

#---Dictionary Generation -------------------------------------------------------------------
REFLEX_GENERATE_DICTIONARY( GaudiPoolDb src/PoolDb_dict.h  src/PoolDb_dict.xml)
#---Linker/Component Library-----------------------------------------------------------------
GAUDI_LINKER_LIBRARY( GaudiPoolDb 
                      "*.cpp;${CMAKE_CURRENT_BINARY_DIR}/GaudiPoolDb_dict.cpp"
                      GaudiKernel ${POOL_LIBRARIES} ${CORAL_LIBRARIES} Cintex)
GAUDI_GENERATE_ROOTMAP(GaudiPoolDb)
GAUDI_GENERATE_CONFIGURATION(GaudiPoolDb)
GAUDI_INSTALL_CONFIGURATION()

#---Set variables for clients!!
#SET( GAUDI_INCLUDE_DIRS  ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS}  PARENT_SCOPE )
#SET( GAUDI_LIBRARY_DIRS ${ROOT_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} PARENT_SCOPE )

#---Installation
GAUDI_INSTALL_HEADERS(GaudiPoolDb)


