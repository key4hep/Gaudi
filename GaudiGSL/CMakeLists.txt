############################################################################
# CMakeLists.txt file for building GaudiGSL
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiAlg)

find_package(CLHEP)
find_package(GSL)

if(NOT APPLE AND NOT WIN32)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiGSLLib Lib/*.cpp
                     LINK_LIBRARIES GaudiAlgLib ${GSL_LIBRARIES} ${CLHEP_LIBRARIES}
                     INCLUDE_DIRS GSL CLHEP)
gaudi_add_module(GaudiGSL Components/*.cpp LINK_LIBRARIES GaudiGSLLib)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiGSLMath dict/GaudiGSLMath.h dict/GaudiGSLMath.xml LINK_LIBRARIES GaudiGSLLib)

#---Executables-------------------------------------------------------------
GAUDI_UNIT_TEST(IntegralInTest Tests/IntegralInTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(DerivativeTest Tests/DerivativeTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(2DoubleFuncTest Tests/2DoubleFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(GSLAdaptersTest Tests/GSLAdaptersTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(PFuncTest Tests/PFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(ExceptionsTest Tests/ExceptionsTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(SimpleFuncTest Tests/SimpleFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(3DoubleFuncTest Tests/3DoubleFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(InterpTest Tests/Interp.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(Integral1Test Tests/Integral1Test.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiGSL GaudiMath)

