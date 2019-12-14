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
find_projects(projects tools data/projects/SpecialConventions/CMakeLists.txt)
set_paths_from_projects(${tools} ${projects})

include(TestMacros)

assert_strequal(projects "SPECIALCONVENTIONS;DEVPROJECT;NOVERSION")

get_filename_component(base_dir data/projects ABSOLUTE)
assert_same_path(NOVERSION_ROOT_DIR "${base_dir}/NoVersion")
assert_same_path(DEVPROJECT_ROOT_DIR "${base_dir}/DevProject_v3r9")
assert_same_path(SPECIALCONVENTIONS_ROOT_DIR "${base_dir}/SpecialConventions")

assert_same_path_list(CMAKE_PREFIX_PATH "${SPECIALCONVENTIONS_ROOT_DIR};${DEVPROJECT_ROOT_DIR};${NOVERSION_ROOT_DIR}/InstallArea/${BINARY_TAG}/cmake;${NOVERSION_ROOT_DIR};${base_dir};${base_dir}_cmt")
assert_same_path_list(CMAKE_MODULE_PATH "${SPECIALCONVENTIONS_ROOT_DIR};${DEVPROJECT_ROOT_DIR};${NOVERSION_ROOT_DIR}/InstallArea/${BINARY_TAG}/cmake;${NOVERSION_ROOT_DIR};${CMAKE_CURRENT_LIST_DIR};..;.")
