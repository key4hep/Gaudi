#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# - Locate Intel Amplifier toolkit
# Defines:
#
#  INTELAMPLIFIER_FOUND
#  INTELAMPLIFIER_INCLUDE_DIR
#  INTELAMPLIFIER_INCLUDE_DIRS (not cached)
#  INTELAMPLIFIER_LIB_DIRS
#  INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE
#  INTELAMPLIFIER_LIBITTNOTIFY
#
# ENV{INTELAMPLIFIER_INSTALL_DIR} will searched first to find the paths
# Otherwise it will try to search for the amplxe-cl in the ENV{PATH}
# and set other directories wrt amplxe-cl.
# ENV{INTELAMPLIFIER_INSTALL_DIR} should point to installation root of the package
#
# Imports:
#
#  IntelAmplifier::libittnotify
#  IntelAmplifier::amplxe-cl (executable)
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{INTELAMPLIFIER_INCLUDE_DIR})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
  set(INTELBINSUFFIX "64")
else()
  set(INTELBINSUFFIX "32")
endif()

if($ENV{INTELAMPLIFIER_INSTALL_DIR})
  find_path(INTELAMPLIFIER_INCLUDE_DIR ittnotify.h HINTS $ENV{INTELAMPLIFIER_INSTALL_DIR} PATH_SUFFIXES "include")
  if(INTELAMPLIFIER_INCLUDE_DIR)
    find_program(INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE amplxe-cl HINTS $ENV{INTELAMPLIFIER_INSTALL_DIR} PATH_SUFFIXES "bin${INTELBINSUFFIX}" "." )
    set(INTELAMPLIFIER_INCLUDE_DIRS "$ENV{INTELAMPLIFIER_INSTALL_DIR}/include")
    find_library(INTELAMPLIFIER_LIBITTNOTIFY ittnotify HINTS $ENV{INTELAMPLIFIER_INSTALL_DIR} PATH_SUFFIXES "lib${INTELBINSUFFIX}")
    get_filename_component(INTELAMPLIFIER_LIB_DIRS "${INTELAMPLIFIER_LIBITTNOTIFY}" DIRECTORY)
  endif()
else()
  find_program(INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE amplxe-cl HINTS $ENV{INTELAMPLIFIER_INSTALL_DIR} PATH_SUFFIXES "bin${INTELBINSUFFIX}" "." )
  if(INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE)
    get_filename_component(INTELAMPLIFIER_AMPLXE_INST_BASE "${INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE}" DIRECTORY)
    get_filename_component(INTELAMPLIFIER_AMPLXE_INST_BASE "${INTELAMPLIFIER_AMPLXE_INST_BASE}" DIRECTORY)
    find_path(INTELAMPLIFIER_INCLUDE_DIR ittnotify.h PATHS "${INTELAMPLIFIER_AMPLXE_INST_BASE}/include")
    set(INTELAMPLIFIER_INCLUDE_DIRS "${INTELAMPLIFIER_INCLUDE_DIR}")
    find_library(INTELAMPLIFIER_LIBITTNOTIFY ittnotify HINTS ${INTELAMPLIFIER_AMPLXE_INST_BASE} PATH_SUFFIXES "lib${INTELBINSUFFIX}")
    get_filename_component(INTELAMPLIFIER_LIB_DIRS "${INTELAMPLIFIER_LIBITTNOTIFY}" DIRECTORY)
  endif()
endif()

FIND_PACKAGE_HANDLE_STANDARD_ARGS(IntelAmplifier FOUND_VAR INTELAMPLIFIER_FOUND
  REQUIRED_VARS INTELAMPLIFIER_INCLUDE_DIRS INTELAMPLIFIER_LIBITTNOTIFY INTELAMPLIFIER_LIB_DIRS INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE)
mark_as_advanced(INTELAMPLIFIER_FOUND INTELAMPLIFIER_INCLUDE_DIR INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE INTELAMPLIFIER_LIBITTNOTIFY INTELAMPLIFIER_LIB_DIRS )

# Modernisation: create interface targets to link against and use
if(TARGET IntelAmplifier::libittnotify)
    return()
endif()
if(INTELAMPLIFIER_FOUND)
  add_library(IntelAmplifier::libittnotify IMPORTED INTERFACE)
  target_include_directories(IntelAmplifier::libittnotify SYSTEM INTERFACE "${INTELAMPLIFIER_INCLUDE_DIRS}")
  target_link_libraries(IntelAmplifier::libittnotify INTERFACE "${INTELAMPLIFIER_LIBITTNOTIFY}" ${CMAKE_DL_LIBS})
  add_executable(IntelAmplifier::amplxe-cl IMPORTED)
  set_target_properties(IntelAmplifier::amplxe-cl PROPERTIES IMPORTED_LOCATION ${INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE})
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: IntelAmplifier::libittnotify")
    message(STATUS "  Import executable target: IntelAmplifier::amplxe-cl")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  foreach(v IN ITEMS INTELAMPLIFIER_INCLUDE_DIR INTELAMPLIFIER_INCLUDE_DIRS
                     INTELAMPLIFIER_LIB_DIRS INTELAMPLIFIER_AMPLXE_CL_EXECUTABLE
                     INTELAMPLIFIER_LIBITTNOTIFY)
    variable_watch(${v} __deprecate_var_for_target)
  endforeach()
endif()
