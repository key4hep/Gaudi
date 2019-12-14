#!/bin/bash -ex
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

# prepare directories
rm -rf tmp dest
mkdir -pv tmp dest

# work in the temporary build area
export CMAKE_PREFIX_PATH=$(pwd)/tmp:$(pwd)/../../..

cp -a A B DBASE tmp
mkdir -p tmp/A/build
(cd tmp/A/build && cmake .. && make install)

mkdir -p tmp/B/build
(cd tmp/B/build && cmake .. && make install)

# work in the destination (release) area
export CMAKE_PREFIX_PATH=$(pwd)/dest:$(pwd)/../../..

mv tmp/* dest
cp -a C dest
mkdir -p dest/C/build
(cd dest/C/build && cmake ..)
