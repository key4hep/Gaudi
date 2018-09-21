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
