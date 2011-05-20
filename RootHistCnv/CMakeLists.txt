############################################################################
# CMakeLists.txt file for building RootHistCnv
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(AIDA)
#G AUDI_USE_PACKAGE(ROOT)

find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(RootHistCnv *.cpp LIBRARIES GaudiKernel ${ROOT_LIBRARIES}
                        ${ROOT_RIO_LIBRARY} ${ROOT_Hist_LIBRARY} ${ROOT_Matrix_LIBRARY} ${ROOT_Tree_LIBRARY})


