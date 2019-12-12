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
# Module locating the C++ Guidelines Support Library headers.
#
# Defines:
#  - CPPGSL_FOUND
#  - CPPGSL_INCLUDE_DIR
#  - CPPGSL_INCLUDE_DIRS (not cached)

# Look for the header directory:
find_path( CPPGSL_INCLUDE_DIR
   NAMES gsl/gsl
   HINTS $ENV{CPPGSL_ROOT_DIR} ${CPPGSL_ROOT_DIR} )
set( CPPGSL_INCLUDE_DIRS ${CPPGSL_INCLUDE_DIR} )

# Handle the regular find_package arguments:
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( cppgsl DEFAULT_MSG CPPGSL_INCLUDE_DIR )

# Mark the cached variables as "advanced":
mark_as_advanced( CPPGSL_FOUND CPPGSL_INCLUDE_DIR )
