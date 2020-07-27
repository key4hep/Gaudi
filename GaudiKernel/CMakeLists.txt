#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
gaudi_subdir(GaudiKernel)

gaudi_depends_on_subdirs(GaudiPluginService GaudiPolicy)

find_package(Boost REQUIRED COMPONENTS filesystem thread system regex)
find_package(ROOT REQUIRED COMPONENTS Core)
find_package(TBB REQUIRED)
find_package(CppUnit)
# Runtime dependency for ROOT (required for SLC6 as of LCG 96)
find_package(LibLZMA)

# Hide some Boost/TBB/CppUnit/ROOT compile time warnings
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
   ${ROOT_INCLUDE_DIRS} )
if( CPPUNIT_FOUND )
   include_directories( SYSTEM ${CPPUNIT_INCLUDE_DIRS} )
endif()

# Look for VectorClass required header
find_path(VectorClass_INCLUDE_DIR NAMES instrset_detect.cpp)
if(VectorClass_INCLUDE_DIR AND NOT VectorClass_VERSION)
  # check that the version is good enough
  set(VectorClass_VERSION 0.0)
  file(STRINGS ${VectorClass_INCLUDE_DIR}/instrset.h _vectorclass_guard REGEX "define +INSTRSET_H +[0-9]+")
  list(GET _vectorclass_guard 0 _vectorclass_guard)
  if(_vectorclass_guard MATCHES "INSTRSET_H +([0-9][0-9][0-9][0-9][0-9])")
    string(REGEX REPLACE "([0-9]+)([0-9][0-9])([0-9][0-9])" "\\1.\\2.\\3" VectorClass_VERSION "${CMAKE_MATCH_1}")
  elseif(_vectorclass_guard MATCHES "INSTRSET_H +([0-9][0-9][0-9])")
    string(REGEX REPLACE "([0-9]+)([0-9][0-9])" "\\1.\\2" VectorClass_VERSION "${CMAKE_MATCH_1}")
  endif()
  set(VectorClass_VERSION "${VectorClass_VERSION}" CACHE INTERNAL "")
endif()
if(NOT VectorClass_INCLUDE_DIR OR VectorClass_VERSION VERSION_LESS 2.01.02)
  if(VectorClass_INCLUDE_DIR)
    message(STATUS "Found VectorClass instrset_detect ${VectorClass_VERSION} at ${VectorClass_INCLUDE_DIR}")
  endif()
  message(WARNING "using internal VectorClass instrset_detect")
  set(VectorClass_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/contrib" CACHE PATH "" FORCE)
  set(VectorClass_VERSION "1.25" CACHE INTERNAL "" FORCE)
endif()
mark_as_advanced(VectorClass_INCLUDE_DIR)
message(STATUS "Using VectorClass instrset_detect ${VectorClass_VERSION} at ${VectorClass_INCLUDE_DIR}")

include_directories(${VectorClass_INCLUDE_DIR})

# Extra settings when building on MacOS:
set( extra_sources )
set( extra_libraries )
if( APPLE )
   set( extra_sources src/Lib/Platform/SystemMacOS.h
                      src/Lib/Platform/SystemMacOS.mm )
   find_library( FOUNDATION_FRAMEWORK Foundation )
   set( extra_libraries ${FOUNDATION_FRAMEWORK} )
elseif( WIN32 )
   set( extra_sources src/Lib/Platform/SystemWin32.h
                      src/Lib/Platform/SystemWin32.cpp )
elseif( UNIX )
   # The UNIX variable is set for MacOS as well, so we can't put this in the
   # front.
   set( extra_sources src/Lib/Platform/SystemLinux.h
                      src/Lib/Platform/SystemLinux.cpp )
   set( extra_libraries rt )
endif()

# Only use LibLZMA if it was found. As in a standalone build, or when using
# an older LCG version, it may not have been.
if( LIBLZMA_FOUND )
  list( APPEND extra_libraries LibLZMA )
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiKernel src/Lib/*.cpp ${extra_sources}
                  LINK_LIBRARIES ${CMAKE_DL_LIBS} Boost ROOT TBB GaudiPluginService ${extra_libraries} fmt::fmt
                  INCLUDE_DIRS Boost ROOT TBB ${CPPGSL_INCLUDE_DIRS}
                  PUBLIC_HEADERS Gaudi GaudiKernel)

# Ensure libfmt is on the LD_LIBRARY_PATH
string(TOUPPER "${CMAKE_BUILD_TYPE}" _upper_build_type)
set(fmt_loc "")
foreach(_type "_${_upper_build_type}" "_RELEASE" "_DEBUG" "_RELWITHDEBINFO" "")
   get_property(_locIsSet TARGET fmt::fmt
                PROPERTY "IMPORTED_LOCATION${_type}" SET )
   if(NOT _locIsSet)
      continue()
   endif()
   unset(_locIsSet)
   get_target_property(fmt_loc fmt::fmt "IMPORTED_LOCATION${_type}")
   break()
endforeach()
if("${fmt_loc}" STREQUAL "")
   message(SEND_ERROR "Could not find the location of the fmt library")
endif()
get_filename_component(fmt_loc "${fmt_loc}" DIRECTORY)
get_target_property(fmt_inc fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
gaudi_env(PREPEND LD_LIBRARY_PATH "${fmt_loc}"
          PREPEND ROOT_INCLUDE_PATH "${fmt_inc}")

#---Tests-------------------------------------------------------------------
if( CPPUNIT_FOUND )
   gaudi_add_unit_test(DirSearchPath_test tests/src/DirSearchPath_test.cpp
      LINK_LIBRARIES GaudiKernel)
   gaudi_add_unit_test(test_SerializeSTL tests/src/test_SerializeSTL.cpp
      LINK_LIBRARIES GaudiKernel)
   gaudi_add_unit_test(test_AnyDataObject tests/src/test_AnyDataObject.cpp
      LINK_LIBRARIES GaudiKernel)
   gaudi_add_unit_test(test_DataHandleVector tests/src/test_DataHandleVector.cpp
      LINK_LIBRARIES GaudiKernel)
   gaudi_add_unit_test(PathResolver_test tests/src/PathResolver_test.cpp
      LINK_LIBRARIES GaudiKernel)
   if( GAUDI_BUILD_TESTS )
      set_property(TEST GaudiKernel.PathResolver_test PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
   endif()
   gaudi_add_unit_test(test_GaudiTime tests/src/test_GaudiTime.cpp
      LINK_LIBRARIES GaudiKernel)
   if (NOT SANITIZER_ENABLED)
     gaudi_add_unit_test(test_GaudiTiming tests/src/test_GaudiTiming.cpp
        LINK_LIBRARIES GaudiKernel)
   endif()
   gaudi_add_unit_test(Parsers_test tests/src/parsers.cpp
      LINK_LIBRARIES GaudiKernel)
   gaudi_add_unit_test(Memory_test tests/src/Memory_test.cpp
      LINK_LIBRARIES GaudiKernel)
   gaudi_add_unit_test(SerialTaskQueue_test tests/src/SerialTaskQueue_test.cpp
      LINK_LIBRARIES GaudiKernel TBB)
   gaudi_add_unit_test(test_LockedHandle tests/src/test_LockedHandle.cpp)
   if(CMAKE_SYSTEM_NAME MATCHES Linux)
      target_link_libraries(test_LockedHandle -Wl,--no-as-needed -ldl -Wl,--as-needed ${Boost_LIBRARIES})
   else()
      target_link_libraries(test_LockedHandle ${Boost_LIBRARIES})
   endif()
endif()
gaudi_add_unit_test(ContextSpecificPtrTest tests/src/test_ContextSpecificPtr.cpp
   LINK_LIBRARIES GaudiKernel TYPE Boost)

# These are a compile-time tests:
if( CPPUNIT_FOUND )
   gaudi_add_unit_test(test_GAUDI-905 tests/src/GAUDI-905.cpp
      LINK_LIBRARIES GaudiKernel)
endif()

if( GAUDI_CPP11 AND CPPUNIT_FOUND )
  gaudi_add_unit_test(test_GAUDI-973 tests/src/GAUDI-973.cpp
                      LINK_LIBRARIES GaudiKernel)
endif()

gaudi_add_test(confdb
               COMMAND nosetests -v
               ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose/confdb)

gaudi_add_test(nose
               COMMAND nosetests -v
               ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose)

if( GAUDI_BUILD_TESTS )
  add_library(test_CustomFactory MODULE tests/src/custom_factory.cpp)
  target_link_libraries(test_CustomFactory GaudiPluginService)
  install(TARGETS test_CustomFactory LIBRARY DESTINATION lib OPTIONAL)

  get_filename_component(genconf_cmd_name ${genconf_cmd} NAME)
  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/custom_factory_testdir)
  gaudi_add_test(genconf_with_custom_factory
                 COMMAND ${genconf_cmd_name}
                             -o ${CMAKE_CURRENT_BINARY_DIR}/custom_factory_testdir
                             -p CustomFactoryTest
                             -i test_CustomFactory
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/custom_factory_testdir)
endif()

if( CPPUNIT_FOUND )
   gaudi_add_unit_test(AttribStringParser_test tests/src/test_AttribStringParser.cpp
      LINK_LIBRARIES GaudiKernel)
endif()
gaudi_add_unit_test(test_PropertyHolder tests/src/test_PropertyHolder.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_Property tests/src/test_Property.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_StatusCode tests/src/test_StatusCode.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_EventIDBase tests/src/test_EventIDBase.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_EventIDRange tests/src/test_EventIDRange.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_SystemTypeinfoName tests/src/test_SystemTypeinfoName.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_SystemCmdLineArgs tests/src/test_SystemCmdLineArgs.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_compose  tests/src/test_compose.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_reverse  tests/src/test_reverse.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_GaudiTimer tests/src/test_GaudiTimer.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_Counters tests/src/CountersUnitTest.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_MonotonicArena tests/src/test_MonotonicArena.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)

gaudi_add_compile_test(test_StatusCodeFail tests/src/test_StatusCode_fail.cxx
                       ERRORS "FAIL01;FAIL02;FAIL03;FAIL04")

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiKernel dict/dictionary.h  dict/dictionary.xml LINK_LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
# Disable generation of ConfUserDB (must be done before gaudi_install_python_modules)
set_directory_properties(PROPERTIES CONFIGURABLE_USER_MODULES None)

gaudi_install_python_modules()
gaudi_install_scripts()

#---Test-----------------------------------------------------------------------
gaudi_add_test(QMTest QMTEST)

#---Special options
if(GAUDI_HIDE_WARNINGS)
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

add_executable(profile_Property tests/src/profile_Property.cpp)
target_link_libraries(profile_Property GaudiKernel)

find_program(gdb_command NAMES gdb)
if(gdb_command)
  get_filename_component(gdb_bindir "${gdb_command}" PATH)
  # check if gdb was found in $ENV{PATH}
  file(TO_CMAKE_PATH "$ENV{PATH}" envpath)
  list(FIND envpath ${gdb_bindir} idx)
  if(idx LESS 0)
    # it's not in the standard PATH, so we have to add it
    gaudi_env(PREPEND PATH ${gdb_bindir})
  endif()
else()
  message(WARNING "gdb not found, you will not be able to have stack traces for problems")
endif()
