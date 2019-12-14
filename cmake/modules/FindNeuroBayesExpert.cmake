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
# - Locate NeuroBayesExpert library
# Defines:
#
#  NEUROBAYESEXPERT_FOUND
#  NEUROBAYESEXPERT_INCLUDE_DIR
#  NEUROBAYESEXPERT_INCLUDE_DIRS (not cached)
#  NEUROBAYESEXPERT_LIBRARIES
#  NEUROBAYESEXPERT_LIBRARY_DIRS (not cached)

find_path(NEUROBAYESEXPERT_INCLUDE_DIR NeuroBayesExpert.hh)
find_library(NEUROBAYESEXPERT_LIBRARIES NAMES NeuroBayesExpertCPP)

set(NEUROBAYESEXPERT_INCLUDE_DIRS ${NEUROBAYESEXPERT_INCLUDE_DIR})
get_filename_component(NEUROBAYESEXPERT_LIBRARY_DIRS ${NEUROBAYESEXPERT_LIBRARIES} PATH)

# handle the QUIETLY and REQUIRED arguments and set NEUROBAYESEXPERT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NeuroBayesExpert DEFAULT_MSG NEUROBAYESEXPERT_INCLUDE_DIR NEUROBAYESEXPERT_LIBRARIES)

mark_as_advanced(NEUROBAYESEXPERT_FOUND NEUROBAYESEXPERT_INCLUDE_DIR NEUROBAYESEXPERT_LIBRARIES)
