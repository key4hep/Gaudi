############################################################################
# CMakeLists.txt file for building GaudiExamples
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel GaudiUtils GaudiGSL GaudiAlg)

find_package(HepPDT)
find_package(ROOT COMPONENTS Tree RIO Hist)
find_package(Boost COMPONENTS python)
find_package(CLHEP)
find_package(GSL)
find_package(PythonLibs)
find_package(RELAX)

#---Libraries---------------------------------------------------------------
set( GaudiExamples_srcs AbortEvent/*.cpp
AlgSequencer/*.cpp
AlgTools/*.cpp
DataOnDemand/*.cpp
AlgErrAud/*.cpp
GSLTools/*.cpp
RandomNumber/*.cpp
Histograms/*.cpp
NTuples/*.cpp
TupleEx/*.cpp
CounterEx/*.cpp
PartProp/*.cpp
Properties/*.cpp
ExtendedProperties/*.cpp
POOLIO/*.cpp
ColorMsg/ColorMsgAlg.cpp
History/History.cpp
THist/*.cpp
ErrorLog/ErrorLogTest.cpp
EvtColsEx/EvtColAlg.cpp
Maps/*.cpp
MultipleLogStreams/*.cpp
GaudiCommonTests/*.cpp
IncidentSvc/*.cpp
bug_34121/*.cpp
Auditors/*.cpp
Timing/*.cpp
Selections/*.cpp
SvcInitLoop/*.cpp
StringKeys/*.cpp
StatusCodeSvc/*.cpp
testing/*.cpp )

gaudi_add_library(GaudiExamplesLib Lib/*.cpp
                  LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib HepPDT ROOT
                  INCLUDE_DIRS HepPDT ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Lib
                  NO_PUBLIC_HEADERS)
gaudi_add_module(GaudiExamples ${GaudiExamples_srcs} LINK_LIBRARIES GaudiExamplesLib)
gaudi_add_python_module(PyExample PythonModule/*.cpp
                        LINK_LIBRARIES ${Boost_PYTHON_LIBRARY}
                        INCLUDE_DIRS Boost PythonLibs)

GAUDI_INSTALL_PYTHON_MODULES()

#---Executables-------------------------------------------------------------
gaudi_add_executable(Allocator Allocator/*.cpp
                     LINK_LIBRARIES GaudiExamplesLib GaudiAlgLib GaudiKernel ROOT CLHEP GSL
                     INCLUDE_DIRS CLHEP GSL)


#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiExamples src/POOLIO/dict.h  src/POOLIO/dict.xml LINK_LIBRARIES GaudiExamplesLib)


#---Tests-------------------------------------------------------------------
gaudi_add_test(Properties FRAMEWORK options/Properties.opts ENVIRONMENT JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/options)
gaudi_add_test(AlgTools FRAMEWORK options/AlgTools.opts ENVIRONMENT JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/options)

gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
                 STDOPTS=${CMAKE_CURRENT_SOURCE_DIR}/options
                 JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/options
                 GAUDIEXAMPLESROOT=${CMAKE_CURRENT_SOURCE_DIR}
                 PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest)

SET(GAUDIEXAMPLES_ENVIRONMENT STDOPTS=${CMAKE_SOURCE_DIR}/GaudiExamples/options
                              JOBOPTSEARCHPATH+=${CMAKE_SOURCE_DIR}/GaudiExamples/options
                              PARENT_SCOPE)
SET(GAUDIEXAMPLES_PYTHON_PATH ${CMAKE_SOURCE_DIR}/GaudiExamples/tests/qmtest PARENT_SCOPE)
