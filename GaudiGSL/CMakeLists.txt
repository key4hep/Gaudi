############################################################################
# CMakeLists.txt file for building GaudiGSL
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
#G AUDI_USE_PACKAGE(GaudiKernel)
#G AUDI_USE_PACKAGE(GaudiAlg)
#G AUDI_USE_PACKAGE(GSL)
#G AUDI_USE_PACKAGE(CLHEP)
#G AUDI_USE_PACKAGE(AIDA)

find_package(AIDA)
include_directories(${AIDA_INCLUDE_DIRS})

find_package(ROOT)
include_directories(${ROOT_INCLUDE_DIRS})
link_directories(${ROOT_LIBRARY_DIRS})

find_package(CLHEP)
include_directories(${CLHEP_INCLUDE_DIRS})

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

find_package(GSL)
include_directories(${GSL_INCLUDE_DIRS})

if(NOT APPLE AND NOT WIN32)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiGSL Components/*.cpp LIBRARIES GaudiGSLLib)
GAUDI_LINKER_LIBRARY(GaudiGSLLib Lib/*.cpp LIBRARIES GaudiAlgLib ${GSL_LIBRARIES} ${CLHEP_LIBRARIES})

#---Dictionaries------------------------------------------------------------
REFLEX_GENERATE_DICTIONARY( GaudiGSLMath dict/GaudiGSLMath.h dict/GaudiGSLMath.xml LIBRARIES GaudiGSLLib )

#---Executables-------------------------------------------------------------
GAUDI_UNIT_TEST(IntegralInTest Tests/IntegralInTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(DerivativeTest Tests/DerivativeTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(2DoubleFuncTest Tests/2DoubleFuncTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(GSLAdaptersTest Tests/GSLAdaptersTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(PFuncTest Tests/PFuncTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(ExceptionsTest Tests/ExceptionsTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(SimpleFuncTest Tests/SimpleFuncTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(3DoubleFuncTest Tests/3DoubleFuncTest.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(InterpTest Tests/Interp.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)
GAUDI_UNIT_TEST(Integral1Test Tests/Integral1Test.cpp LIBRARIES GaudiGSLLib GaudiUtilsLib)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiGSL GaudiMath)

