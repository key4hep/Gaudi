############################################################################
# CMakeLists.txt file for building GaudiGSL
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
GAUDI_USE_PACKAGE(GaudiKernel)
GAUDI_USE_PACKAGE(GaudiAlg)
GAUDI_USE_PACKAGE(GSL)
GAUDI_USE_PACKAGE(CLHEP)
GAUDI_USE_PACKAGE(AIDA)


if( NOT APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiGSL Components/*.cpp GaudiGSLLib)
GAUDI_LINKER_LIBRARY(GaudiGSLLib Lib/*.cpp GaudiAlgLib ${GSL_LIBRARIES} ${CLHEP_LIBRARIES})

#---Dictionaries------------------------------------------------------------
REFLEX_GENERATE_DICTIONARY( GaudiGSLMath dict/GaudiGSLMath.h dict/GaudiGSLMath.xml GaudiGSLLib )

#---Executables-------------------------------------------------------------
GAUDI_TEST(IntegralInTest Tests/IntegralInTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(DerivativeTest Tests/DerivativeTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(2DoubleFuncTest Tests/2DoubleFuncTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(GSLAdaptersTest Tests/GSLAdaptersTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(PFuncTest Tests/PFuncTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(ExceptionsTest Tests/ExceptionsTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(SimpleFuncTest Tests/SimpleFuncTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(3DoubleFuncTest Tests/3DoubleFuncTest.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(InterpTest Tests/Interp.cpp GaudiGSLLib GaudiUtilsLib)
GAUDI_TEST(Integral1Test Tests/Integral1Test.cpp GaudiGSLLib GaudiUtilsLib)

#---Installation------------------------------------------------------------
GAUDI_INSTALL_HEADERS(GaudiGSL GaudiMath)



