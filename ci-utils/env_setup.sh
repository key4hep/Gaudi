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
# default values for some variables defined in .gitlab-ci.yml
: ${TARGET_BRANCH:=master}

: ${BINARY_TAG:=x86_64-centos7-gcc9-opt}
: ${BUILDDIR:=build.${BINARY_TAG}}
: ${TESTS_REPORT:=test_report}

: ${LCG_hostos:=${BINARY_TAG%-*-*}}
: ${LCG_release_area:=/cvmfs/sft.cern.ch/lcg/releases}
: ${LCG_contrib:=/cvmfs/sft.cern.ch/lcg/contrib}

: ${CCACHE_VERSION:=3.7.1-7651f}

# initial required environment
# - get Python, nosetests and xenv from LHCb installation
source /cvmfs/lhcb.cern.ch/lib/var/lib/LbEnv/prod/${LCG_hostos}/bin/activate
# - add CMake and Ninja-build
export PATH=${LCG_contrib}/CMake/latest/Linux-x86_64/bin:${LCG_contrib}/ninja/1.7.2.gcc0ea.kitware.dyndep-1/x86_64-slc6:${PATH}
# - use LHCb compiler wrappers
export PATH=/cvmfs/lhcb.cern.ch/lib/bin/${LCG_hostos}:${PATH}
# - enable optional VTune based profiler
export CMAKE_PREFIX_PATH=${LCG_release_area}:/cvmfs/projects.cern.ch/intelsw/psxe/linux/x86_64/2019/vtune_amplifier

export CCACHE_DIR=${PWD}/.ccache
unset CMTCONFIG
