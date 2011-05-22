############################################################################
# CMakeLists.txt file for building GaudiKernel
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
find_package(Boost REQUIRED COMPONENTS filesystem thread system)
include_directories(${Boost_INCLUDE_DIRS})

find_package(CppUnit)
include_directories(${CppUnit_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
GAUDI_LINKER_LIBRARY(GaudiKernel Lib/*.cpp LIBRARIES ${CMAKE_DL_LIBS} ${ROOT_LIBRARIES} ${Boost_LIBRARIES})

#---Tests-------------------------------------------------------------------
GAUDI_UNIT_TEST(DirSearchPath_test ../tests/src/DirSearchPath_test.cpp LIBRARIES GaudiKernel)
GAUDI_UNIT_TEST(test_SerializeSTL ../tests/src/test_SerializeSTL.cpp LIBRARIES GaudiKernel ${CppUnit_LIBRARIES})
GAUDI_UNIT_TEST(PathResolver_test ../tests/src/PathResolver_test.cpp LIBRARIES GaudiKernel)
GAUDI_UNIT_TEST(test_GaudiTime ../tests/src/test_GaudiTime.cpp LIBRARIES GaudiKernel ${CppUnit_LIBRARIES})
# This is just a compile-time tests.
GAUDI_UNIT_TEST(test_headers_build ../tests/src/test_headers_build.cpp LIBRARIES GaudiKernel)

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(GaudiKernel dict/dictionary.h  dict/dictionary.xml LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
# Disable generation of ConfUserDB (must be done before GAUDI_INSTALL_PYTHON_MODULES)
set_directory_properties(PROPERTIES CONFIGURABLE_USER_MODULES None)

GAUDI_INSTALL_HEADERS(GaudiKernel)
GAUDI_INSTALL_PYTHON_MODULES()
GAUDI_INSTALL_SCRIPTS()

#---Test-----------------------------------------------------------------------
foreach(test dirsearchpath pathresolver serializestl gaudi_time)
  GAUDI_QMTEST_TEST(gaudikernel.${test})
endforeach()

#---Special options
if(HIDE_WARNINGS)
  if(UNIX)
    # This hides warnings from a broken header file in CppUnit, but may
    # hide problems in the actual test source
    set_property(SOURCE tests/src/test_SerializeSTL.cpp tests/src/test_GaudiTime.cpp
                 PROPERTY COMPILE_FLAGS -Wno-unused-parameter)
    set_property(TARGET GaudiKernelDict PROPERTY COMPILE_FLAGS -Wno-overloaded-virtual)
  endif()
  if(WIN32)
    set_property(TARGET GaudiKernelDict PROPERTY COMPILE_DEFINITIONS _SCL_SECURE_NO_WARNINGS)
  endif()
endif()
