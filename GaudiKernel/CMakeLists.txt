###############################################
# CMakeLists.txt file for building GaudiKernel
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(ROOT)
GAUDI_USE_PACKAGE(AIDA)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ) 
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS} ${ROOT_LIBRARY_DIRS} )

#---Linker Library---------------------------------------------------------------------------
GAUDI_LINKER_LIBRARY( GaudiKernel 
                      Lib/*.cpp 
                      ${ROOT_LIBRARIES} ${Boost_LIBRARIES})

#---Dictionary-------------------------------------------------------------------------------
REFLEX_BUILD_DICTIONARY( GaudiKernel 
                         dict/dictionary.h 
                         dict/dictionary.xml
                         GaudiKernel )

#---genconf command--------------------------------------------------------------------------
GAUDI_EXECUTABLE( genconf.exe Util/genconf.cpp GaudiKernel)

#---Set variables for clients----------------------------------------------------------------
SET( GAUDI_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} 
                        ${Boost_INCLUDE_DIRS} ${AIDA_INCLUDE_DIRS} PARENT_SCOPE )
SET( GAUDI_LIBRARY_DIRS ${ROOT_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} PARENT_SCOPE )

#---Installation-----------------------------------------------------------------------------
GAUDI_INSTALL_HEADERS()
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()



