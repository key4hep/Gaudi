#####################################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
#[========================================================================[.rst:
Tests of the functions defined im GaudiToolbox.cmake
----------------------------------------------------

NB: It does not check the gaudi_add_module() function because there is a
lot of things happening behind the scene with this function.

#]========================================================================]

cmake_policy(PUSH)
cmake_policy(VERSION 3.15)

if(DEFINED TEST_EXECTUTE_BINARY_DIR)
    # Try to execute locally
    execute_process(COMMAND ${BIN_DIR}/dummyProjectBinaryDir/qux # finds .so with rpath
                            RESULT_VARIABLE returned_value)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "qux cannot be run.")
    endif()
    # Test the script run
    if(NOT EXISTS ${BIN_DIR}/dummyProjectBinaryDir/run)
        message(FATAL_ERROR "run is not generated.")
    endif()
    execute_process(COMMAND ${BIN_DIR}/dummyProjectBinaryDir/run
                            "${CMAKE_CURRENT_LIST_DIR}/dummyProject/tests/test_run.sh"
                            RESULT_VARIABLE returned_value)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "run does not set environment correctly.")
    endif()
    # Try to execute with the script run
    execute_process(COMMAND ${BIN_DIR}/dummyProjectBinaryDir/run
                            ${BIN_DIR}/dummyProjectBinaryDir/qux
                            RESULT_VARIABLE returned_value)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "run does not work but qux does.")
    endif()
elseif(DEFINED TEST_INSTALLATION_LAYOUT)
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR})
        message(FATAL_ERROR "the install directory does not exist")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/lib/libfoo.so)
        message(FATAL_ERROR "libfoo.so shared library is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/lib/libbar.so)
        message(FATAL_ERROR "libbar.so plugin is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/lib/FooBar.components)
        message(FATAL_ERROR "list of all components FooBar.components is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/lib/libbaz.so)
        message(FATAL_ERROR "libbaz.so shared library is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/bin/qux)
        message(FATAL_ERROR "qux executable is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/include/baz.hpp)
        message(FATAL_ERROR "include directory (public headers) is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/include/FOOBAR_VERSION.h)
        message(FATAL_ERROR "version header is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/python/dummyProject/dummyModule.py)
        message(FATAL_ERROR "Python package is not installed")
    endif()
    if(NOT EXISTS ${BIN_DIR}/${INSTALL_DIR}/bin/dummyRun.py)
        message(FATAL_ERROR "Scripts are not installed")
    endif()
    # try to execute
    execute_process(COMMAND qux RESULT_VARIABLE returned_value)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Cannot run the executable qux or something went wrong.")
    endif()
    execute_process(COMMAND dummyRun.py RESULT_VARIABLE returned_value)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Cannot run the script dummyRun.py.")
    endif()
else()
    # Test gaudi_add_* with a dummy project
    # -- clean previous install and cache
    add_test(NAME cmake.test_dummyProject_clean
             COMMAND rm -rf ${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir)
    set_tests_properties(cmake.test_dummyProject_clean PROPERTIES
        FIXTURES_SETUP cmake.test_dummyProject_clean
        LABELS CMake)
    # -- (re)configure
    add_test(NAME cmake.test_dummyProject_configure
             COMMAND ${CMAKE_COMMAND}
                -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir/install
                -DBIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -S ${CMAKE_CURRENT_LIST_DIR}/dummyProject
                -B ${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir
                -G ${CMAKE_GENERATOR}
                $<$<BOOL:CMAKE_TOOLCHAIN_FILE>:-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}>)
    set_tests_properties(cmake.test_dummyProject_configure PROPERTIES
        DEPENDS cmake.test_dummyProject_clean
        FIXTURES_REQUIRED cmake.test_dummyProject_clean
        FIXTURES_SETUP cmake.test_dummyProject_configure
        LABELS CMake)
    # -- build
    add_test(NAME cmake.test_dummyProject_build
             COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir)
    set_tests_properties(cmake.test_dummyProject_build PROPERTIES
        DEPENDS cmake.test_dummyProject_configure
        FIXTURES_REQUIRED cmake.test_dummyProject_configure
        FIXTURES_SETUP cmake.test_dummyProject_build
        LABELS CMake)
    # -- test
    add_test(NAME cmake.test_dummyProject_test
             COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir
                                      --target test)
    set_tests_properties(cmake.test_dummyProject_test PROPERTIES
        DEPENDS cmake.test_dummyProject_build
        FIXTURES_REQUIRED cmake.test_dummyProject_build
        FIXTURES_SETUP cmake.test_dummyProject_test
        LABELS CMake)
    # -- run (binaries from the build tree)
    add_test(NAME cmake.test_dummyProject_run
             COMMAND ${CMAKE_COMMAND} -DTEST_EXECTUTE_BINARY_DIR:BOOL=TRUE
                                      -DBIN_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                                      -P ${CMAKE_CURRENT_LIST_FILE})
    set_tests_properties(cmake.test_dummyProject_run PROPERTIES
        DEPENDS cmake.test_dummyProject_build
        FIXTURES_REQUIRED cmake.test_dummyProject_build
        FIXTURES_SETUP cmake.test_dummyProject_run
        LABELS CMake)
    # -- install
    add_test(NAME cmake.test_dummyProject_install
             COMMAND ${CMAKE_COMMAND}
                --build ${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir
                --target install)
    set_tests_properties(cmake.test_dummyProject_install PROPERTIES
        DEPENDS cmake.test_dummyProject_build
        FIXTURES_REQUIRED cmake.test_dummyProject_build
        FIXTURES_SETUP cmake.test_dummyProject_install
        LABELS CMake)
    # -- check install
    add_test(NAME cmake.test_dummyProject_install_checks
             COMMAND ${CMAKE_COMMAND} -DTEST_INSTALLATION_LAYOUT:BOOL=TRUE
                                      -DBIN_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}
                                      -DINSTALL_DIR=dummyProjectBinaryDir/install
                                      -P ${CMAKE_CURRENT_LIST_FILE})
    set_tests_properties(cmake.test_dummyProject_install_checks PROPERTIES
        DEPENDS cmake.test_dummyProject_install
        FIXTURES_REQUIRED cmake.test_dummyProject_install
        FIXTURES_SETUP cmake.test_dummyProject_install_checks
        LABELS CMake
        ENVIRONMENT "PATH=${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir/install/bin:$ENV{PATH};LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir/install/lib:$ENV{LD_LIBRARY_PATH};PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/dummyProjectBinaryDir/python:${CMAKE_SOURCE_DIR}/cmake/tests/dummyProject/python:$ENV{PYTHONPATH}"
        )
endif()

cmake_policy(POP)
