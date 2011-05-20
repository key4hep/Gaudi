############################################################################
# CMakeLists.txt file for building GaudiKernel
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
find_package(Boost COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

# FIXME: for some reason (probably related to being its subdirectory) we do not get
# the include directories correct.
include_directories(${CMAKE_SOURCE_DIR}/GaudiKernel)

#---Util Executables--------------------------------------------------------
#set(CMAKE_BUILD_TYPE Release)
GAUDI_EXECUTABLE(genconf genconf.cpp LIBRARIES GaudiKernel ${Boost_LIBRARIES} ${ROOT_LIBRARIES})
if(WIN32)
  GAUDI_EXECUTABLE(genwindef genwindef.cpp LibSymbolInfo.cpp)
endif()
