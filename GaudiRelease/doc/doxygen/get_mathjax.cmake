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
# Download MathJax
set(MathJax_url https://github.com/mathjax/MathJax/archive/master.tar.gz)
set(MathJax_dst ${TARFILE_DEST}/MathJax.tar.gz)

message(STATUS "Downloading MathJax...")
file(DOWNLOAD ${MathJax_url} ${MathJax_dst})

message(STATUS "Unpacking MathJax...")
file(MAKE_DIRECTORY ${INSTALL_DIR})
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${TARFILE_DEST}/MathJax.tar.gz
                WORKING_DIRECTORY ${INSTALL_DIR})
file(RENAME ${INSTALL_DIR}/MathJax-master ${INSTALL_DIR}/mathjax)

message(STATUS "MathJax installed.")
