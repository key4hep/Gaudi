# default values for some variables defined in .gitlab-ci.yml
: ${TARGET_BRANCH:=master}

: ${BINARY_TAG:=x86_64-centos7-gcc7-opt}
: ${BUILDDIR:=build.${BINARY_TAG}}
: ${TESTS_REPORT:=test_report}

: ${LCG_hostos:=${BINARY_TAG%-*-*}}
: ${LCG_release_area:=/cvmfs/sft.cern.ch/lcg/releases}
: ${LCG_contrib:=/cvmfs/sft.cern.ch/lcg/contrib}

: ${CLANG_FORMAT_VERSION:=3.9}
: ${CCACHE_VERSION:=3.3.4-e92e5}


# initial required environment
export PATH=/cvmfs/lhcb.cern.ch/lib/contrib/CMake/3.11.0/Linux-x86_64/bin:${LCG_contrib}/ninja/1.7.1/x86_64-slc6:${LCG_release_area}/Python/2.7.13-597a5/${BINARY_TAG}/bin${PATH:+:$PATH}
export PATH=/cvmfs/lhcb.cern.ch/lib/lhcb/LBSCRIPTS/dev/InstallArea/scripts:${PATH}
export PYTHONPATH=/cvmfs/lhcb.cern.ch/lib/lhcb/LBSCRIPTS/dev/InstallArea/python${PYTHONPATH:+:$PYTHONPATH}
export CMAKE_PREFIX_PATH=${LCG_release_area}:/cvmfs/projects.cern.ch/intelsw/psxe/linux/x86_64/2017/vtune_amplifier_xe
export CCACHE_DIR=${PWD}/.ccache

# special environment required for LCG binutils
export PATH=${LCG_contrib}/bintuils/2.28/${LCG_hostos}/bin:${PATH}
export LD_LIBRARY_PATH=${LCG_contrib}/bintuils/2.28/${LCG_hostos}/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
unset CMTCONFIG
