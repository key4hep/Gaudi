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
# - Locate CppUnit library
# Defines:
#
#  CPPUNIT_FOUND
#  CPPUNIT_INCLUDE_DIR
#  CPPUNIT_INCLUDE_DIRS (not cached)
#  CPPUNIT_LIBRARY
#  CPPUNIT_LIBRARIES (not cached)
#
# Imports:
#
#  CppUnit::cppunit
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{CPPUNIT_INCLUDE_DIR})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

find_path(CPPUNIT_INCLUDE_DIR cppunit/Test.h)
find_library(CPPUNIT_LIBRARY NAMES cppunit)

set(CPPUNIT_INCLUDE_DIRS ${CPPUNIT_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set CPPUNIT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CppUnit DEFAULT_MSG CPPUNIT_INCLUDE_DIR CPPUNIT_LIBRARY)

mark_as_advanced(CPPUNIT_FOUND CPPUNIT_INCLUDE_DIR CPPUNIT_LIBRARY)

set(CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARY} ${CMAKE_DL_LIBS})

# Modernisation: create an interface target to link against
if(TARGET CppUnit::cppunit)
    return()
endif()
if(CPPUNIT_FOUND)
  add_library(CppUnit::cppunit IMPORTED INTERFACE)
  target_include_directories(CppUnit::cppunit SYSTEM INTERFACE "${CPPUNIT_INCLUDE_DIRS}")
  target_link_libraries(CppUnit::cppunit INTERFACE "${CPPUNIT_LIBRARIES}")
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: CppUnit::cppunit")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  foreach(v IN ITEMS CPPUNIT_INCLUDE_DIR CPPUNIT_INCLUDE_DIRS CPPUNIT_LIBRARY CPPUNIT_LIBRARIES)
    variable_watch(${v} __deprecate_var_for_target)
  endforeach()
endif()
