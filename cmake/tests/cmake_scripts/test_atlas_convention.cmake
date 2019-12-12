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
find_projects(projects tools data/projects/ATLASTop/1.1/CMakeLists.txt)
set_paths_from_projects(${tools} ${projects})

include(TestMacros)

assert_strequal(projects "ATLASTOP;ATLASCONVENTION;FOO")

get_filename_component(base_dir data/projects ABSOLUTE)
assert_same_path(FOO_ROOT_DIR "${base_dir}/foo/bar")
assert_same_path(ATLASCONVENTION_ROOT_DIR "${base_dir}/ATLASConvention/1.2.3.4.5")
assert_same_path(ATLASTOP_ROOT_DIR "${base_dir}/ATLASTop/1.1")

assert_same_path_list(CMAKE_PREFIX_PATH "${ATLASTOP_ROOT_DIR};${ATLASCONVENTION_ROOT_DIR};${FOO_ROOT_DIR};${base_dir};${base_dir}_cmt")
assert_same_path_list(CMAKE_MODULE_PATH "${ATLASTOP_ROOT_DIR};${ATLASCONVENTION_ROOT_DIR};${FOO_ROOT_DIR};${CMAKE_CURRENT_LIST_DIR};..;.")
