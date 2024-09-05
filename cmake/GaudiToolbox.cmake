#####################################################################################
# (c) Copyright 2020-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
#[========================================================================[.rst:
GaudiToolbox
============

.. default-domain:: cmake

This file contains all global variables and functions used inside Gaudi's
CMake files.

.. contents:: Contents
    :local:

Global options (added to the project)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. variable:: GAUDI_INSTALL_OPTIONAL

The installation of targets is optional if TRUE. FALSE by default.

.. variable:: GAUDI_GENCONF_NO_FAIL

Prevent genconf to fail if TRUE (for builds with a sanitizer). FALSE by default.

.. variable:: GAUDI_PREFER_LOCAL_TARGETS

When linking against an imported target (``Project::Target``), if there is a local
target with the same name, use that instead of the imported one.
This is useful in to LHCb satellite projects.

.. variable:: GAUDI_PGO

Used to tune Profile Guided Optimization builds. When set to ``GENERATE`` the
binaries are instrumented to generate profile reports on execution, while if set
to ``USE`` the compiler is instructed to optimize the code according to the
collected profile reports. Note that PGO works best when paired to IPO/LTO,
so, if possible, the latter will be enabled by default when using PGO.

.. variable:: GAUDI_PGO_PATH

Directory where to store profile reports when ``GAUDI_PGO=GENERATE`` or read them from
when using ``GAUDI_PGO=USE``.


Other Cached variables (added to the project)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. variable:: CMAKE_INSTALL_BINDIR

CMake will install executable in ``<prefix>/${CMAKE_INSTALL_BINDIR}``

Default value: ``bin``


.. variable:: CMAKE_INSTALL_LIBDIR

CMake will install libraries in ``<prefix>/${CMAKE_INSTALL_LIBDIR}``

Default value: ``lib``


.. variable:: CMAKE_INSTALL_INCLUDEDIR

CMake will install public headers in ``<prefix>/${CMAKE_INSTALL_INCLUDEDIR}``

Default value: ``include``


.. variable:: GAUDI_INSTALL_PLUGINDIR

CMake will install plugins in ``<prefix>/${GAUDI_INSTALL_PLUGINDIR}``

Default value: ``${CMAKE_INSTALL_LIBDIR}``


.. variable:: GAUDI_INSTALL_PYTHONDIR

CMake will install python packages in ``<prefix>/${GAUDI_INSTALL_PYTHONDIR}``

Default value: ``python``


.. variable:: GAUDI_INSTALL_CONFIGDIR

CMake will install cmake files in ``<prefix>/${GAUDI_INSTALL_CONFIGDIR}``

Default value: ``lib/cmake/${PROJECT_NAME}``


Functions
^^^^^^^^^

#]========================================================================]

include_guard(GLOBAL) # Protect from multiple include (global scope, because
                      # everything defined in this file is globally visible)

set(GAUDI_TOOLBOX_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE PATH "Directory containing this file")

################################ Global options ################################

# Option used to know if the install() function must be called with OPTIONAL
# inside gaudi_add_*()
option(GAUDI_INSTALL_OPTIONAL "Target installation is optional if TRUE" FALSE)
if(GAUDI_INSTALL_OPTIONAL)
    set(_gaudi_install_optional OPTIONAL)
endif()

# Option to prevent genconf to fail (turn this to true if you use
# a sanitizer)
option(GAUDI_GENCONF_NO_FAIL "Prevent genconf to fail (for builds with a sanitizer)" FALSE)
if(GAUDI_GENCONF_NO_FAIL)
    set(_gaudi_no_fail "||" "true")
endif()

# Option to prefer local targets to imported ones
option(GAUDI_PREFER_LOCAL_TARGETS "Prefer local targets over imported ones" FALSE)

option(GAUDI_TEST_PUBLIC_HEADERS_BUILD
    "Execute a test build of all public headers in the global target 'all'"
    FALSE)

# Default layout fo the installation (may be overridden in the cache)
set(CMAKE_INSTALL_BINDIR "bin" CACHE STRING "Install executable in <prefix>/\${CMAKE_INSTALL_BINDIR}")
set(CMAKE_INSTALL_LIBDIR "lib" CACHE STRING "Install libraries in <prefix>/\${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE STRING "Install public headers in <prefix>/\${CMAKE_INSTALL_INCLUDEDIR}")
set(GAUDI_INSTALL_PLUGINDIR "${CMAKE_INSTALL_LIBDIR}" CACHE STRING "Install plugins in <prefix>/\${GAUDI_INSTALL_PLUGINDIR}")
set(GAUDI_INSTALL_PYTHONDIR "python" CACHE STRING "Install python packages in <prefix>/\${GAUDI_INSTALL_PYTHONDIR}")
set(GAUDI_INSTALL_CONFIGDIR "lib/cmake/${PROJECT_NAME}" CACHE STRING "Install cmake files in <prefix>/\${GAUDI_INSTALL_CONFIGDIR}")

set(scan_dict_deps_command ${GAUDI_TOOLBOX_DIR}/scan_dict_deps.py
    CACHE INTERNAL "command to use to scan dependencies of dictionary headers")

##################################### PGO  #####################################

if(DEFINED GAUDI_PGO)
    # use case insensitive values for GAUDI_PGO
    string(TOUPPER "${GAUDI_PGO}" GAUDI_PGO)
endif()
if(GAUDI_PGO STREQUAL "GENERATE")
    if("${GAUDI_PGO_PATH}" STREQUAL "")
        message(STATUS "PGO: building to generate execution profiles")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate")
    else()
        message(STATUS "PGO: building to generate execution profiles in ${GAUDI_PGO_PATH}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate=${GAUDI_PGO_PATH}")
    endif()
elseif(GAUDI_PGO STREQUAL "USE")
    if("${GAUDI_PGO_PATH}" STREQUAL "")
        message(STATUS "PGO: use profiles to optimize the build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use -fprofile-correction")
    else()
        message(STATUS "PGO: use profiles from ${GAUDI_PGO_PATH} to optimize the build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use=${GAUDI_PGO_PATH} -fprofile-correction")
    endif()
    if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        include(CheckIPOSupported)
        check_ipo_supported(RESULT CMAKE_INTERPROCEDURAL_OPTIMIZATION LANGUAGES CXX OUTPUT _ipo_output)
    else()
        # this message is printed if the IPO flag was already set (so we do not check)
        # and a few lines below we find it set to false
        set(_ipo_output "explicitly disabled")
    endif()
    if(NOT CMAKE_INTERPROCEDURAL_OPTIMIZATION)
        message(WARNING "PGO works better when combined with IPO, but IPO is not used (${_ipo_output})")
    endif()
elseif(GAUDI_PGO)
    message(FATAL_ERROR "invalid value for GAUDI_PGO, allowed values are 'GENERATE', 'USE' or a false value (GAUDI_PGO == '${GAUDI_PGO}')")
endif()

################################## Functions  ##################################

# Helper function that replaces imported targets in ``<var>`` with local ones,
# if :variable:`GAUDI_PREFER_LOCAL_TARGETS` is true and if the local target exists.
macro(_resolve_local_targets var)
    if(GAUDI_PREFER_LOCAL_TARGETS)
        set(_links)
        foreach(item IN LISTS ${var})
            # if it looks like an imported target (proj::tgt) and the bare target
            # exists, use the bare target
            if(item MATCHES "^.*::([^:]+)\$")
                if(TARGET "${CMAKE_MATCH_1}")
                    set(item "${CMAKE_MATCH_1}")
                endif()
            endif()
            list(APPEND _links "${item}")
        endforeach()
        set(${var} ${_links})
    endif()
endmacro()

# Helper function to prepend a value to the special runtime properties
macro(_gaudi_runtime_prepend runtime value)
    get_property(_orig_value TARGET target_runtime_paths PROPERTY runtime_${runtime})
    set_property(TARGET target_runtime_paths PROPERTY runtime_${runtime} ${value} ${_orig_value})
endmacro()
# Helper function to append a value to the special runtime properties
macro(_gaudi_runtime_append runtime value)
    set_property(TARGET target_runtime_paths APPEND PROPERTY runtime_${runtime} ${value})
endmacro()

# Helper function to add build test for every public header
function(_test_build_public_headers lib_name)
    if(NOT BUILD_TESTING)
        # we test the build of the public headers only when tests are enabled
        return()
    endif()
    # collect the list of public headers
    file(GLOB_RECURSE headers
        RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include
        CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx
        ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
    )
    # list the test source files generated (in a previous config)
    file(GLOB_RECURSE old_srcs
        ${CMAKE_CURRENT_BINARY_DIR}/headers_build_test/*.h.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/headers_build_test/*.hxx.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/headers_build_test/*.hpp.cpp
    )
    set(srcs)
    if(headers)
        # create a cpp file for each header file to test
        foreach(header IN LISTS headers)
            set(s "${CMAKE_CURRENT_BINARY_DIR}/headers_build_test/${header}.cpp")
            configure_file("${GAUDI_TOOLBOX_DIR}/header_build_test.tpl" "${s}")
            list(APPEND srcs "${s}")
        endforeach()
        # create an object library (we never need to use the product)
        add_library(test_public_headers_build_${lib_name} OBJECT EXCLUDE_FROM_ALL ${srcs})
        target_link_libraries(test_public_headers_build_${lib_name}
            PRIVATE ${lib_name})
        target_compile_definitions(test_public_headers_build_${lib_name}
            PRIVATE GAUDI_TEST_PUBLIC_HEADERS_BUILD)
        # add it to the global target "test_public_headers_build" (making sure it exists)
        if(NOT TARGET test_public_headers_build)
            if(GAUDI_TEST_PUBLIC_HEADERS_BUILD)
                add_custom_target(test_public_headers_build ALL
                    COMMENT "test build of public headers")
            else()
                add_custom_target(test_public_headers_build
                    COMMENT "test build of public headers")
            endif()
        endif()
        add_dependencies(test_public_headers_build test_public_headers_build_${lib_name})
        # keep in old_srcs only the source files not needed anymore
        list(REMOVE_ITEM old_srcs ${srcs})
    endif()
    # remove stale test source files
    if(old_srcs)
        file(REMOVE ${old_srcs})
    endif()
endfunction()

#[========================================================================[.rst:
.. command:: gaudi_add_library

  .. code-block:: cmake

    gaudi_add_library(lib_name
                      SOURCES file.cpp...
                      [LINK PUBLIC <lib>... PRIVATE <lib>... INTERFACE <lib>...])

  This function builds a shared library with the given parameters.
  It also provides the alias ``${PROJECT_NAME}::${lib_name}``
  and configure the installation of the library and its public headers.

  .. warning::
    Public headers must be under ``${CMAKE_CURRENT_SOURCE_DIR}/include``
    (e.g. ``${CMAKE_CURRENT_SOURCE_DIR}/include/Gaudi/file.hpp``)

  ``lib_name``
    the name of the target to build

  ``SOURCES file.cpp...``
    the list of file to compile

  ``LINK PUBLIC <lib>... PRIVATE <lib>... INTERFACE <lib>...``
    ``<lib>`` can be ``Package::Lib``, ``MyTarget``, ``SomeLib``
    with the same syntax as :cmake:command:`target_link_libraries`.

    If :variable:`GAUDI_PREFER_LOCAL_TARGETS` is true, override imported targets with
    local ones.
#]========================================================================]
function(gaudi_add_library lib_name)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;LINK"
        ${ARGN}
    )
    _update_headers_db(${lib_name})
    add_library(${lib_name} SHARED "${ARG_SOURCES}")
    # Public headers
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_include_directories(${lib_name}
            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    endif()
    # Dependencies
    if(ARG_LINK)
        _resolve_local_targets(ARG_LINK)
        target_link_libraries(${lib_name} ${ARG_LINK})
    endif()
    # Alias
    add_library(${PROJECT_NAME}::${lib_name} ALIAS ${lib_name})
    # Install the library
    install(
        TARGETS ${lib_name}
        EXPORT  ${PROJECT_NAME}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        ${_gaudi_install_optional})
    # -- and the include directory
    get_property(include_installed DIRECTORY PROPERTY include_installed)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include AND NOT include_installed)
        install(DIRECTORY include/
                TYPE INCLUDE)
        set_property(DIRECTORY PROPERTY include_installed TRUE)
        _test_build_public_headers(${lib_name})
    endif()
    # Runtime ROOT_INCLUDE_PATH
    _gaudi_runtime_append(root_include_path $<TARGET_PROPERTY:${lib_name},INTERFACE_INCLUDE_DIRECTORIES>)
    _gaudi_runtime_prepend(root_include_path ${CMAKE_CURRENT_SOURCE_DIR}/include)
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_add_header_only_library

  .. code-block:: cmake

    gaudi_add_header_only_library(lib_name
                                  [LINK <lib>... ])

  This function registers a header only library (a library that only have
  header files (.h or .hpp)).
  This function does not produce any object files at build time but
  it creates an interface target that one can link against at configure time.
  It also provides the alias ``${PROJECT_NAME}::${lib_name}``
  and configure the installation of the public headers.
  The target is exported and usable in downstream projects.

  .. warning::
    Public headers must be under ``${CMAKE_CURRENT_SOURCE_DIR}/include``
    (e.g. ``${CMAKE_CURRENT_SOURCE_DIR}/include/Gaudi/file.hpp``)

  ``lib_name``
    the name of the target to build

  ``LINK <lib>...``
    ``<lib>`` can be ``Package::Lib``, ``MyTarget``, ``SomeLib``
    with the same syntax as :cmake:command:`target_link_libraries` (``INTERFACE`` is implied).

    If :variable:`GAUDI_PREFER_LOCAL_TARGETS` is true, override imported targets with
    local ones.
#]========================================================================]
function(gaudi_add_header_only_library lib_name)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "LINK"
        ${ARGN}
    )
    _update_headers_db(${lib_name})
    add_library(${lib_name} INTERFACE)
    # Public headers
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
        message(FATAL_ERROR "Cannot find public headers in include/")
    endif()
    target_include_directories(${lib_name}
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    # Dependencies
    if(ARG_LINK)
        _resolve_local_targets(ARG_LINK)
        target_link_libraries(${lib_name} INTERFACE ${ARG_LINK})
    endif()
    # Alias
    add_library(${PROJECT_NAME}::${lib_name} ALIAS ${lib_name})
    # Install the library
    install(
        TARGETS ${lib_name}
        EXPORT  ${PROJECT_NAME}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        ${_gaudi_install_optional})
    get_property(include_installed DIRECTORY PROPERTY include_installed)
    if(NOT include_installed)
        install(DIRECTORY include/
                TYPE INCLUDE)
        set_property(DIRECTORY PROPERTY include_installed TRUE)
        _test_build_public_headers(${lib_name})
    endif()
    # Runtime ROOT_INCLUDE_PATH
    _gaudi_runtime_append(root_include_path $<TARGET_PROPERTY:${lib_name},INTERFACE_INCLUDE_DIRECTORIES>)
    _gaudi_runtime_prepend(root_include_path ${CMAKE_CURRENT_SOURCE_DIR}/include)
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_add_module

  .. code-block:: cmake

    gaudi_add_module(plugin_name
                     SOURCES file.cpp...
                     [LINK <lib>...]
                     [GENCONF_OPTIONS --opt=val...])

  This function builds a plugin library with the given parameters.
  It generates ``.components`` files with listcomponents and ``.confdb``
  and python modules with genconf.
  It also configure the installation.

  ``plugin_name``
    the name of the target to build (not possible to link against)

  ``SOURCES file.cpp...``
    the list of file to compile

  ``LINK <lib>...``
    ``<lib>`` can be ``Package::Lib``, ``MyTarget``, ``SomeLib``
    with the same syntax as :cmake:command:`target_link_libraries` (``PRIVATE`` is implied).

    If :variable:`GAUDI_PREFER_LOCAL_TARGETS` is true, override imported targets with
    local ones.

  ``GENCONF_OPTIONS --opt=val...``
    a list of additional arguments to pass to genconf
    e.g. ``--user-module=package.module --load-library=library``
#]========================================================================]
function(gaudi_add_module plugin_name)
    cmake_parse_arguments(
        ARG
        ""
        ""
        "SOURCES;LINK;GENCONF_OPTIONS"
        ${ARGN}
    )
    add_library(${plugin_name} MODULE "${ARG_SOURCES}")
    # dependencies
    if(ARG_LINK)
        _resolve_local_targets(ARG_LINK)
        target_link_libraries(${plugin_name} PRIVATE ${ARG_LINK})
    endif()
    # install
    install(
        TARGETS ${plugin_name}
        LIBRARY DESTINATION "${GAUDI_INSTALL_PLUGINDIR}"
        ${_gaudi_install_optional})
    # Create a symlink to the module in a directory that is added to the rpath of every target of the build
    add_custom_command(TARGET ${plugin_name} POST_BUILD
        BYPRODUCTS ${CMAKE_BINARY_DIR}/.plugins/${CMAKE_SHARED_MODULE_PREFIX}${plugin_name}${CMAKE_SHARED_MODULE_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:${plugin_name}> ${CMAKE_BINARY_DIR}/.plugins/$<TARGET_FILE_NAME:${plugin_name}>)
    # generate the list of components for all the plugins of the project
    add_custom_command(TARGET ${plugin_name} POST_BUILD
        BYPRODUCTS ${plugin_name}.components
        COMMAND run $<TARGET_FILE:Gaudi::listcomponents> --output ${plugin_name}.components $<TARGET_FILE_NAME:${plugin_name}>)
    _merge_files(${PROJECT_NAME}_MergeComponents "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.components" # see private functions at the end
        ${plugin_name} "${CMAKE_CURRENT_BINARY_DIR}/${plugin_name}.components"
        "Merging .components files for ${PROJECT_NAME}")
    # genconf
    get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${package_name})
    add_custom_command(TARGET ${plugin_name} POST_BUILD
        BYPRODUCTS genConfDir/${package_name}/${plugin_name}.confdb
                   genConfDir/${package_name}/${plugin_name}.confdb2_part
                   genConfDir/${package_name}/${plugin_name}Conf.py # genConfDir/${package_name}/__init__.py
        COMMAND run $<TARGET_FILE:Gaudi::genconf>
            --configurable-module=GaudiKernel.Proxy
            --configurable-default-name=Configurable.DefaultName
            --configurable-algorithm=ConfigurableAlgorithm
            --configurable-algtool=ConfigurableAlgTool
            --configurable-auditor=ConfigurableAuditor
            --configurable-service=ConfigurableService
            ${ARG_GENCONF_OPTIONS}
            -o genConfDir/${package_name}
            -p ${package_name}
            -i $<TARGET_FILE:${plugin_name}>
            ${_gaudi_no_fail}
        COMMAND_EXPAND_LISTS)
    if(TARGET listcomponents) # if we are building Gaudi (this function is not called from a downstream project)
        add_dependencies(${plugin_name} listcomponents genconf GaudiCoreSvc)
    endif()
    set(init_file)
    if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/python/${package_name}/__init__.py) # if there is no python package in the source tree
        set(init_file "${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${package_name}/__init__.py") # we install the __init__.py generated by genconf
    endif()
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${package_name}/${plugin_name}Conf.py
            ${init_file}
        DESTINATION "${GAUDI_INSTALL_PYTHONDIR}/${package_name}" # Install the python module alongside the other python modules of this package
        ${_gaudi_install_optional})
    # Merge the .confdb files
    _merge_files_confdb(${plugin_name} "${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${package_name}/${plugin_name}.confdb") # see private functions at the end
    _merge_files_confdb2(${plugin_name} "${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${package_name}/${plugin_name}.confdb2_part")
    # To append the path to the generated library to LD_LIBRARY_PATH with run
    _gaudi_runtime_prepend(ld_library_path $<TARGET_FILE_DIR:${plugin_name}>)
    # To append the path to the generated Conf.py file to PYTHONPATH
    _gaudi_runtime_prepend(pythonpath ${CMAKE_CURRENT_BINARY_DIR}/genConfDir)
    # Add the path to the merged confdb and components files to LD_LIBRARY_PATH
    _gaudi_runtime_prepend(ld_library_path ${CMAKE_BINARY_DIR})
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_add_python_module

  .. code-block:: cmake

    gaudi_add_python_module(module_name
                            SOURCES file.cpp...
                            [LINK <lib>...]
                            [PACKAGE <path>])

  This function builds a compiled python module from the sources
  and install it.

  ``SOURCES file.cpp...``
    the list of file to compile

  ``LINK <lib>...``
    ``<lib>`` can be ``Package::Lib``, ``MyTarget``, ``SomeLib``
    with the same syntax as :cmake:command:`target_link_libraries` (``PRIVATE`` is implied)

    ``LINK Python::Module`` is implied.

    If :variable:`GAUDI_PREFER_LOCAL_TARGETS` is true, override imported targets with
    local ones.

  ``PACKAGE <path>``
    The path to the python package the compiled module should be part of
    and installed with.

    Default value: ``${CMAKE_CURRENT_SOURCE_DIR}/python/${package_name}``
    (``${package_name}`` is the name of the subdirectory we are in)
#]========================================================================]
function(gaudi_add_python_module module_name)
    cmake_parse_arguments(
        ARG
        ""
        "PACKAGE"
        "SOURCES;LINK"
        ${ARGN}
    )
    # Detect which python package is used and compile the module
    # FIXME: (The day CMake can invoke functions whose name are in a variable, use a loop)
    if(COMMAND Python_add_library)
        Python_add_library(${module_name} MODULE ${ARG_SOURCES})
    elseif(COMMAND Python3_add_library)
        Python3_add_library(${module_name} MODULE ${ARG_SOURCES})
    elseif(COMMAND Python2_add_library)
        Python2_add_library(${module_name} MODULE ${ARG_SOURCES})
    else()
        message(FATAL_ERROR "Python is not available, call find_package(Python) first.")
    endif()
    # dependencies
    if(ARG_LINK)
        _resolve_local_targets(ARG_LINK)
        target_link_libraries(${module_name} PRIVATE ${ARG_LINK})
    endif()
    if(ARG_PACKAGE)
        set(package_path "${ARG_PACKAGE}")
        get_filename_component(package_name "${package_path}" NAME)
    else()
        get_filename_component(package_name "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
        set(package_path "${CMAKE_CURRENT_SOURCE_DIR}/python/${package_name}")
    endif()

    # make the module accessible from the build tree
    add_custom_command(TARGET ${module_name} POST_BUILD
        BYPRODUCTS ${CMAKE_BINARY_DIR}/python/${package_name}/${module_name}${CMAKE_SHARED_MODULE_SUFFIX}
        COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:${module_name}>
            ${CMAKE_BINARY_DIR}/python/${package_name}/$<TARGET_FILE_NAME:${module_name}>)

    install(
        TARGETS ${module_name}
        LIBRARY DESTINATION "${GAUDI_INSTALL_PYTHONDIR}/${package_name}"
        ${_gaudi_install_optional})
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_add_executable

  .. code-block:: cmake

    gaudi_add_executable(exe_name
                         SOURCES file.cpp...
                         [LINK <lib>...]
                         [TEST])

  This function builds an executable with the given parameters.
  It also configure its installation.

  ``exe_name``
    the name of the target to build

  ``SOURCES file.cpp...``
    the list of file to compile

  ``LINK <lib>...``
    ``<lib>`` can be ``Package::Lib``, ``MyTarget``, ``SomeLib``
    with the same syntax as :cmake:command:`target_link_libraries` (``PRIVATE`` is implied).

    If :variable:`GAUDI_PREFER_LOCAL_TARGETS` is true, override imported targets with
    local ones.

  ``TEST``
    This flag specify that the executable must be added to the test set
    to be run by ctest.
#]========================================================================]
function(gaudi_add_executable exe_name)
    cmake_parse_arguments(
        ARG
        "TEST"
        ""
        "SOURCES;LINK"
        ${ARGN}
    )
    if(ARG_TEST AND NOT BUILD_TESTING)
        return()
    endif()
    add_executable(${exe_name} "${ARG_SOURCES}")
    # dependencies
    if(ARG_LINK)
        _resolve_local_targets(ARG_LINK)
        target_link_libraries(${exe_name} PRIVATE ${ARG_LINK})
    endif()
    # test
    if(ARG_TEST)
        get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
        add_test(NAME ${package_name}.${exe_name} COMMAND run $<TARGET_FILE:${exe_name}>)
        set_tests_properties(${package_name}.${exe_name} PROPERTIES LABELS "${PROJECT_NAME};${package_name}")
    endif()
    # install
    set(_export)
    if(NOT ARG_TEST)
        set(_export EXPORT ${PROJECT_NAME})
        add_executable(${PROJECT_NAME}::${exe_name} ALIAS ${exe_name})
    endif()
    install(
        TARGETS  ${exe_name}
        ${_export}
        ${_gaudi_install_optional})
    # Runtime PATH with run
    _gaudi_runtime_prepend(path $<TARGET_FILE_DIR:${exe_name}>)
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_add_tests

  .. code-block:: cmake

    gaudi_add_tests(QMTest|pytest [test_directory])

  This function adds unit tests of a given test framework to the project.
  Test names will be inferred from the directory name and files names.

  ``QMTest``
    Adds QMTest tests.
    Default test_directory is ``${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest``

    .. warning:
      Each time a new .qmt file is added, it is mandatory to reconfigure the project

  ``pytest``
    Adds pytest tests.

    Default test_directory is ``${CMAKE_CURRENT_SOURCE_DIR}/tests/pytest``

  ``test_directory``
    The directory containing the test files.
    Providing a value for this argument overrides the default one.
    Do not put a ``/`` at the end.
#]========================================================================]
function(gaudi_add_tests type)
    if(NOT BUILD_TESTING)
        return()
    endif()

    # Naming convention for tests: PackageName.TestName
    get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    # Custom test_directory
    set(test_directory)
    if(ARGV1)
        set(test_directory "${ARGV1}")
    endif()

    # Test framework used
    if(type STREQUAL "QMTest")

        if(NOT TARGET Python::Interpreter)
            message(FATAL_ERROR "No python interpreter was found, call find_package(Python REQUIRED Interpreter) first.")
        endif()

        if(test_directory)
            set(qmtest_root_dir "${test_directory}")
        else()
            set(qmtest_root_dir "${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest")
        endif()
        file(GLOB_RECURSE qmt_files RELATIVE "${qmtest_root_dir}" "${qmtest_root_dir}/*.qmt") # ordered lexicographically
        string(TOLOWER "${package_name}" subdir_name_lower)
        file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/qmtest_tmp")
        # Add a test for each qmt files
        foreach(qmt_file IN LISTS qmt_files)
            string(REPLACE ".qmt" "" qmt_name "${qmt_file}")
            string(REPLACE ".qms/" "." qmt_name "${qmt_name}")
            string(REGEX REPLACE "^${subdir_name_lower}\\." "" qmt_name "${qmt_name}")
            add_test(NAME ${package_name}.${qmt_name}
                     COMMAND run $<TARGET_FILE:Python::Interpreter> -m GaudiTesting.Run
                        --skip-return-code 77
                        --report ctest
                        --common-tmpdir ${CMAKE_CURRENT_BINARY_DIR}/qmtest_tmp
                        --workdir ${qmtest_root_dir}
                        ${qmtest_root_dir}/${qmt_file}
                     WORKING_DIRECTORY "${qmtest_root_dir}")
            set_tests_properties(${package_name}.${qmt_name} PROPERTIES LABELS "${PROJECT_NAME};${package_name};QMTest"
                                                                        SKIP_RETURN_CODE 77
                                                                        TIMEOUT 0)
        endforeach()
        # Extract dependencies to a cmake file
        find_file(extract_qmtest_metadata extract_qmtest_metadata.py
                  PATHS ${Gaudi_SOURCE_DIR}/cmake # When building Gaudi
                        ${Gaudi_DIR}              # When using an installed Gaudi
                  NO_DEFAULT_PATH)
        if(NOT extract_qmtest_metadata)
            message(FATAL_ERROR "Cannot find extract_qmtest_metadata.py")
        endif()
        mark_as_advanced(extract_qmtest_metadata)
        # Note: we rely on the Python instalation available a configure time
        execute_process(COMMAND python3 ${extract_qmtest_metadata}
                                ${package_name} ${qmtest_root_dir}
                        OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/qmt_deps.cmake
                        RESULT_VARIABLE qmt_deps_retcode)
        if(NOT qmt_deps_retcode EQUAL "0")
            message(FATAL_ERROR "Failed to compute dependencies of QMTest tests.")
            return()
        endif()
        # Include the generated file with the QMTest dependencies
        include(${CMAKE_CURRENT_BINARY_DIR}/qmt_deps.cmake)

    elseif(type STREQUAL "pytest")
        _import_pytest() # creates the imported target pytest for the
                         # generator expression $<TARGET_FILE:pytest>
        if(NOT test_directory)
            set(test_directory "${CMAKE_CURRENT_SOURCE_DIR}/tests/pytest")
        endif()
        get_filename_component(name "${test_directory}" NAME)
        add_test(NAME ${package_name}.${name}
                COMMAND run $<TARGET_FILE:pytest> -v --doctest-modules ${test_directory})
        set_tests_properties(${package_name}.${name} PROPERTIES LABELS "${PROJECT_NAME};${package_name}")
    else()
        message(FATAL_ERROR "${type} is not a valid test framework.")
    endif()
endfunction()

#[========================================================================[.rst:
.. command:: gaudi_add_pytest

  .. code-block:: cmake

    gaudi_add_pytest(
        [path1 [path2...]]
        [ROOT_DIR base/path/for/tests]
        [PREFIX prefix.for.test.name]
        [LABELS label1 label2]
        [OPTIONS pytest_option_1 pytest_option_2]
        [WORKING_DIRECTORY path]
        [PROPERTIES name value...]
    )

  This function is used to declare a directories and/or files with pytest tests.
  The test files are collected and for each of them one CTest test is added using the name
  ``${PREFIX}<filename>`` (where ``<filename>`` is relative to ``${ROOT_DIR}``).
  The list of tests is constructed when ``ctest`` collects the available tests, so additions
  or removals of tests are automatically detected at the cost of a small performance
  penalty when ctest is invoked.

  If :variable:`BUILD_TESTING` is false, this call is a no-op.

  ``[path1 [path2...]]``
    directories or files to be searched for pytest tests

  ``ROOT_DIR base/path/for/tests``
    base directoy to compute relative file names of test files, by default it is
    deduced from the provided paths; explicitly setting the ``ROOT_DIR`` value
    is useful when the test names generated with the default are not satisfactory

  ``PREFIX prefix.for.test.name``
    string to prefix to the filename to construct the test name, by default it
    is ``<current_dir basename>.pytest.<ROOT_DIR base name>.``

  ``LABELS label1 label2``
    optional labels to add to all the tests generated with this call
    (labels ``${PROJECT_NAME}`` , ``<current_dir basename>`` and ``pytest`` are always added)

  ``OPTIONS pytest_option_1 pytest_option_2``
    optional extra arguments for pytest

  ``WORKING_DIRECTORY path``
    where pytest should be executed from, by default set to the current directory

  ``PROPERTIES name value...``
    list of properties to set on the discovered tests
#]========================================================================]
function(gaudi_add_pytest)
    if(NOT BUILD_TESTING)
        return()
    endif()

    # ensure the imported target pytest for the generator expression $<TARGET_FILE:pytest>
    _import_pytest()

    cmake_parse_arguments(
        ARG
        ""
        "PREFIX;ROOT_DIR;WORKING_DIRECTORY"
        "LABELS;OPTIONS;PROPERTIES"
        ${ARGN}
    )
    get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    # check args
    foreach(path IN LISTS ARG_UNPARSED_ARGUMENTS)
        if(IS_ABSOLUTE "${path}")
            set(abs_path "${path}")
        else()
            set(abs_path "${CMAKE_CURRENT_SOURCE_DIR}/${path}")
        endif()
        if(NOT EXISTS "${abs_path}")
            message(FATAL_ERROR "specified path '${path}' does not exist")
        endif()
    endforeach()
    # prepare args to pytest
    if(ARG_UNPARSED_ARGUMENTS)
        string(JOIN " " collect_roots ${ARG_UNPARSED_ARGUMENTS})
    else()
        set(collect_roots ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    # set baseline for PREFIX and ROOT_DIR defaults
    set(ARG_PREFIX_DEFAULT "${package_name}.pytest.")
    set(ARG_ROOT_DIR_DEFAULT "${CMAKE_CURRENT_SOURCE_DIR}")
    # deal with the different possible numbers of paths (none, one, many)
    list(LENGTH ARG_UNPARSED_ARGUMENTS N_of_roots)
    if(N_of_roots EQUAL 0)
        # "no path" is equivalent to pass the current subdirectory
        set(roots_msg "${package_name}")
    elseif(N_of_roots EQUAL 1)
        # one path:
        # - ROOT_DIR defaults to the path passed
        # - PREFIX defaults extended with ROOT_DIR basename
        #   (removing the .py extension if path points to a file)
        set(roots_msg "${package_name}/${ARG_UNPARSED_ARGUMENTS}")
        if("${ARG_ROOT_DIR}" STREQUAL "")
            set(ARG_ROOT_DIR "${ARG_UNPARSED_ARGUMENTS}")
        endif()
        get_filename_component(root_name ${ARG_ROOT_DIR} NAME)
        string(REGEX REPLACE "\\.py\$" "" root_name "${root_name}")
        # we already hawe "pytest." in the default prefix, so we do not
        # want to add it again
        if(NOT root_name STREQUAL "pytest")
            string(APPEND ARG_PREFIX_DEFAULT "${root_name}.")
        endif()
    else()
        # multiple paths, same root and prefix as no path, but need
        # to tune the command line arguments
        string(JOIN " ${package_name}/" roots_msg ${ARG_UNPARSED_ARGUMENTS})
        string(PREPEND roots_msg "${package_name}/")
    endif()
    # check if we need the defaults or not
    if("${ARG_ROOT_DIR}" STREQUAL "")
        set(ARG_ROOT_DIR "${ARG_ROOT_DIR_DEFAULT}")
    endif()
    if("${ARG_PREFIX}" STREQUAL "")
        set(ARG_PREFIX "${ARG_PREFIX_DEFAULT}")
    endif()

    if("${ARG_WORKING_DIRECTORY}" STREQUAL "")
        set(ARG_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    list(TRANSFORM ARG_LABELS PREPEND --ctest-label=)

    set(base_filename ${CMAKE_CURRENT_BINARY_DIR}/pytest/collect_${ARG_PREFIX}${CMAKE_BUILD_TYPE})
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/pytest)
    string(JOIN " " ARG_OPTIONS_CMD ${ARG_OPTIONS})
    string(JOIN "\\ " ARG_OPTIONS_ESC ${ARG_OPTIONS})
    string(JOIN "\\ " ARG_PROPERTIES_ESC WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY} ${ARG_PROPERTIES})
    file(GENERATE OUTPUT ${base_filename}.cmake
        CONTENT "
set(files_to_hash \${CMAKE_CURRENT_LIST_FILE})
foreach(file IN ITEMS ${collect_roots})
    if(NOT IS_ABSOLUTE \${file})
        string(PREPEND file ${CMAKE_CURRENT_SOURCE_DIR}/)
    endif()
    if(IS_DIRECTORY \${file})
        file(GLOB_RECURSE tmp LIST_DIRECTORIES false \${file}/*.py)
        list(APPEND files_to_hash \${tmp})
    else()
        list(APPEND files_to_hash \${file})
    endif()
endforeach()
list(SORT files_to_hash)
set(hash)
foreach(file IN LISTS files_to_hash)
    file(MD5 \${file} tmp)
    string(MD5 hash \${hash}\${file}\${tmp})
endforeach()
set(old_hash)
if(EXISTS ${base_filename}.tests.checksum)
    file(READ ${base_filename}.tests.checksum old_hash)
endif()
if(NOT hash STREQUAL old_hash OR NOT EXISTS ${base_filename}.tests.cmake)
    if(NOT DEFINED PREFECTH_PYTEST_TESTS)
        message(\"... collect pytest tests from ${roots_msg}\")
    endif()
    execute_process(
        COMMAND $<TARGET_FILE:run> $<TARGET_FILE:pytest>
            --collect-only --strict-markers
            ${ARG_OPTIONS_CMD}
            -p GaudiTesting.pytest.collect_for_ctest
            --ctest-output-file=${base_filename}.tests.cmake
            --ctest-pytest-command=$<TARGET_FILE:run>\\ $<TARGET_FILE:pytest>\\ ${ARG_OPTIONS_ESC}
            --ctest-pytest-root-dir=${ARG_ROOT_DIR}
            --ctest-prefix=${ARG_PREFIX}
            --ctest-label=${PROJECT_NAME}
            --ctest-label=${package_name}
            --ctest-properties=${ARG_PROPERTIES_ESC}
            --ctest-binary-dir=${CMAKE_CURRENT_BINARY_DIR}
            ${ARG_LABELS}
            ${collect_roots}
        WORKING_DIRECTORY ${ARG_WORKING_DIRECTORY}
        RESULT_VARIABLE pytest_result
        OUTPUT_VARIABLE pytest_output
    )
    if(NOT pytest_result EQUAL 0)
        message(\"\${pytest_output}\")
        message(FATAL_ERROR \"pytest invocation failed!\")
    endif()
    file(WRITE ${base_filename}.tests.checksum \${hash})
endif()
if(NOT DEFINED PREFECTH_PYTEST_TESTS)
    include(${base_filename}.tests.cmake)
endif()
")

    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES ${base_filename}.cmake)

    string(REGEX REPLACE "\\.$" "" target_name "${ARG_PREFIX}")
    # the same prefix may be used in several places, so we need to somehow disambiguate
    # the generated target names
    while(TARGET "pytest-prefetch-${package_name}-${target_name}")
        string(APPEND target_name ".")
    endwhile()
    add_custom_target(pytest-prefetch-${package_name}-${target_name}
        ALL
        COMMAND ${CMAKE_COMMAND} -DPREFECTH_PYTEST_TESTS=TRUE -P ${base_filename}.cmake
        COMMENT "Collecting pytest tests for ${package_name}:${target_name}"
    )
endfunction()

#[========================================================================[.rst:
.. command:: gaudi_add_dictionary

  .. code-block:: cmake

    gaudi_add_dictionary(dictionary
                         HEADERFILES header.h...
                         SELECTION select.xml
                         [OPTIONS opt...]
                         [LINK <lib>...])

  This function generates the sources of a ROOT reflex dictionary. (build
  target: ``${dictionary}-gen``).
  Then it builds the dictionary as a plugin. (build target: ``${dictionary}``)
  Finally it installs the compiled dictionary and the ``_rdict.pcm`` files alongside
  the other plugins of the project.
  Futhermore, all the ``.rootmap`` files generated are merged in
  ``${PROJECT_NAME}.rootmap`` and this file is installed with the shared
  libraries of the project.

  ``dictionary``
    The name of the target to compile the dictionary.
    By convention, call it ``<GaudiSubDir>Dict``

  ``HEADERFILES header.h...``
    One or more C++ headers

  ``SELECTION select.xml``
    The selection file, XML

  ``OPTIONS opt...``
    options to pass verbatim to genreflex when generating the sources
    e.g. ``--debug``

  ``LINK <lib>...``
    Libraries to link against when building the dictionary.
    ``<lib>`` can be ``Package::Lib``, ``MyTarget``, ``SomeLib``
    with the same syntax as :cmake:command:`target_link_libraries` (``PRIVATE`` is implied).

    If :variable:`GAUDI_PREFER_LOCAL_TARGETS` is true, override imported targets with
    local ones.
#]========================================================================]
function(gaudi_add_dictionary dictionary)
    cmake_parse_arguments(
        ARG
        ""
        "SELECTION"
        "HEADERFILES;OPTIONS;LINK"
        ${ARGN}
    )
    # Generate sources of the dictionary
    set(gensrcdict ${CMAKE_CURRENT_BINARY_DIR}/${dictionary}.cxx)
    set(rootmapname ${CMAKE_CURRENT_BINARY_DIR}/${dictionary}.rootmap)
    set(pcmfile ${CMAKE_CURRENT_BINARY_DIR}/${dictionary}_rdict.pcm)
    if(DEFINED GENREFLEX_JOB_POOL)
        set(job_pool JOB_POOL ${GENREFLEX_JOB_POOL})
    endif()

    # Workaround for rootcling not knowing what nodiscard is
    if(ROOT_VERSION MATCHES "^6\.22.*")
      list(APPEND ARG_OPTIONS -Wno-unknown-attributes)
    endif()

    if(TARGET Python::Interpreter
            AND (CMAKE_GENERATOR MATCHES "Ninja"
                OR (CMAKE_GENERATOR MATCHES "Makefile" AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.20")))
        # Ninja requires the DEPFILE to mention the relative path to the target file
        # (but the relative to what depends on the version of CMake) otherwise the dictionary
        # is always rebuilt.
        if(POLICY CMP0116)
            cmake_policy(PUSH)
            cmake_policy(SET CMP0116 NEW)
            file(RELATIVE_PATH dep_target "${CMAKE_CURRENT_BINARY_DIR}" "${gensrcdict}")
        else()
            file(RELATIVE_PATH dep_target "${CMAKE_BINARY_DIR}" "${gensrcdict}")
        endif()
        add_custom_command(OUTPUT ${gensrcdict} ${rootmapname} ${pcmfile}
            COMMAND run
                ${ROOT_genreflex_CMD} # comes from ROOTConfig.cmake
                    ${ARG_HEADERFILES}
                    -o ${gensrcdict}
                    --rootmap=${rootmapname}
                    --rootmap-lib=lib${dictionary}
                    --select=${ARG_SELECTION}
                    "-I$<JOIN:$<TARGET_PROPERTY:${dictionary},INCLUDE_DIRECTORIES>,;-I>"
                    "-D$<JOIN:$<TARGET_PROPERTY:${dictionary},COMPILE_DEFINITIONS>,;-D>"
                    ${ARG_OPTIONS}
            COMMAND run $<TARGET_FILE:Python::Interpreter>
                ${scan_dict_deps_command}
                    "-I$<JOIN:$<TARGET_PROPERTY:${dictionary},INCLUDE_DIRECTORIES>,;-I>"
                    ${CMAKE_CURRENT_BINARY_DIR}/${dictionary}.d
                    ${dep_target}
                    ${ARG_HEADERFILES}
            DEPENDS ${ARG_HEADERFILES} ${ARG_SELECTION}
            DEPFILE ${CMAKE_CURRENT_BINARY_DIR}/${dictionary}.d
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating ${dictionary}.cxx and ${dictionary}.rootmap and ${dictionary}_rdict.pcm"
            COMMAND_EXPAND_LISTS
            ${job_pool})
        if(POLICY CMP0116)
            cmake_policy(POP)
        endif()
    else()
        if(NOT _root_dicts_deps_warning)
            message(WARNING "dependencies of ROOT dictionaries are not complete, this feature needs Python::Interpreter and a Ninja generator or CMake >= 3.20")
            set(_root_dicts_deps_warning 1 CACHE INTERNAL "")
        endif()
        add_custom_command(OUTPUT ${gensrcdict} ${rootmapname} ${pcmfile}
            COMMAND run
                ${ROOT_genreflex_CMD} # comes from ROOTConfig.cmake
                    ${ARG_HEADERFILES}
                    -o ${gensrcdict}
                    --rootmap=${rootmapname}
                    --rootmap-lib=lib${dictionary}
                    --select=${ARG_SELECTION}
                    "-I$<JOIN:$<TARGET_PROPERTY:${dictionary},INCLUDE_DIRECTORIES>,;-I>"
                    "-D$<JOIN:$<TARGET_PROPERTY:${dictionary},COMPILE_DEFINITIONS>,;-D>"
                    ${ARG_OPTIONS}
            DEPENDS ${ARG_HEADERFILES} ${ARG_SELECTION}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating ${dictionary}.cxx and ${dictionary}.rootmap and ${dictionary}_rdict.pcm"
            COMMAND_EXPAND_LISTS
            ${job_pool})
    endif()

    add_custom_target(${dictionary}-gen ALL DEPENDS "${gensrcdict};${rootmapname};${pcmfile}")
    # Build the dictionary as a plugin
    add_library(${dictionary} MODULE ${gensrcdict})
    if(ARG_LINK)
        _resolve_local_targets(ARG_LINK)
        target_link_libraries(${dictionary} PRIVATE ${ARG_LINK})
    endif()
    target_include_directories(${dictionary} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(${dictionary} PRIVATE GAUDI_DICTIONARY)
    # Install the dictionary
    install(
        TARGETS ${dictionary}
        LIBRARY DESTINATION "${GAUDI_INSTALL_PLUGINDIR}"
        ${_gaudi_install_optional})
    install(FILES ${pcmfile}
        DESTINATION "${GAUDI_INSTALL_PLUGINDIR}"
        ${_gaudi_install_optional})
    # Merge all the .rootmap files
    _merge_files(${PROJECT_NAME}_MergeRootmaps "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Dict.rootmap" # see private functions at the end
        ${dictionary}-gen "${CMAKE_CURRENT_BINARY_DIR}/${dictionary}.rootmap"
        "Merging .rootmap files for ${PROJECT_NAME}")
    # The merged rootmap should also depend on the library (runtime dependency)
    add_dependencies(${PROJECT_NAME}_MergeRootmaps ${dictionary})
    # To append the path to the generated library to LD_LIBRARY_PATH with run
    _gaudi_runtime_prepend(ld_library_path $<TARGET_FILE_DIR:${dictionary}>)
    # Add the path to the merged rootmap file to LD_LIBRARY_PATH
    _gaudi_runtime_prepend(ld_library_path ${CMAKE_BINARY_DIR})

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
       AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0
       AND CMAKE_BUILD_TYPE STREQUAL "FastDebug")
        # Hide warnings produced by gcc 13 from dictionary code that ends up looking like
        # https://godbolt.org/z/je7dE3vr1 (only with `-Og`)
        set_source_files_properties(${gensrcdict} PROPERTIES COMPILE_OPTIONS -Wno-maybe-uninitialized)
    endif()
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_install

  .. code-block:: cmake

    gaudi_install(PYTHON [directory])
    gaudi_install(SCRIPTS [directory])
    gaudi_install(CMAKE files|directories...)

  ``PYTHON [directory]``
    This function installs a Python package (and adds it to the temporary
    ``PYTHONPATH`` used at runtime locally).

    ``directory`` is the he path to the directory to install (absolute or relative
    to ``${CMAKE_CURRENT_SOURCE_DIR}``).  Default value: ``python/``

  ``SCRIPTS [directory]``
    This function installs a directory containing executable scripts (and adds
    it to the temporary ``PATH`` used at runtime locally).

    ``directory`` is the path to the directory to install (absolute or relative
    to ``${CMAKE_CURRENT_SOURCE_DIR}``).  Default value: ``scripts/``

  ``CMAKE files|directories...``
    This function installs cmake files and directories.

    ``files|directories...``
    Paths to files or directories to install (absolute or relative
    to ``${CMAKE_CURRENT_SOURCE_DIR}``).

.. note::
  If directory is specified without a trailing slash (e.g. mydir), the
  directory itself will be installed.
  If directory is specified with a trailing slash (e.g. mydir/), the content
  of the directory will be installed.

  See: `<https://cmake.org/cmake/help/latest/command/install.html#directory>`_
#]========================================================================]
function(gaudi_install type)
    if(type STREQUAL "PYTHON")
        if(ARGV1)
            set(directory ${ARGV1})
        else()
            set(directory python/)
        endif()
        if(NOT IS_ABSOLUTE ${directory})
            set(directory ${CMAKE_CURRENT_SOURCE_DIR}/${directory})
        endif()
        if(NOT EXISTS ${directory})
            message(FATAL_ERROR "Directory ${directory} does not exist")
        endif()
        install(DIRECTORY ${directory}
            DESTINATION "${GAUDI_INSTALL_PYTHONDIR}"
            REGEX "(~|\\.py[co]|\\.in)$" EXCLUDE)
        # Generate a special __init__.py in the build tree that gather every other pieces
        file(GLOB python_packages LIST_DIRECTORIES true "${directory}/*")
        list(FILTER python_packages EXCLUDE REGEX "\\.(py[co]?|in)$")
        foreach(python_package IN LISTS python_packages)
            get_filename_component(python_package_name ${python_package} NAME)
            file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/python/${python_package_name})
            file(WRITE ${CMAKE_BINARY_DIR}/python/${python_package_name}/__init__.py
"import os, sys
__path__ = [d for d in [os.path.join(os.path.realpath(d), '${python_package_name}') for d in sys.path if d]
            if (d.startswith(os.path.realpath('${CMAKE_CURRENT_BINARY_DIR}')) or
                d.startswith(os.path.realpath('${CMAKE_CURRENT_SOURCE_DIR}')) or
                d.startswith(os.path.realpath('${CMAKE_BINARY_DIR}/python/${python_package_name}'))) and
            (os.path.exists(d) or 'python.zip' in d)]
fname = '${CMAKE_CURRENT_SOURCE_DIR}/python/${python_package_name}/__init__.py'
if os.path.exists(fname):
    with open(fname) as f:
        code = compile(f.read(), fname, 'exec')
        exec(code)
")
        endforeach()
        # Runtime PYTHONPATH with run
        _gaudi_runtime_prepend(pythonpath ${directory})
        # make sure the shallow copies in ${CMAKE_BINARY_DIR}/python are first in the PYTHONPATH
        _gaudi_runtime_prepend(pythonpath ${CMAKE_BINARY_DIR}/python)
    elseif(type STREQUAL "SCRIPTS")
        if(ARGV1)
            set(directory ${ARGV1})
        else()
            set(directory scripts/)
        endif()
        if(NOT IS_ABSOLUTE ${directory})
            set(directory ${CMAKE_CURRENT_SOURCE_DIR}/${directory})
        endif()
        if(NOT EXISTS ${directory})
            message(FATAL_ERROR "Directory ${directory} does not exist")
        endif()
        install(DIRECTORY ${directory}
            TYPE BIN
            FILE_PERMISSIONS OWNER_READ GROUP_READ WORLD_READ # same as PROGRAMS
                             OWNER_WRITE
                             OWNER_EXECUTE GROUP_EXECUTE WORLD_EXECUTE
            REGEX "(~|\\.py[co]|\\.in)$" EXCLUDE)
        # Runtime PATH with run
        _gaudi_runtime_prepend(path ${directory})
    elseif(type STREQUAL "CMAKE")
        foreach(entity IN LISTS ARGN)
            if(NOT IS_ABSOLUTE ${entity})
                set(entity "${CMAKE_CURRENT_SOURCE_DIR}/${entity}")
            endif()
            if(NOT EXISTS ${entity})
                message(FATAL_ERROR "Path ${entity} does not exist")
            endif()
            if(IS_DIRECTORY ${entity})
                install(DIRECTORY "${entity}"
                        DESTINATION "${GAUDI_INSTALL_CONFIGDIR}")
            else()
                install(FILES "${entity}"
                        DESTINATION "${GAUDI_INSTALL_CONFIGDIR}")
            endif()
        endforeach()
    else()
        message(FATAL_ERROR "${type} is not a valid first argument for gaudi_install().")
    endif()
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_generate_confuserdb

  .. code-block:: cmake

    gaudi_generate_confuserdb([modules]
                              [OPTIONS ...])

  This function adds ``ConfigurableUser`` specializations
  to ``${PROJECT_NAME}.confdb``.

  ``modules``
    the list of modules that contain the configuration
    they must be inside ``${CMAKE_CURRENT_SOURCE_DIR}/python/``

    Default value: ``${package_name}.Configuration``
    (``${package_name}`` is the name of the subdirectory we are in)

  ``OPTIONS``
    extra options to pass to ``genconfuser.py``
#]========================================================================]
function(gaudi_generate_confuserdb)
    cmake_parse_arguments(PARSE_ARGV 0
        ARG "" "" "OPTIONS"
    )
    # Get package_name
    get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    # Handle default value
    if(NOT ARG_UNPARSED_ARGUMENTS)
        set(modules "${package_name}.Configuration")
    else()
        set(modules "${ARG_UNPARSED_ARGUMENTS}")
    endif()
    # Handle options
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${package_name})
    set(output_file "${CMAKE_CURRENT_BINARY_DIR}/genConfDir/${package_name}/${package_name}_user.confdb")
    # Add the custom target to generate the _user.confdb file
    string(REPLACE "." "/" modules_path_list "${modules}")
    list(TRANSFORM modules_path_list PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/python/")
    list(TRANSFORM modules_path_list APPEND ".py")
    add_custom_command(OUTPUT "${output_file}"
        COMMAND run genconfuser.py
                --build-dir ${CMAKE_BINARY_DIR}
                --project-name ${PROJECT_NAME}
                --root ${CMAKE_CURRENT_SOURCE_DIR}/python
                --output "${output_file}"
                ${ARG_OPTIONS}
                ${package_name}
                ${modules}
                ${_gaudi_no_fail}
        DEPENDS "${modules_path_list}"
        COMMENT "Generating ConfigurableUser DB for ${package_name}"
        COMMAND_EXPAND_LISTS)
    add_custom_target(${package_name}_confuserdb ALL DEPENDS "${output_file}")
    # Merge ${package_name}_user.confdb with the others in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.confdb
    _merge_files_confdb(${package_name}_confuserdb "${output_file}") # see private functions at the end
    # Add the path to the merged confdb file to LD_LIBRARY_PATH
    _gaudi_runtime_prepend(ld_library_path ${CMAKE_BINARY_DIR})
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_check_python_module

  .. code-block:: cmake

    gaudi_check_python_module(modules...)

  This function checks if the specified python modules are available.
  If one or more cannot be imported, an error occurs and the list
  of python modules that cannot be imported is displayed.

  ``modules...``
    a list of python modules or packages that could be imported with ``import``.
    If one of them cannot be imported by python, an error occurs.
#]========================================================================]
function(gaudi_check_python_module)
    if(NOT TARGET Python::Interpreter)
        message(FATAL_ERROR "No python interpreter was found, call find_package(Python COMPONENT Interpreter) first.")
    endif()

    set(not_found)
    foreach(module IN LISTS ARGV)
        execute_process(COMMAND run $<TARGET_FILE:Python::Interpreter> -c "import ${module}"
                        RESULT_VARIABLE returned_value
                        OUTPUT_QUIET ERROR_QUIET)
        if(NOT returned_value EQUAL "0")
            list(APPEND not_found ${module})
        endif()
    endforeach()
    if(not_found)
        message(FATAL_ERROR "The following python module(s) cannot be imported: ${not_found}.")
    endif()
endfunction()


#[========================================================================[.rst:
.. command:: gaudi_generate_version_header_file

  .. code-block:: cmake

    gaudi_generate_version_header_file([name])

  This function generates a file ``${name}Version.h`` in ``${CMAKE_BINARY_DIR}/include``
  with the following content (``${NAME}`` is ``${name}`` uppercased):

  .. code-block:: cpp

      #ifndef ${NAME}_VERSION_H
      #define ${NAME}_VERSION_H

      #ifndef CALC_GAUDI_VERSION
      #define CALC_GAUDI_VERSION(maj,min) (((maj) << 16) + (min))
      #endif

      #define ${NAME}_MAJOR_VERSION ${PROJECT_VERSION_MAJOR}
      #define ${NAME}_MINOR_VERSION ${PROJECT_VERSION_MINOR}
      #define ${NAME}_PATCH_VERSION ${PROJECT_VERSION_PATCH}

      #define ${NAME}_VERSION CALC_GAUDI_VERSION(${NAME}_MAJOR_VERSION,${NAME}_MINOR_VERSION)

      #endif // ${NAME}_VERSION_H

  ``name``
    The name of the (sub-)project.

    If no name is provided, the same file is generated but its name is
    ``toupper(${PROJECT_NAME})_VERSION.h`` and it is installed with the
    other headers.
#]========================================================================]
function(gaudi_generate_version_header_file)
    # Handle default value
    if(NOT ARGV0)
        string(TOUPPER ${PROJECT_NAME} NAME)
        set(output_file_name ${NAME}_VERSION.h)
    else()
        set(name ${ARGV0})
        string(TOUPPER ${name} NAME)
        set(output_file_name ${name}Version.h)
    endif()
    install(FILES ${CMAKE_BINARY_DIR}/include/${output_file_name} TYPE INCLUDE)
    # No patch number => set to 0
    if(NOT PROJECT_VERSION_PATCH)
        set(PROJECT_VERSION_PATCH 0)
    endif()
    # Generate the configure file only once
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/include/${output_file_name} CONTENT
"#ifndef ${NAME}_VERSION_H
#define ${NAME}_VERSION_H

#ifndef CALC_GAUDI_VERSION
#define CALC_GAUDI_VERSION(maj,min) (((maj) << 16) + (min))
#endif

#define ${NAME}_MAJOR_VERSION ${PROJECT_VERSION_MAJOR}
#define ${NAME}_MINOR_VERSION ${PROJECT_VERSION_MINOR}
#define ${NAME}_PATCH_VERSION ${PROJECT_VERSION_PATCH}

#define ${NAME}_VERSION CALC_GAUDI_VERSION(${NAME}_MAJOR_VERSION,${NAME}_MINOR_VERSION)

#endif // ${NAME}_VERSION_H")
endfunction()



################################################################################
#                              Private functions                               #
################################################################################

# This functions merge files generated during the build
# (.components, .rootmap, .confdb) in a single one.
#   merge_target: target to merge the byproducts of other targets
#   output_file: the absolute path to the file that will contain all the others
#   dependency: a target on which ${merge_target} depends
#   file_to_merge: the absolute path to the file to be merged in ${output_file}
#   message: a message to display while building ${merge_target}
function(_merge_files merge_target output_file dependency file_to_merge message)
    if(NOT TARGET ${merge_target})
        add_custom_command(OUTPUT "${output_file}"
            COMMAND $<TARGET_PROPERTY:${merge_target},command> $<TARGET_PROPERTY:${merge_target},fragments>
                    $<TARGET_PROPERTY:${merge_target},output_option> "${output_file}"
            DEPENDS $<TARGET_PROPERTY:${merge_target},fragments>
            COMMENT "${message}"
            COMMAND_EXPAND_LISTS)
        add_custom_target(${merge_target} ALL DEPENDS "${output_file}")
        set_property(TARGET ${merge_target} PROPERTY command cat)
        set_property(TARGET ${merge_target} PROPERTY output_option >)
        install(FILES "${output_file}"
            TYPE LIB
            ${_gaudi_install_optional})
    endif()
    add_dependencies(${merge_target} ${dependency})
    set_property(TARGET ${merge_target} APPEND PROPERTY fragments "${file_to_merge}")
endfunction()
# special wrapper for genconf and genconfuser so that they always have the same merge_target
function(_merge_files_confdb dependency file_to_merge)
    _merge_files(${PROJECT_NAME}_MergeConfdb "${CMAKE_BINARY_DIR}/${PROJECT_NAME}.confdb"
        ${dependency} "${file_to_merge}"
        "Merging .confdb files for ${PROJECT_NAME}")
endfunction()
# special merge function for .confdb2 as a workaround for https://gitlab.cern.ch/gaudi/Gaudi/-/issues/258
function(_merge_files_confdb2 dependency file_to_merge)
    set(merge_target ${PROJECT_NAME}_MergeConfDB2)
    if(NOT TARGET ${merge_target})
        if(APPLE)
            set(output_files ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.confdb2 ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.confdb2.db)
        else()
            set(output_files ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.confdb2)
        endif()
        add_custom_command(OUTPUT ${output_files}
            COMMAND run merge_confdb2_parts $<TARGET_PROPERTY:${merge_target},fragments>
                    --output ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.confdb2
            DEPENDS $<TARGET_PROPERTY:${merge_target},fragments>
            COMMENT "Merging .confdb2 files for ${PROJECT_NAME}"
            COMMAND_EXPAND_LISTS)
        add_custom_target(${merge_target} ALL DEPENDS ${output_files})
        install(FILES ${output_files}
            TYPE LIB
            ${_gaudi_install_optional})
    endif()
    add_dependencies(${merge_target} ${dependency})
    set_property(TARGET ${merge_target} APPEND PROPERTY fragments "${file_to_merge}")
endfunction()

# A function to factor out pytest lookup
function(_import_runtime runtime)
    if(TARGET ${runtime}) # this function can be called several times
        return()
    endif()
    # Ensure that we can find Python 3 specific verions of the test frameworks
    if (runtime STREQUAL "pytest")
        set(binaryfiles "pytest-3;pytest")
    else()
        set(binaryfiles "${runtime}")
    endif()
    foreach(binaryfile ${binaryfiles})
        message(DEBUG "Searching for ${runtime}_PROGRAM as ${binaryfile}")
        find_program(${runtime}_PROGRAM ${binaryfile})
        if(${runtime}_PROGRAM)
            break()
        endif()
    endforeach()
    if (NOT ${runtime}_PROGRAM)
        message(FATAL_ERROR "Unable to find ${runtime}, ${runtime} is required (searched ${binaryfiles}).")
    endif()
    message(DEBUG "Found runtime for ${runtime} as ${${runtime}_PROGRAM}")
    mark_as_advanced(${runtime}_PROGRAM)
    add_executable(${runtime} IMPORTED GLOBAL)
    set_target_properties(${runtime} PROPERTIES IMPORTED_LOCATION ${${runtime}_PROGRAM})
endfunction()


# This function imports pytest to make it usable in gaudi_add_tests(pytest)
function(_import_pytest)
    _import_runtime(pytest)
endfunction()


################################################################################
#                                                                              #
#                            PRIVATE internal magic                            #
#                                                                              #
################################################################################

# Function to generate a CSV file mapping public headers to target providing them to directory
# where it is defined.
file(WRITE "${CMAKE_BINARY_DIR}/headers_db.csv" "header,target,directory\n")
install(FILES "${CMAKE_BINARY_DIR}/headers_db.csv" DESTINATION "${GAUDI_INSTALL_CONFIGDIR}")
macro(_update_headers_db target)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
        file(GLOB_RECURSE _headers RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")
        file(RELATIVE_PATH directory "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
        foreach(header IN LISTS _headers)
            file(APPEND "${CMAKE_BINARY_DIR}/headers_db.csv" "${header},${PROJECT_NAME}::${target},${directory}\n")
        endforeach()
    endif()
endmacro()

# To generate the script run and make it executable
if(NOT CMAKE_SCRIPT_MODE_FILE AND NOT TARGET target_runtime_paths)
    add_custom_target(target_runtime_paths)
    foreach(env_var IN ITEMS PATH LD_LIBRARY_PATH PYTHONPATH ROOT_INCLUDE_PATH)
        # Clean the paths
        string(REGEX REPLACE "(^:|:$)" "" _ENV_${env_var} "$ENV{${env_var}}")
        string(REGEX REPLACE "::+" ":" _ENV_${env_var} "${_ENV_${env_var}}")
        file(TO_CMAKE_PATH "${_ENV_${env_var}}" _ENV_${env_var})
    endforeach()
    string(TOLOWER "${CMAKE_PROJECT_NAME}env.sh" _env_file)
    file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/${_env_file}
                  CONTENT "#!/bin/sh
# Auto-generated script to set environment variables
export PATH=\"$<SHELL_PATH:$<FILTER:$<REMOVE_DUPLICATES:$<GENEX_EVAL:$<TARGET_PROPERTY:target_runtime_paths,runtime_path>>;${_ENV_PATH}>,EXCLUDE,^[^/]>>\${PATH:+:\${PATH}}\"
export LD_LIBRARY_PATH=\"$<SHELL_PATH:$<FILTER:$<REMOVE_DUPLICATES:$<GENEX_EVAL:$<TARGET_PROPERTY:target_runtime_paths,runtime_ld_library_path>>;${_ENV_LD_LIBRARY_PATH}>,EXCLUDE,^[^/]>>\${LD_LIBRARY_PATH:+:\${LD_LIBRARY_PATH}}\"
export PYTHONPATH=\"$<SHELL_PATH:$<FILTER:$<REMOVE_DUPLICATES:$<GENEX_EVAL:$<TARGET_PROPERTY:target_runtime_paths,runtime_pythonpath>>;${_ENV_PYTHONPATH}>,EXCLUDE,^[^/]>>\${PYTHONPATH:+:\${PYTHONPATH}}\"
$<$<NOT:$<STREQUAL:$ENV{PYTHONHOME},>>:export PYTHONHOME=\"$ENV{PYTHONHOME}\">
export ROOT_INCLUDE_PATH=\"$<SHELL_PATH:$<FILTER:$<REMOVE_DUPLICATES:$<GENEX_EVAL:$<TARGET_PROPERTY:target_runtime_paths,runtime_root_include_path>>;${_ENV_ROOT_INCLUDE_PATH}>,EXCLUDE,^[^/]>>\${ROOT_INCLUDE_PATH:+:\${ROOT_INCLUDE_PATH}}\"
export ENV_PROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"
export ENV_CMAKE_BINARY_DIR=\"${CMAKE_BINARY_DIR}\"
export ENV_CMAKE_BUILD_TYPE=\"$<CONFIG>\"
$<IF:$<NOT:$<STREQUAL:${BINARY_TAG},>>,export BINARY_TAG=\"${BINARY_TAG}\",$<$<NOT:$<STREQUAL:$ENV{BINARY_TAG},>>:export BINARY_TAG=\"$ENV{BINARY_TAG}\">>

# Other user defined commands
$<GENEX_EVAL:$<TARGET_PROPERTY:target_runtime_paths,extra_commands>>
${RUN_SCRIPT_EXTRA_COMMANDS}
")
    # Since we cannot tell file(GENERATE) to create an executable file (at generation time)
    # we create one (at configure time) that source the first one, then we make it executable
    file(WRITE ${CMAKE_BINARY_DIR}/run "#!/bin/sh\n# Auto-generated script to set the environment before the execution of a command\n. ${CMAKE_BINARY_DIR}/${_env_file}\nexec \"$@\"\n")
    execute_process(COMMAND chmod a+x ${CMAKE_BINARY_DIR}/run)
    # Add a executable target for convenience
    add_executable(run IMPORTED GLOBAL)
    set_target_properties(run PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/run)
    # Prepend the rpath of every target with the folder containing the symlinks to the plugins
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/.plugins)
    link_directories(BEFORE ${CMAKE_BINARY_DIR}/.plugins)
endif()

# deprecate variables if a target exist
function(__deprecate_var_for_target var access val file)
  if(access STREQUAL "READ_ACCESS" AND
        (NOT file MATCHES ".*Find.+\\.cmake") AND
        (NOT file MATCHES ".*Config\\.cmake") )
    message(DEPRECATION "The variable ${var} is deprecated, use the target instead.")
  endif()
endfunction()
# then call
# variable_watch(var_name __deprecate_var_for_target)
# for each variable that is deprecated
# => use it in every Find*.cmake
