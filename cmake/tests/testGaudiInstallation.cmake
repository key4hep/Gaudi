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
Tests concerning the installation of Gaudi
------------------------------------------

This tests aim at making sur Gaudi is installed correctly and that
config files are available.

#]========================================================================]

cmake_policy(PUSH)
cmake_policy(VERSION 3.15)

if(DEFINED TEST_GAUDI_INSTALL_BUILD) # build and install Gaudi
    # Clean previous installation if any
    file(REMOVE_RECURSE ${GAUDI_INSTALL_DIR})
    # Install
    execute_process(COMMAND ${CMAKE_COMMAND}
                            --install ${BIN_DIR}
                            --prefix ${GAUDI_INSTALL_DIR}
                    RESULT_VARIABLE returned_value
                    COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Installation of Gaudi failed")
    endif()
elseif(DEFINED TEST_GAUDI_INSTALL_FILES_EXIST) # check some files that should be found in the install destination
    # Check executable and scripts
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/listcomponents)
        message(FATAL_ERROR "Executable not installed correctly,"
            " listcomponents not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/genconf)
        message(FATAL_ERROR "Executable not installed correctly,"
            " genconf not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/make_patch.py)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " make_patch.py not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/gaudirun.py)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " gaudirun.py not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/hivetimeline.py)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " hivetimeline.py not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/compareRootHistos.py)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " compareRootHistos.py not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/GaudiProfiler)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " GaudiProfiler not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/dumpMetaData)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " dumpMetaData not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/IOTest.py)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " IOTest.py not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/EvtColRead.py)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " EvtColRead.py not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/quick-merge)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " quick-merge not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/merge_confdb2_parts)
        message(FATAL_ERROR "Scripts not installed correctly,"
            " merge_confdb2_parts not found in bin/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/bin/TupleEx3.py) # generated files
        message(FATAL_ERROR "Scripts not installed correctly,"
            " TupleEx3.py not found in bin/")
    endif()
    # Check public headers
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/Gaudi/Algorithm.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " Algorithm.h not found in include/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/Gaudi/Details/PluginServiceCommon.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " PluginServiceCommon.h not found in include/Gaudi/Details/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GaudiKernel/Algorithm.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " Algorithm.h not found in include/GaudiKernel/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GaudiUtils/Aida2ROOT.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " Aida2ROOT.h not found in include/GaudiUtils/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GaudiAlg/Fill.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " Fill.h not found in include/GaudiAlg/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GaudiCommonSvc/Axis.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " Axis.h not found in include/GaudiCommonSvc/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GaudiMP/TESSerializer.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " TESSerializer.h not found in include/GaudiMP/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GaudiPython/AlgDecorators.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " AlgDecorators.h not found in include/GaudiPython/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/RootCnv/RootAddress.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " RootAddress.h not found in include/RootCnv/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GaudiExamples/Event.h)
        message(FATAL_ERROR "Include directory not installed correctly,"
            " Event.h not found in include/GaudiExamples/")
    endif()
    # -- check GAUDI_VERSION.h
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/include/GAUDI_VERSION.h)
        message(FATAL_ERROR "Generated header files not installed correctly,"
            " GAUDI_VERSION.h not found in include/")
    endif()
    # Check shared objects
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/libGaudiPluginService.so)
        message(FATAL_ERROR "Shared libraries not installed correctly,"
            " libGaudiPluginService.so not found in lib/")
    endif()
    # Gaudi.components
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/Gaudi.components)
        message(FATAL_ERROR "Components list not found,"
            " Gaudi.components not found in lib/")
    endif()
    # Gaudi.confdb and genconf stuff
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/Gaudi.confdb)
        message(FATAL_ERROR "Merged confdb not found,"
            " Gaudi.confdb not found in lib/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiUtils/GaudiUtilsConf.py)
        message(FATAL_ERROR "genconf python scripts not installed correctly,"
            " GaudiUtilsConf.py not found in python/GaudiUtils/")
    endif()
    # GaudiDict.rootmap
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/GaudiDict.rootmap)
        message(FATAL_ERROR "Merged rootmap not found,"
            " GaudiDict.rootmap not found in lib/")
    endif()
    # _rdict.pcm files
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/GaudiKernelDict_rdict.pcm)
        message(FATAL_ERROR "_rdict.pcm files are not installed correctly,"
            " GaudiKernelDict_rdict.pcm not found in lib/")
    endif()
    # Check python modules
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiKernel/ConfigurableDb.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " ConfigurableDb.py not found in python/GaudiKernel/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/Gaudi/Main.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " Main.py not found in python/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiHive/precedence.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " precedence.py not found in python/GaudiHive/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiMP/GMPBase.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " GMPBase.py not found in python/GaudiMP/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiProfiling/GenerateGaudiOpts.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " GenerateGaudiOpts.py not found in python/GaudiProfiling/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiPython/HistoUtils.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " HistoUtils.py not found in python/GaudiPython/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiSvc/ExtraModules.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " ExtraModules.py not found in python/GaudiSvc/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiTesting/Run.py)
        message(FATAL_ERROR "Python modules not installed correctly,"
            " Run.py not found in python/GaudiTesting/")
    endif()
    # -- check binary python modules
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiExamples/PyExample.so)
        message(FATAL_ERROR "Binary python modules not installed correctly,"
            " PyExample.so not found in python/GaudiExamples/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/python/GaudiProfiling/PyCPUFamily.so)
        message(FATAL_ERROR "Binary python modules not installed correctly,"
            " PyCPUFamily.so not found in python/GaudiProfiling/")
    endif()
    # -- check every python package has a __init__.py file
    file(GLOB python_packages LIST_DIRECTORIES TRUE ${GAUDI_INSTALL_DIR}/python/*)
    foreach(dir IN LISTS python_packages)
        if(IS_DIRECTORY ${dir} AND NOT ${dir} MATCHES "__pycache__" AND NOT EXISTS ${dir}/__init__.py)
            message(FATAL_ERROR "Python packages are not installed correctly,"
            " __init__.py is missing inside ${dir}")
        endif()
    endforeach()
    # Check config files
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/cmake/Gaudi/GaudiConfig.cmake)
        message(FATAL_ERROR "Config not installed correctly,"
            " GaudiConfig.cmake not found in lib/cmake/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/cmake/Gaudi/GaudiConfigVersion.cmake)
        message(FATAL_ERROR "Version not installed correctly,"
            " GaudiConfigVersion.cmake not found in lib/cmake/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/cmake/Gaudi/GaudiTargets.cmake)
        message(FATAL_ERROR "Targets not installed correctly,"
            " GaudiTargets.cmake not found in lib/cmake/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/cmake/Gaudi/GaudiToolbox.cmake)
        message(FATAL_ERROR "gaudi_*() functions not installed correctly,"
            " GaudiToolbox.cmake not found in lib/cmake/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/cmake/Gaudi/GaudiDependencies.cmake)
        message(FATAL_ERROR "Gaudi not installed correctly,"
            " GaudiDependencies.cmake not found in lib/cmake/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/cmake/Gaudi/extract_qmtest_metadata.py)
        message(FATAL_ERROR "gaudi_*() functions not installed correctly,"
            " extract_qmtest_metadata.py not found in lib/cmake/Gaudi/")
    endif()
    if(NOT EXISTS ${GAUDI_INSTALL_DIR}/lib/cmake/Gaudi/modules/Findunwind.cmake)
        message(FATAL_ERROR "find module files not installed correctly,"
            " Findunwind.cmake not found in lib/cmake/Gaudi/modules/")
    endif()
elseif(DEFINED TEST_GAUDI_INSTALL_PACKAGE)
    file(GLOB files "${BIN_DIR}/${PATTERN_TO_REMOVE}")
    if(files)
        file(REMOVE ${files}) # remove old packages
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND}
                        --build ${BIN_DIR}
                        --target package
                    RESULT_VARIABLE returned_value
                    COMMAND_ECHO STDOUT)
    if(NOT returned_value EQUAL "0")
        message(FATAL_ERROR "Packaging of Gaudi failed")
    endif()
else()
    # Try to install (locally in the build tree)
    set(tmpInstallDest ${CMAKE_CURRENT_BINARY_DIR}/GaudiTestInstall)
    add_test(NAME cmake.test_gaudi_install
             COMMAND ${CMAKE_COMMAND} 
                -D TEST_GAUDI_INSTALL_BUILD:BOOL=TRUE
                -D GAUDI_INSTALL_DIR=${tmpInstallDest}
                -D BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
                -P ${CMAKE_CURRENT_LIST_FILE})
    set_tests_properties(cmake.test_gaudi_install PROPERTIES
        FIXTURES_SETUP cmake.test_gaudi_install
        DEPENDS GaudiKernel.test_StatusCodeFail # because this test need to try to compile a target
        LABELS CMake)
    # Check everything is correct
    add_test(NAME cmake.test_gaudi_install_check
             COMMAND ${CMAKE_COMMAND} -D TEST_GAUDI_INSTALL_FILES_EXIST:BOOL=TRUE
                                      -D GAUDI_INSTALL_DIR=${tmpInstallDest}
                                      -P ${CMAKE_CURRENT_LIST_FILE})
    set_tests_properties(cmake.test_gaudi_install_check PROPERTIES
        DEPENDS cmake.test_gaudi_install
        FIXTURES_REQUIRED cmake.test_gaudi_install
        FIXTURES_SETUP cmake.test_gaudi_install_check
        LABELS CMake)
    # Test packaging
    # add_test(NAME cmake.test_gaudi_install_package
    #          COMMAND ${CMAKE_COMMAND} -D TEST_GAUDI_INSTALL_PACKAGE:BOOL=TRUE
    #                                   -D BIN_DIR=${CMAKE_CURRENT_BINARY_DIR}
    #                                   -D PATTERN_TO_REMOVE:STRING=${PROJECT_NAME}-${PROJECT_VERSION}-*.*
    #                                   -P ${CMAKE_CURRENT_LIST_FILE})
    # set_tests_properties(cmake.test_gaudi_install_package PROPERTIES
    #     FIXTURES_SETUP cmake.test_gaudi_install_package
    #     LABELS CMake)
endif()

cmake_policy(POP)
