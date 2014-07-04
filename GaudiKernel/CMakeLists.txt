gaudi_subdir(GaudiKernel v31r0)

gaudi_depends_on_subdirs(GaudiPluginService)

find_package(Boost REQUIRED COMPONENTS filesystem thread system)
find_package(ROOT REQUIRED COMPONENTS Core)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiKernel src/Lib/*.cpp
                  LINK_LIBRARIES ${CMAKE_DL_LIBS} Boost ROOT GaudiPluginService
                  INCLUDE_DIRS Boost ROOT
                  PUBLIC_HEADERS GaudiKernel)

#---Tests-------------------------------------------------------------------
gaudi_add_unit_test(DirSearchPath_test tests/src/DirSearchPath_test.cpp
                    LINK_LIBRARIES GaudiKernel)
gaudi_add_unit_test(test_SerializeSTL tests/src/test_SerializeSTL.cpp
                    LINK_LIBRARIES GaudiKernel)
gaudi_add_unit_test(PathResolver_test tests/src/PathResolver_test.cpp
                    LINK_LIBRARIES GaudiKernel)
gaudi_add_unit_test(test_GaudiTime tests/src/test_GaudiTime.cpp
                    LINK_LIBRARIES GaudiKernel)
gaudi_add_unit_test(test_GaudiTiming tests/src/test_GaudiTiming.cpp
                    LINK_LIBRARIES GaudiKernel)
gaudi_add_unit_test(Parsers_test tests/src/parsers.cpp
                    LINK_LIBRARIES GaudiKernel)
gaudi_add_unit_test(Memory_test tests/src/Memory_test.cpp
                    LINK_LIBRARIES GaudiKernel)
# This is just a compile-time tests.
gaudi_add_unit_test(test_headers_build tests/src/test_headers_build.cpp
                    LINK_LIBRARIES GaudiKernel)

if(GAUDI_BUILD_TESTS)
  set_property(TEST GaudiKernel.PathResolver_test PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

gaudi_add_test(confdb
               COMMAND nosetests ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose/confdb)

if(GAUDI_BUILD_TESTS)
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
