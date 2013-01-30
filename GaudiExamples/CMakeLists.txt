gaudi_subdir(GaudiExamples v23r6)

gaudi_depends_on_subdirs(GaudiKernel GaudiUtils GaudiGSL GaudiAlg)

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
    src/POOLIO/*.cpp
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
    src/StatusCodeSvc/*.cpp
    src/testing/*.cpp
    src/IntelProfiler/*.cpp)

gaudi_add_library(GaudiExamplesLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiGSLLib GaudiUtilsLib HepPDT ROOT
                  INCLUDE_DIRS HepPDT ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src/Lib
                  NO_PUBLIC_HEADERS)

gaudi_add_module(GaudiExamples ${GaudiExamples_srcs} LINK_LIBRARIES GaudiExamplesLib)

gaudi_add_python_module(PyExample src/PythonModule/*.cpp
                        LINK_LIBRARIES ${Boost_PYTHON_LIBRARY}
                        INCLUDE_DIRS Boost PythonLibs)

gaudi_install_python_modules()

#---Executables-------------------------------------------------------------
gaudi_add_executable(Allocator src/Allocator/*.cpp
                     LINK_LIBRARIES GaudiExamplesLib GaudiAlgLib GaudiKernel ROOT CLHEP GSL
                     INCLUDE_DIRS CLHEP GSL)


#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiExamples src/POOLIO/dict.h  src/POOLIO/dict.xml LINK_LIBRARIES GaudiExamplesLib)


#---Tests-------------------------------------------------------------------
gaudi_add_test(Properties FRAMEWORK options/Properties.opts ENVIRONMENT JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/options)
gaudi_add_test(AlgTools FRAMEWORK options/AlgTools.opts ENVIRONMENT JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/options)

# Ensure that we have the test variables in the build-time environment
# (for easier debugging).
gaudi_build_env(SET     STDOPTS          ${CMAKE_CURRENT_SOURCE_DIR}/options
                PREPEND JOBOPTSEARCHPATH ${CMAKE_CURRENT_SOURCE_DIR}/options
                PREPEND PYTHONPATH       ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest)

gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
                 STDOPTS=${CMAKE_CURRENT_SOURCE_DIR}/options
                 JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/options
                 PYTHONPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest)
