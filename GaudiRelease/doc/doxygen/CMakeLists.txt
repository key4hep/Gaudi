find_package(Doxygen)

if(DOXYGEN_FOUND)
  # directories used in the include search path during compilation
  set(DOXYGEN_INCLUDE_DIRS)
  foreach(package ${packages})
    set(DOXYGEN_INCLUDE_DIRS "${DOXYGEN_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/${package}")
  endforeach()

  configure_file(Doxyfile.in ${CMAKE_CONFIG_OUTPUT_DIRECTORY}/doxygen/Doxyfile @ONLY)
  configure_file(mainpage.md.in ${CMAKE_CONFIG_OUTPUT_DIRECTORY}/doxygen/mainpage.md @ONLY)

  add_custom_target(doc
                    COMMAND ${env_cmd} --xml ${env_xml}
                            ${DOXYGEN_EXECUTABLE} ${CMAKE_CONFIG_OUTPUT_DIRECTORY}/doxygen/Doxyfile)
endif()
