set(CMAKE_MODULE_PATH .. .)

include(GaudiToolchainMacros)

init()
find_projects(projects tools data/projects/DIAMOND/DIAMOND_v7r7/CMakeLists.txt)

include(TestMacros)

assert_strequal(projects "DIAMOND;DERIVED;EXTRA;BASE")

get_filename_component(base_dir data/projects ABSOLUTE)
assert_same_path(BASE_ROOT_DIR "${base_dir}/BASE/BASE_v1r0")
assert_same_path(DERIVED_ROOT_DIR "${base_dir}/DERIVED/DERIVED_v1r2")
assert_same_path(EXTRA_ROOT_DIR "${base_dir}/EXTRA/EXTRA_v3r0")
assert_same_path(DIAMOND_ROOT_DIR "${base_dir}/DIAMOND/DIAMOND_v7r7")
