###############################################
# CMakeLists.txt file for building GaudiPoolDb
# @author Pere Mato, CERN
###############################################

#G AUDI_USE_PACKAGE(ROOT)
#G AUDI_USE_PACKAGE(POOL)
#G AUDI_USE_PACKAGE(CORAL)
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(GaudiUtils)


find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

find_package(POOL)
include_directories(${POOL_INCLUDE_DIRS})

find_package(CORAL)
include_directories(${CORAL_INCLUDE_DIRS})


#---Dictionary Generation -------------------------------------------------------------------
REFLEX_GENERATE_DICTIONARY( GaudiPoolDb src/PoolDb_dict.h  src/PoolDb_dict.xml)
#---Linker/Component Library-----------------------------------------------------------------
GAUDI_LINKER_LIBRARY( GaudiPoolDb *.cpp ${CMAKE_CURRENT_BINARY_DIR}/GaudiPoolDb_dict.cpp
                      LIBRARIES GaudiKernel ${POOL_LIBRARIES} ${CORAL_LIBRARIES} ${ROOT_LIBRARIES}
					  ${ROOT_Cintex_LIBRARY} ${ROOT_Tree_LIBRARY} ${ROOT_RIO_LIBRARY})
if(APPLE)
  SET_TARGET_PROPERTIES(GaudiPoolDb PROPERTIES SUFFIX .so)
endif()
GAUDI_GENERATE_ROOTMAP(GaudiPoolDb)
GAUDI_GENERATE_CONFIGURABLES(GaudiPoolDb)

set(GaudiPoolDb_environment JOBOPTSEARCHPATH+=${CMAKE_SOURCE_DIR}/GaudiPoolDb/options
                            PARENT_SCOPE)

#---Installation-----------------------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiPoolDb)


