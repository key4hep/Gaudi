############################################################################
# CMakeLists.txt file for building GaudiKernel
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(Boost)
GAUDI_USE_PACKAGE(GaudiKernel)


#---Util Executables--------------------------------------------------------

set(CMAKE_BUILD_TYPE Release)
GAUDI_EXECUTABLE(genconf genconf.cpp LIBRARIES GaudiKernel)
if(WIN32)
  GAUDI_EXECUTABLE(genwindef genwindef.cpp LibSymbolInfo.cpp)
endif()
