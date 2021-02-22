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
Test Gaudi downstream
---------------------

Test Gaudi as a dependency of a downstream project with a dummy project by
trying to use a target defined in Gaudi and use gaudi_*() functions.

#]========================================================================]

cmake_policy(PUSH)
cmake_policy(VERSION 3.15)

if(DEFINED TEST_GAUDI_DOWNSTREAM)
    # Clean
    file(REMOVE_RECURSE ${BIN_DIR}/dummyGaudiDownstreamProject)
    # Configure
    if(CMAKE_TOOLCHAIN_FILE)
        set(toolchain "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E env CMAKE_PREFIX_PATH=${GAUDI_INSTALL_DIR}:$ENV{CMAKE_PREFIX_PATH}
                            ${CMAKE_COMMAND}
                            -S ${CMAKE_CURRENT_LIST_DIR}/dummyGaudiDownstreamProject
                            -B ${BIN_DIR}/dummyGaudiDownstreamProject
                            -G ${CMAKE_GENERATOR}
                            ${toolchain}
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Configuration of the downstream project failed.")
    endif()
    # Build
    execute_process(COMMAND ${CMAKE_COMMAND} --build
                            ${BIN_DIR}/dummyGaudiDownstreamProject
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Compilation of the downstream project failed.")
    endif()
    # Try to execute
    execute_process(COMMAND ${BIN_DIR}/dummyGaudiDownstreamProject/run
                            ${BIN_DIR}/dummyGaudiDownstreamProject/mainexe
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Execution of the downstream project failed.")
    endif()
elseif(DEFINED TEST_GAUDI_DOWNSTREAM_FROM_BUILD)
    # Save the cache
    file(MAKE_DIRECTORY ${BIN_DIR}/bck)
    file(COPY ${BIN_DIR}/CMakeCache.txt DESTINATION ${BIN_DIR}/bck)
    file(COPY ${BIN_DIR}/CTestTestfile.cmake DESTINATION ${BIN_DIR}/bck)
    file(COPY ${BIN_DIR}/cmake_install.cmake DESTINATION ${BIN_DIR}/bck)
    file(COPY ${BIN_DIR}/GaudiConfig.cmake DESTINATION ${BIN_DIR}/bck)
    if(CMAKE_TOOLCHAIN_FILE)
        set(toolchain "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()
    # Reconfigure Gaudi
    execute_process(COMMAND ${CMAKE_COMMAND}
                        -D GAUDI_BUILD_TREE_AS_INSTALL_AREA=ON
                        -S ${SRC_DIR}
                        -B ${BIN_DIR}
                        -G ${CMAKE_GENERATOR}
                        ${toolchain}
                    RESULT_VARIABLE returned_value
                    COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        file(RENAME ${BIN_DIR}/bck/CMakeCache.txt ${BIN_DIR}/CMakeCache.txt) # revert to old cache
        file(RENAME ${BIN_DIR}/bck/CTestTestfile.cmake ${BIN_DIR}/CTestTestfile.cmake)
        file(RENAME ${BIN_DIR}/bck/cmake_install.cmake ${BIN_DIR}/cmake_install.cmake)
        file(RENAME ${BIN_DIR}/bck/GaudiConfig.cmake ${BIN_DIR}/GaudiConfig.cmake)
        message(FATAL_ERROR "Reconfiguration of Gaudi failed")
    endif()
    # Configuration
    execute_process(COMMAND ${CMAKE_COMMAND} -E env CMAKE_PREFIX_PATH=${BIN_DIR}:$ENV{CMAKE_PREFIX_PATH}
                            ${CMAKE_COMMAND}
                            -S ${CMAKE_CURRENT_LIST_DIR}/dummyGaudiDownstreamProject
                            -B ${BIN_DIR}/dummyGaudiDownstreamProjectFromBuildTree
                            -G ${CMAKE_GENERATOR}
                            ${toolchain}
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        file(RENAME ${BIN_DIR}/bck/CMakeCache.txt ${BIN_DIR}/CMakeCache.txt) # revert to old cache
        file(RENAME ${BIN_DIR}/bck/CTestTestfile.cmake ${BIN_DIR}/CTestTestfile.cmake)
        file(RENAME ${BIN_DIR}/bck/cmake_install.cmake ${BIN_DIR}/cmake_install.cmake)
        file(RENAME ${BIN_DIR}/bck/GaudiConfig.cmake ${BIN_DIR}/GaudiConfig.cmake)
        message(FATAL_ERROR "Configuration of dummyGaudiDownstreamProject using the build tree of Gaudi failed.")
    endif()
    # Build 
    execute_process(COMMAND ${CMAKE_COMMAND} --build
                            ${BIN_DIR}/dummyGaudiDownstreamProjectFromBuildTree
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        file(RENAME ${BIN_DIR}/bck/CMakeCache.txt ${BIN_DIR}/CMakeCache.txt)
        file(RENAME ${BIN_DIR}/bck/CTestTestfile.cmake ${BIN_DIR}/CTestTestfile.cmake)
        file(RENAME ${BIN_DIR}/bck/cmake_install.cmake ${BIN_DIR}/cmake_install.cmake)
        file(RENAME ${BIN_DIR}/bck/GaudiConfig.cmake ${BIN_DIR}/GaudiConfig.cmake)
        message(FATAL_ERROR "Build of dummyGaudiDownstreamProject using the build tree of Gaudi failed")
    endif()
    # Try to execute
    execute_process(COMMAND ${BIN_DIR}/dummyGaudiDownstreamProjectFromBuildTree/run
                            ${BIN_DIR}/dummyGaudiDownstreamProjectFromBuildTree/mainexe
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    # Reset cached variables
    file(RENAME ${BIN_DIR}/bck/CMakeCache.txt ${BIN_DIR}/CMakeCache.txt)
    file(RENAME ${BIN_DIR}/bck/CTestTestfile.cmake ${BIN_DIR}/CTestTestfile.cmake)
    file(RENAME ${BIN_DIR}/bck/cmake_install.cmake ${BIN_DIR}/cmake_install.cmake)
    file(RENAME ${BIN_DIR}/bck/GaudiConfig.cmake ${BIN_DIR}/GaudiConfig.cmake)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Execution of dummyGaudiDownstreamProject using the build tree of Gaudi failed.")
    endif()
elseif(DEFINED TEST_GAUDI_DOWNSTREAM_FULL_STACK)
    # Configure the FullStack project
    # Configuration
    if(CMAKE_TOOLCHAIN_FILE)
        set(toolchain "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND}
                            -D SRC_DIR=${SRC_DIR}
                            -D BIN_DIR=${BIN_DIR}
                            -S ${CMAKE_CURRENT_LIST_DIR}/dummyLHCbFullStack
                            -B ${BIN_DIR}/dummyLHCbFullStack
                            -G ${CMAKE_GENERATOR}
                            ${toolchain}
                    RESULT_VARIABLE returned_value
                    COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Configuration of dummyLHCbFullStack failed")
    endif()
    # Build
    execute_process(COMMAND ${CMAKE_COMMAND} --build
                            ${BIN_DIR}/dummyLHCbFullStack
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Build of dummyLHCbFullStack failed")
    endif()
    # Try to execute
    execute_process(COMMAND ${BIN_DIR}/dummyLHCbFullStack/run
                            ${BIN_DIR}/dummyLHCbFullStack/dummyGaudiDownstreamProject/mainexe
                            RESULT_VARIABLE returned_value
                            COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Execution of dummyLHCbFullStack failed.")
    endif()
else()
    # Test usability from an install folder
    add_test(NAME cmake.test_dummyGaudiDownstreamProject
             COMMAND ${CMAKE_COMMAND} -D TEST_GAUDI_DOWNSTREAM:BOOL=TRUE
                                      -D GAUDI_INSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}/GaudiTestInstall
                                      -D BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
                                      -D CMAKE_GENERATOR=${CMAKE_GENERATOR}
                                      $<$<BOOL:CMAKE_TOOLCHAIN_FILE>:-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}>
                                      -P ${CMAKE_CURRENT_LIST_FILE})
    set_tests_properties(cmake.test_dummyGaudiDownstreamProject PROPERTIES
        DEPENDS cmake.test_gaudi_install
        FIXTURES_REQUIRED cmake.test_gaudi_install
        FIXTURES_SETUP cmake.test_dummyGaudiDownstreamProject
        LABELS CMake)
    # Test usability from the build tree
    # add_test(NAME cmake.test_dummyGaudiDownstreamProject_from_build_tree
    #          COMMAND ${CMAKE_COMMAND} -D TEST_GAUDI_DOWNSTREAM_FROM_BUILD:BOOL=TRUE
    #                                   -D SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    #                                   -D BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
    #                                   -D CMAKE_GENERATOR=${CMAKE_GENERATOR}
    #                                   $<$<BOOL:CMAKE_TOOLCHAIN_FILE>:-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}>
    #                                   -P ${CMAKE_CURRENT_LIST_FILE})
    # set_tests_properties(cmake.test_dummyGaudiDownstreamProject_from_build_tree PROPERTIES
    #     DEPENDS "cmake.test_gaudi_install;cmake.test_gaudi_install_package" # not to mess up the cache
    #     FIXTURES_SETUP cmake.test_dummyGaudiDownstreamProject_from_build_tree
    #     LABELS CMake)
    # Test usability as part of a stack
    # add_test(NAME cmake.test_dummyGaudiDownstreamProject_full_stack
    #          COMMAND ${CMAKE_COMMAND} -D TEST_GAUDI_DOWNSTREAM_FULL_STACK:BOOL=TRUE
    #                                   -D SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    #                                   -D BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
    #                                   -D CMAKE_GENERATOR=${CMAKE_GENERATOR}
    #                                   $<$<BOOL:CMAKE_TOOLCHAIN_FILE>:-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}>
    #                                   -P ${CMAKE_CURRENT_LIST_FILE})
    # set_tests_properties(cmake.test_dummyGaudiDownstreamProject_full_stack PROPERTIES
    #     DEPENDS cmake.test_dummyGaudiDownstreamProject_from_build_tree
    #     FIXTURES_SETUP cmake.test_dummyGaudiDownstreamProject_full_stack
    #     LABELS CMake)
endif()

cmake_policy(POP)
