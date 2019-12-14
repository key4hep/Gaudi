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
# Download Doxygen tags from cppreference.com
set(tags_url http://upload.cppreference.com/mwiki/images/f/f8/cppreference-doxygen-web.tag.xml)

message(STATUS "Downloading Doxygen tags from cppreference.com...")
file(DOWNLOAD ${tags_url} ${DEST_DIR}/cppreference-doxygen-web.tag.xml)

message(STATUS "Doxygen tags downloaded.")
