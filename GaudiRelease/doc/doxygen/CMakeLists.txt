find_package(Doxygen)

if(DOXYGEN_FOUND)
  # directories used in the include search path during compilation
  set(DOXYGEN_INCLUDE_DIRS)
  foreach(package ${packages})
    set(DOXYGEN_INCLUDE_DIRS "${DOXYGEN_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/${package}")
  endforeach()

  foreach(template Doxyfile mainpage.md externaldocs.md)
    configure_file(${template}.in ${CMAKE_CONFIG_OUTPUT_DIRECTORY}/doxygen/${template} @ONLY)
  endforeach()

  add_custom_target(doc
                    COMMAND ${env_cmd} --xml ${env_xml}
                            ${DOXYGEN_EXECUTABLE} ${CMAKE_CONFIG_OUTPUT_DIRECTORY}/doxygen/Doxyfile)
endif()
