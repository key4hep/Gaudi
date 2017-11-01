gaudi_subdir(GaudiExamples)

gaudi_depends_on_subdirs(GaudiKernel GaudiUtils GaudiGSL GaudiAlg RootCnv)

find_package(AIDA)
find_package(HepPDT)
find_package(ROOT COMPONENTS Tree RIO Hist Net REQUIRED)
find_package(Boost COMPONENTS python REQUIRED)
find_package(CLHEP)
find_package(GSL)
find_package(PythonLibs)
find_package(RELAX)
find_package(TBB)
# ROOT runtime
find_package(PNG)

# Hide some Boost/ROOT/CLHEP/TBB/HepPDT compile time warnings
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   ${TBB_INCLUDE_DIRS} )
if( CLHEP_FOUND )
   include_directories( SYSTEM ${CLHEP_INCLUDE_DIRS} )
endif()
if( HEPPDT_FOUND )
   include_directories( SYSTEM ${HEPPDT_INCLUDE_DIRS} )
endif()

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
  # FindOpenCL.cmake has been introduced in CMake 3.1.0
  find_package(OpenCL)
  if(NOT OpenCL_FOUND)
    message(STATUS "OpenCL not found, not building OpenCLAlg example.")
  endif()
endif()

#---Libraries---------------------------------------------------------------
set(GaudiExamples_srcs
    src/AbortEvent/*.cpp
    src/AlgSequencer/*.cpp
    src/AlgTools/*.cpp
    src/AnyData/*.cpp
    src/DataOnDemand/*.cpp
    src/AlgErrAud/*.cpp
    src/NTuples/*.cpp
    src/CounterEx/*.cpp
    src/Properties/*.cpp
    src/ExtendedProperties/*.cpp
    src/ColorMsg/ColorMsgAlg.cpp
    src/History/History.cpp
    src/THist/*.cpp
    src/ErrorLog/ErrorLogTest.cpp
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
    src/PluginService/*.cpp
    src/ToolHandles/*.cpp
)

# Add the sources requiring non-standard externals:
if( AIDA_FOUND AND GSL_FOUND AND CLHEP_FOUND )
   list( APPEND GaudiExamples_srcs src/GSLTools/*.cpp )
endif()
if( AIDA_FOUND )
   list( APPEND GaudiExamples_srcs src/Histograms/*.cpp
      src/RandomNumber/*.cpp
      src/EvtColsEx/EvtColAlg.cpp
      src/IO/*.cpp )
endif()
if( HEPPDT_FOUND )
   list( APPEND GaudiExamples_srcs src/PartProp/*.cpp )
endif()
if( CLHEP_FOUND )
   list( APPEND GaudiExamples_srcs src/TupleEx/*.cpp )
endif()
if( TARGET RootCnvLib )
  list( APPEND GaudiExamples_srcs src/MultiInput/*.cpp )
endif()
if( OpenCL_FOUND )
   list( APPEND GaudiExamples_srcs src/OpenCL/*.cpp )
endif()
if( RANGES_V3_FOUND )
   list( APPEND GaudiExamples_srcs src/FunctionalAlgorithms/*.cpp )
endif()

# Set up the include and linker flags for the examples libraries:
set( extra_includes )
set( extra_libs )
if( AIDA_FOUND )
   list( APPEND extra_includes ${AIDA_INCLUDE_DIRS} )
   list( APPEND extra_libs     ${AIDA_LIBRARIES} )
endif()
if( CLHEP_FOUND )
   list( APPEND extra_includes ${CLHEP_INCLUDE_DIRS} )
   list( APPEND extra_libs     ${CLHEP_LIBRARIES} )
endif()
if( HEPPDT_FOUND )
   list( APPEND extra_includes ${HEPPDT_INCLUDE_DIRS} )
   list( APPEND extra_libs     ${HEPPDT_LIBRARIES} )
endif()
if( TARGET GaudiGSLLib )
   list( APPEND extra_libs GaudiGSLLib )
endif()
if( TARGET RootCnvLib )
   list( APPEND extra_libs RootCnvLib )
endif()
if( OpenCL_FOUND )
   list( APPEND extra_includes ${OpenCL_INCLUDE_DIRS} )
   list( APPEND extra_libs ${OpenCL_LIBRARIES} )
endif()

# Now set up the libraries:
gaudi_add_library( GaudiExamplesLib src/Lib/*.cpp
   LINK_LIBRARIES GaudiKernel
   PUBLIC_HEADERS GaudiExamples )

gaudi_add_module( GaudiExamples ${GaudiExamples_srcs}
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${extra_includes}
   LINK_LIBRARIES GaudiKernel GaudiAlgLib GaudiUtilsLib GaudiExamplesLib
   ${ROOT_LIBRARIES} ${extra_libs} )

find_file(StatusCodeHeader NAMES GaudiKernel/StatusCode.h
          PATHS ${CMAKE_SOURCE_DIR}/GaudiKernel
          HINTS ${Gaudi_DIR}/include)
if(NOT StatusCodeHeader)
  message(FATAL_ERROR "Cannot find file StatusCode.h for the StatusCodeSvc test")
endif()

message(STATUS "creating hacked version of ${StatusCodeHeader}")
file(READ ${StatusCodeHeader} StatusCodeHeaderData)
string(REPLACE "~StatusCode" "__attribute__ ((noinline)) ~StatusCode" StatusCodeHeaderData "${StatusCodeHeaderData}")
if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/HackedStatusCode.h")
  file(READ "${CMAKE_CURRENT_BINARY_DIR}/HackedStatusCode.h" StatusCodeHeaderData_previous)
endif()
if(NOT StatusCodeHeaderData_previous STREQUAL StatusCodeHeaderData)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/HackedStatusCode.h" "${StatusCodeHeaderData}")
endif()

if(Boost_PYTHON_FOUND)
  gaudi_add_python_module(PyExample src/PythonModule/*.cpp
                          LINK_LIBRARIES ${Boost_PYTHON_LIBRARY}
                          INCLUDE_DIRS Boost PythonLibs)
endif()

gaudi_install_python_modules()

#---Executables-------------------------------------------------------------
if( CLHEP_FOUND AND GSL_FOUND AND TARGET GaudiExamplesLib AND
      TARGET GaudiAlgLib )
   gaudi_add_executable(Allocator src/Allocator/*.cpp
      LINK_LIBRARIES GaudiExamplesLib GaudiAlgLib GaudiKernel ROOT CLHEP GSL
      INCLUDE_DIRS CLHEP GSL)
endif()

#---Dictionaries------------------------------------------------------------
if( TARGET GaudiExamplesLib )
   gaudi_add_dictionary(GaudiExamples src/IO/dict.h  src/IO/dict.xml LINK_LIBRARIES GaudiExamplesLib)
endif()

#---Tests-------------------------------------------------------------------
# Ensure that we have the test variables in the build-time environment
# (for easier debugging).
gaudi_build_env(SET     STDOPTS          ${CMAKE_CURRENT_SOURCE_DIR}/options
                PREPEND JOBOPTSEARCHPATH ${CMAKE_CURRENT_SOURCE_DIR}/options
                PREPEND JOBOPTSEARCHPATH ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest
                PREPEND PYTHONPATH       ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest)

gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
               BINARY_TAG=${BINARY_TAG}
               STDOPTS=${CMAKE_CURRENT_SOURCE_DIR}/options
               JOBOPTSEARCHPATH+=${CMAKE_CURRENT_SOURCE_DIR}/options
               JOBOPTSEARCHPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest
               PYTHONPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest)

gaudi_add_test(WriteAndReadHandle
               FRAMEWORK options/ROOT_IO/WriteAndReadHandle.py)

gaudi_add_test(WriteAndReadHandleError
               FRAMEWORK options/ROOT_IO/WriteAndReadHandleError.py
               FAILS)

gaudi_add_test(WriteAndReadHandleWhiteBoard
               FRAMEWORK options/ROOT_IO/WriteAndReadHandleWhiteBoard.py)

gaudi_add_test(nose
               COMMAND nosetests -v
               ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose)
