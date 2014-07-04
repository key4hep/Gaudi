# Dummy configuration for an empty subdir (used in CMT to group dependencies).
gaudi_subdir(GaudiRelease v25r3)


set(DOXYGEN_INCLUDE_DIRS)
foreach(package ${packages})
  set(DOXYGEN_INCLUDE_DIRS "${DOXYGEN_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/${package}")
endforeach()

configure_file(doc/doxygen/Doxyfile.in ${CMAKE_CONFIG_OUTPUT_DIRECTORY}/Doxyfile @ONLY)
add_custom_target(doc
                  COMMAND doxygen ${CMAKE_CONFIG_OUTPUT_DIRECTORY}/Doxyfile)
