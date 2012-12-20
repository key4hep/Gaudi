gaudi_subdir(GaudiGSL v7r12)

gaudi_depends_on_subdirs(GaudiAlg)

find_package(CLHEP)
find_package(GSL)

if(NOT APPLE AND NOT WIN32)
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiGSLLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiAlgLib GSL CLHEP
                  INCLUDE_DIRS GSL CLHEP
                  PUBLIC_HEADERS GaudiGSL GaudiMath)
gaudi_add_module(GaudiGSL src/Components/*.cpp
                 LINK_LIBRARIES GaudiGSLLib)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiGSLMath dict/GaudiGSLMath.h dict/GaudiGSLMath.xml LINK_LIBRARIES GaudiGSLLib)

#---Executables-------------------------------------------------------------
macro(add_gsl_unit_test name)
  gaudi_add_unit_test(${name} src/Tests/${name}.cpp LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib)
endmacro()

foreach(test IntegralInTest DerivativeTest 2DoubleFuncTest GSLAdaptersTest
             PFuncTest ExceptionsTest SimpleFuncTest 3DoubleFuncTest InterpTest
             Integral1Test)
  add_gsl_unit_test(${test})
endforeach()
