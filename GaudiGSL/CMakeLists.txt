############################################################################
# CMakeLists.txt file for building GaudiGSL
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
depends_on_subdirs(GaudiAlg)

find_package(CLHEP)
find_package(GSL)

if(NOT APPLE AND NOT WIN32)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiGSLLib Lib/*.cpp
                  LINK_LIBRARIES GaudiAlgLib GSL CLHEP
                  INCLUDE_DIRS GSL CLHEP
                  PUBLIC_HEADERS GaudiGSL GaudiMath)
gaudi_add_module(GaudiGSL Components/*.cpp
                 LINK_LIBRARIES GaudiGSLLib)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiGSLMath dict/GaudiGSLMath.h dict/GaudiGSLMath.xml LINK_LIBRARIES GaudiGSLLib)

#---Executables-------------------------------------------------------------
gaudi_add_unit_test(IntegralInTest Tests/IntegralInTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(DerivativeTest Tests/DerivativeTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(2DoubleFuncTest Tests/2DoubleFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(GSLAdaptersTest Tests/GSLAdaptersTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(PFuncTest Tests/PFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(ExceptionsTest Tests/ExceptionsTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(SimpleFuncTest Tests/SimpleFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(3DoubleFuncTest Tests/3DoubleFuncTest.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(InterpTest Tests/Interp.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
gaudi_add_unit_test(Integral1Test Tests/Integral1Test.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
