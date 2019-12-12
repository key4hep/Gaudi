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
find_projects(projects tools data/projects/TOOLUSER/TOOLUSER_v1r0/CMakeLists.txt)
set_paths_from_projects(${tools} ${projects})

include(TestMacros)

assert_strequal(projects "TOOLUSER")

get_filename_component(base_dir data/projects ABSOLUTE)
assert_same_path(TOOLUSER_ROOT_DIR "${base_dir}/TOOLUSER/TOOLUSER_v1r0")

assert_strequal(tools "ATool;AnotherTool")
assert_same_path(ATool_DIR "${base_dir}/ATool/1.5.7")
assert_same_path(AnotherTool_DIR "${base_dir}/AnotherTool/0.0.0/${BINARY_TAG}")

assert_same_path_list(CMAKE_PREFIX_PATH "${ATool_DIR};${AnotherTool_DIR};${TOOLUSER_ROOT_DIR}/AtlasExternalsRelease/cmake;${TOOLUSER_ROOT_DIR};${base_dir};${base_dir}_cmt")
assert_same_path_list(CMAKE_MODULE_PATH "${ATool_DIR};${AnotherTool_DIR};${TOOLUSER_ROOT_DIR}/AtlasExternalsRelease/cmake;${TOOLUSER_ROOT_DIR};${CMAKE_CURRENT_LIST_DIR};..;.")
