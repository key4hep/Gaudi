set(CMAKE_MODULE_PATH .. .)

include(UseHEPTools)

use_heptools(123)

include(TestMacros)

get_filename_component(base_dir data/heptools/LCG_123 ABSOLUTE)

assert_strequal(cxx_compiler_names lcg-g++-4.8.2)

assert_strequal(name1_config_version 1)
assert_same_path(name1_home ${base_dir}/name1/1/bin)

assert_strequal(name2_config_version 2)
assert_same_path(name2_home ${base_dir}/name2/2/bin)

assert_strequal(name3_config_version 3)
assert_same_path(name3_home /usr/local/name3/bin)

assert_same_path_list(CMAKE_PREFIX_PATH "${name1_home};${name2_home};${name3_home}")
