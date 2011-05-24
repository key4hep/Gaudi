############################################################################
# CMakeLists.txt file for building GaudiKernel
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
find_package(Boost COMPONENTS program_options)
find_package(ROOT)

#---Util Executables--------------------------------------------------------
#set(CMAKE_BUILD_TYPE Release)
GAUDI_EXECUTABLE(genconf genconf.cpp
                 LIBRARIES GaudiKernel ${Boost_LIBRARIES} ${ROOT_LIBRARIES}
                 USE_HEADERS Boost ROOT)
if(WIN32)
  GAUDI_EXECUTABLE(genwindef genwindef.cpp LibSymbolInfo.cpp)
endif()
