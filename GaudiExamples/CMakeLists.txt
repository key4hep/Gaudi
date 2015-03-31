gaudi_subdir(GaudiExamples v26r1p1)

gaudi_depends_on_subdirs(GaudiKernel GaudiUtils GaudiGSL GaudiAlg RootCnv)

find_package(HepPDT)
find_package(ROOT COMPONENTS Tree RIO Hist)
find_package(Boost COMPONENTS python)
find_package(CLHEP)
find_package(GSL)
find_package(PythonLibs)
find_package(RELAX)

#---Libraries---------------------------------------------------------------
set(GaudiExamples_srcs
    src/AbortEvent/*.cpp
    src/AlgSequencer/*.cpp
    src/AlgTools/*.cpp
    src/DataOnDemand/*.cpp
    src/AlgErrAud/*.cpp
    src/GSLTools/*.cpp
    src/RandomNumber/*.cpp
    src/Histograms/*.cpp
    src/NTuples/*.cpp
    src/TupleEx/*.cpp
    src/CounterEx/*.cpp
    src/PartProp/*.cpp
    src/Properties/*.cpp
    src/ExtendedProperties/*.cpp
    src/IO/*.cpp
    src/ColorMsg/ColorMsgAlg.cpp
    src/History/History.cpp
    src/THist/*.cpp
    src/ErrorLog/ErrorLogTest.cpp
    src/EvtColsEx/EvtColAlg.cpp
    src/Maps/*.cpp
    src/MultipleLogStreams/*.cpp
    src/GaudiCommonTests/*.cpp
    src/IncidentSvc/*.cpp
    src/bug_34121/*.cpp
    src/Auditors/*.cpp
    src/Timing/*.cpp
    src/Selections/*.cpp
    src/SvcInitLoop/*.cpp
    src/StringKeys/*.cpp
    src/StatusCodeSvc/SCSAlg.cpp
    src/StatusCodeSvc/special/SCSAlg_no_inline.cpp
    src/FileMgr/*.cpp
    src/testing/*.cpp
    src/IntelProfiler/*.cpp
    src/PluginService/*.cpp)

if(TARGET RootCnvLib)
  list(APPEND GaudiExamples_srcs src/MultiInput/*.cpp)
endif()

gaudi_add_library(GaudiExamplesLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib HepPDT ROOT
                  INCLUDE_DIRS HepPDT ROOT
                  PUBLIC_HEADERS GaudiExamples)

gaudi_add_module(GaudiExamples ${GaudiExamples_srcs} LINK_LIBRARIES GaudiExamplesLib)

# we compile SCSAlg.cpp in a special way
# FIXME: this is to be compatible with CMT
set_property(SOURCE src/StatusCodeSvc/SCSAlg.cpp
             PROPERTY COMPILE_DEFINITIONS special_fncUnchecked2)

find_file(StatusCodeHeader NAMES GaudiKernel/StatusCode.h
          PATHS ${CMAKE_SOURCE_DIR}/GaudiKernel)
if(NOT StatusCodeHeader)
  message(FATAL_ERROR "Cannot file StatusCode.h for the StatusCodeSvc test")
endif()

message(STATUS "creating hacked version of ${StatusCodeHeader}")
file(READ ${StatusCodeHeader} StatusCodeHeaderData)
string(REPLACE "~StatusCode" "~StatusCode(); void _dummy" StatusCodeHeaderData "${StatusCodeHeaderData}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/HackedStatusCode.h" "${StatusCodeHeaderData}")


gaudi_add_python_module(PyExample src/PythonModule/*.cpp
                        LINK_LIBRARIES ${Boost_PYTHON_LIBRARY}
                        INCLUDE_DIRS Boost PythonLibs)

gaudi_install_python_modules()

#---Executables-------------------------------------------------------------
gaudi_add_executable(Allocator src/Allocator/*.cpp
                     LINK_LIBRARIES GaudiExamplesLib GaudiAlgLib GaudiKernel ROOT CLHEP GSL
                     INCLUDE_DIRS CLHEP GSL)


#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiExamples src/IO/dict.h  src/IO/dict.xml LINK_LIBRARIES GaudiExamplesLib)


#---Tests-------------------------------------------------------------------
# Ensure that we have the test variables in the build-time environment
# (for easier debugging).
gaudi_build_env(SET     STDOPTS          ${CMAKE_CURRENT_SOURCE_DIR}/options
                PREPEND JOBOPTSEARCHPATH ${CMAKE_CURRENT_SOURCE_DIR}/options
                PREPEND JOBOPTSEARCHPATH ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest
                PREPEND PYTHONPATH       ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest)

# FIXME: these variables must be unset in the tests for compatibility with CMT
gaudi_build_env(UNSET GAUDIAPPNAME
                UNSET GAUDIAPPVERSION)

gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
                 STDOPTS=${CMAKE_CURRENT_SOURCE_DIR}/options
                 JOBOPTSEARCHPATH+=${CMAKE_CURRENT_SOURCE_DIR}/options
                 JOBOPTSEARCHPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest
                 PYTHONPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest)
