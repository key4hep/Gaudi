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
set(CMAKE_MODULE_PATH .. .)

include(GaudiToolchainMacros)

init()
find_projects(projects tools data/toolchain/MULTI/MULTI_v1r0/CMakeLists.txt)

include(TestMacros)

assert_strequal(projects "MULTI;DERIVED;BASE")

get_filename_component(base_dir data/toolchain ABSOLUTE)
assert_same_path(BASE_ROOT_DIR "${base_dir}/BASE/BASE_v1r0")
assert_same_path(DERIVED_ROOT_DIR "${base_dir}/DERIVED/DERIVED_v1r0")
assert_same_path(MULTI_ROOT_DIR "${base_dir}/MULTI/MULTI_v1r0")

set_paths_from_projects(${projects})

assert(base_toolchain_called)
assert(derived_toolchain_called)
assert(multi_toolchain_called)

assert_strequal(entries "MULTI;DERIVED;BASE")
