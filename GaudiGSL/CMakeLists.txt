###############################################
# CMakeLists.txt file for building GaudiGSL
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(GSL)
GAUDI_USE_PACKAGE(CLHEP)
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiAlg)

#INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${GAUDI_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${GSL_INCLUDE_DIRS}) 
#LINK_DIRECTORIES(${GAUDI_LIBRARY_DIRS} ${CLHEP_LIBRARY_DIRS} ${GSL_LIBRARY_DIRS})

IF( NOT APPLE)
   SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
ENDIF()

#---Linker Library
GAUDI_LINKER_LIBRARY( GaudiGSLLib 
                      Lib/*.cpp 
                      GaudiKernel ${CLHEP_LIBRARIES} ${GSL_LIBRARIES})

#---Component Library
GAUDI_COMPONENT_LIBRARY( GaudiGSL 
                         Components/*.cpp 
                         GaudiGSLLib GaudiAlgLib)

#---Dictionary
REFLEX_GENERATE_DICTIONARY( GaudiGSLMath 
                            dict/GaudiGSLMath.h 
                            dict/GaudiGSLMath.xml
                            GaudiGSLLib )

#---Installation
GAUDI_INSTALL_HEADERS(GaudiGSL GaudiMath)



