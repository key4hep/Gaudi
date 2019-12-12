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
find_projects(projects tools data/projects/DERIVED/DERIVED_v1r2/CMakeLists.txt)
set_paths_from_projects(${tools} ${projects})

include(TestMacros)

assert_strequal(projects "DERIVED;BASE")

get_filename_component(base_dir data/projects ABSOLUTE)
assert_same_path(BASE_ROOT_DIR "${base_dir}/BASE/BASE_v1r0")
assert_same_path(DERIVED_ROOT_DIR "${base_dir}/DERIVED/DERIVED_v1r2")

assert_same_path_list(CMAKE_PREFIX_PATH "${DERIVED_ROOT_DIR};${BASE_ROOT_DIR}/cmake;${BASE_ROOT_DIR};${base_dir};${base_dir}_cmt")
assert_same_path_list(CMAKE_MODULE_PATH "${DERIVED_ROOT_DIR};${BASE_ROOT_DIR}/cmake;${BASE_ROOT_DIR};${CMAKE_CURRENT_LIST_DIR};..;.")
