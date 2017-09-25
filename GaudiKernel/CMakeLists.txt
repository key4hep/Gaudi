gaudi_subdir(GaudiKernel)

gaudi_depends_on_subdirs(GaudiPluginService)

find_package(Boost REQUIRED COMPONENTS filesystem thread system regex)
find_package(ROOT REQUIRED COMPONENTS Core)
find_package(TBB)
find_package(CppUnit)

# Hide some Boost/TBB/CppUnit/ROOT compile time warnings
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} ${TBB_INCLUDE_DIRS}
   ${ROOT_INCLUDE_DIRS} )
if( CPPUNIT_FOUND )
   include_directories( SYSTEM ${CPPUNIT_INCLUDE_DIRS} )
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiKernel src/Lib/*.cpp
                  LINK_LIBRARIES ${CMAKE_DL_LIBS} Boost ROOT TBB GaudiPluginService
                  INCLUDE_DIRS Boost ROOT TBB
                  PUBLIC_HEADERS GaudiKernel)
#Make sure VectorClass uses a custom namespace to avoid conflicts
set_property(TARGET GaudiKernel APPEND_STRING PROPERTY COMPILE_FLAGS " -DVCL_NAMESPACE=Gaudi ")

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
   gaudi_add_unit_test(test_GaudiTiming tests/src/test_GaudiTiming.cpp
      LINK_LIBRARIES GaudiKernel)
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
   gaudi_add_unit_test(test_headers_build tests/src/test_headers_build.cpp
      LINK_LIBRARIES GaudiKernel)
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
                 COMMAND ${env_cmd} --xml ${env_xml}
                           ${genconf_cmd_name}
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
gaudi_add_unit_test(test_IssueSeverity tests/src/test_IssueSeverity.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)
gaudi_add_unit_test(test_SystemTypeinfoName tests/src/test_SystemTypeinfoName.cpp
                    LINK_LIBRARIES GaudiKernel
                    TYPE Boost)

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
