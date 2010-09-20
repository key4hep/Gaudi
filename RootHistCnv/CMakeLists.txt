############################################################################
# CMakeLists.txt file for building RootHistCnv
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(AIDA)
GAUDI_USE_PACKAGE(ROOT)

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(RootHistCnv *.cpp LIBRARIES GaudiKernel ${ROOT_LIBRARIES} RIO Hist Matrix Tree)


