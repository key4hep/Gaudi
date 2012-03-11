############################################################################
# CMakeLists.txt file for building RootHistCnv
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

find_package(AIDA)
find_package(ROOT)
find_package(Boost)

#---Libraries---------------------------------------------------------------
gaudi_add_module(RootHistCnv *.cpp
                        LINK_LIBRARIES GaudiKernel ${ROOT_LIBRARIES} ${ROOT_RIO_LIBRARY}
                                  ${ROOT_Hist_LIBRARY} ${ROOT_Matrix_LIBRARY} ${ROOT_Tree_LIBRARY}
                        INCLUDE_DIRS Boost ROOT AIDA)
