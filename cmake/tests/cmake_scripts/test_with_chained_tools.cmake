set(CMAKE_MODULE_PATH .. .)

include(GaudiToolchainMacros)

init()
find_projects(projects tools data/projects/CHAINTOOL/CHAINTOOL_v1r0/CMakeLists.txt)

include(TestMacros)

assert_strequal(projects "CHAINTOOL;TOOLUSER")

get_filename_component(base_dir data/projects ABSOLUTE)
assert_same_path(TOOLUSER_ROOT_DIR "${base_dir}/TOOLUSER/TOOLUSER_v1r0")
assert_same_path(CHAINTOOL_ROOT_DIR "${base_dir}/CHAINTOOL/CHAINTOOL_v1r0")

assert_strequal(tools "ThirdTool;ATool;AnotherTool")
assert_same_path(ATool_DIR "${base_dir}/ATool/1.5.7")
assert_same_path(AnotherTool_DIR "${base_dir}/AnotherTool/0.0.0/${BINARY_TAG}")
assert_same_path(ThirdTool_DIR "${base_dir}/ThirdTool/3.3")
