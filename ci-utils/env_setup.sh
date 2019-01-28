# default values for some variables defined in .gitlab-ci.yml
: ${TARGET_BRANCH:=master}

: ${BINARY_TAG:=x86_64-centos7-gcc8-opt}
: ${BUILDDIR:=build.${BINARY_TAG}}
: ${TESTS_REPORT:=test_report}

: ${LCG_hostos:=${BINARY_TAG%-*-*}}
: ${LCG_release_area:=/cvmfs/sft.cern.ch/lcg/releases}
: ${LCG_contrib:=/cvmfs/sft.cern.ch/lcg/contrib}

: ${CCACHE_VERSION:=3.3.4-e92e5}

# initial required environment
# - get Python, nosetests and xenv from LHCb installation
source /cvmfs/lhcb.cern.ch/lib/var/lib/LbEnv/prod/${LCG_hostos}/bin/activate
# - add CMake ann Ninja-build
export PATH=${LCG_contrib}/CMake/3.11.1/Linux-x86_64/bin:${LCG_contrib}/ninja/1.7.2.gcc0ea.kitware.dyndep-1/x86_64-slc6:${PATH}
# - use LHCb compiler wrappers
export PATH=/cvmfs/lhcb.cern.ch/lib/bin/${LCG_hostos}:${PATH}
# - enable optional VTune based profiler
export CMAKE_PREFIX_PATH=${LCG_release_area}:/cvmfs/projects.cern.ch/intelsw/psxe/linux/x86_64/2017/vtune_amplifier_xe

export CCACHE_DIR=${PWD}/.ccache
unset CMTCONFIG
