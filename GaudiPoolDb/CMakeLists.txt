###############################################
# CMakeLists.txt file for building GaudiPoolDb
# @author Pere Mato, CERN
###############################################

GAUDI_USE_PACKAGE(POOL)
GAUDI_USE_PACKAGE(CORAL)
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiUtils)


#---Dictionary Generation -------------------------------------------------------------------
REFLEX_GENERATE_DICTIONARY( GaudiPoolDb src/PoolDb_dict.h  src/PoolDb_dict.xml)
#---Linker/Component Library-----------------------------------------------------------------
GAUDI_LINKER_LIBRARY( GaudiPoolDb 
                      "*.cpp;${CMAKE_CURRENT_BINARY_DIR}/GaudiPoolDb_dict.cpp"
                      GaudiKernel ${POOL_LIBRARIES} ${CORAL_LIBRARIES} Cintex)
IF (APPLE)
  SET_TARGET_PROPERTIES(GaudiPoolDb PROPERTIES SUFFIX .so)
ENDIF()
GAUDI_GENERATE_ROOTMAP(GaudiPoolDb)
GAUDI_GENERATE_CONFIGURATION(GaudiPoolDb)

#---Installation
GAUDI_INSTALL_HEADERS(GaudiPoolDb)


