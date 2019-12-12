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
# - Locate tcmalloc library
# Defines:
#
#  TCMALLOC_FOUND
#  TCMALLOC_INCLUDE_DIR
#  TCMALLOC_INCLUDE_DIRS (not cached)
#  TCMALLOC_tcmalloc_LIBRARY
#  TCMALLOC_profiler_LIBRARY
#  TCMALLOC_LIBRARIES (not cached)
#  TCMALLOC_LIBRARY_DIRS (not cached)
#  PPROF_EXECUTABLE

find_path(TCMALLOC_INCLUDE_DIR NAMES gperftools/tcmalloc.h google/tcmalloc.h)
foreach(component tcmalloc profiler)
  find_library(TCMALLOC_${component}_LIBRARY NAMES ${component})
  mark_as_advanced(TCMALLOC_${component}_LIBRARY)
endforeach()

find_program(PPROF_EXECUTABLE NAMES pprof
             HINTS ${TCMALLOC_INCLUDE_DIR}/../bin)

set(TCMALLOC_INCLUDE_DIRS ${TCMALLOC_INCLUDE_DIR})
set(TCMALLOC_LIBRARIES ${TCMALLOC_tcmalloc_LIBRARY} ${TCMALLOC_profiler_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set TCMALLOC_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(tcmalloc DEFAULT_MSG TCMALLOC_INCLUDE_DIR TCMALLOC_LIBRARIES)

mark_as_advanced(TCMALLOC_FOUND TCMALLOC_INCLUDE_DIR PPROF_EXECUTABLE)

if(TCMALLOC_tcmalloc_LIBRARY)
  get_filename_component(TCMALLOC_LIBRARY_DIRS ${TCMALLOC_tcmalloc_LIBRARY} PATH)
elseif(TCMALLOC_profiler_LIBRARY)
  get_filename_component(TCMALLOC_LIBRARY_DIRS ${TCMALLOC_profiler_LIBRARY} PATH)
endif()
