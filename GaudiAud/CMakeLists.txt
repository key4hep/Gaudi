############################################################################
# CMakeLists.txt file for building GaudiAud
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiAud *.cpp LINK_LIBRARIES GaudiKernel)
