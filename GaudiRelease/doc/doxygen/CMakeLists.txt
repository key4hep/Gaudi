find_package(Doxygen)

if(DOXYGEN_FOUND)
  # directories used in the include search path during compilation
  set(DOXYGEN_INCLUDE_DIRS)
  foreach(package ${packages})
    set(DOXYGEN_INCLUDE_DIRS "${DOXYGEN_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/${package}")
  endforeach()

  # add the release notes of each package
  set(rel_notes_main ${CMAKE_CURRENT_BINARY_DIR}/release_notes_main.md)
  file(WRITE ${rel_notes_main} "Release Notes {#release_notes_main}
==

[Release notes of the project](release.notes.html)

Release notes of the packages:
")
  foreach(package ${packages})
    if(EXISTS ${CMAKE_SOURCE_DIR}/${package}/doc/release.notes)
      file(APPEND ${rel_notes_main} "* @subpage ${package}-notes\n")
      #configure_file(${CMAKE_SOURCE_DIR}/${package}/doc/release.notes
      #               ${CMAKE_CURRENT_BINARY_DIR}/${package}-notes.md COPYONLY)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${package}-notes.md
           "${package} {#${package}-notes}
==

\\verbinclude ${package}/doc/release.notes\n")
    endif()
  endforeach()

  # add extra files for the HTML pages
  file(GLOB project_rel_notes ${CMAKE_SOURCE_DIR}/GaudiRelease/doc/release.notes*.html)
  set(DOXYGEN_HTML_EXTRA_FILES)
  foreach(extra_file
            ${CMAKE_CURRENT_SOURCE_DIR}/issue_tracker_links.js
            ${project_rel_notes})
    set(DOXYGEN_HTML_EXTRA_FILES "${DOXYGEN_HTML_EXTRA_FILES} ${extra_file}")
  endforeach()

  # run Doxygen to generate the documentation
  add_custom_target(run-doxygen
                    COMMAND ${env_cmd} --xml ${env_xml}
                            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
                    COMMENT "Running Doxygen...")

  # 'doc' target
  add_custom_target(doc DEPENDS run-doxygen)

  option(DOXYGEN_WITH_LOCAL_MATHJAX
         "Use a local copy of MathJax instead of taking it from cdn.mathjax.org"
         FALSE)

  if(DOXYGEN_WITH_LOCAL_MATHJAX)
    # install MathJax Javascript math renderer
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/doxygen/mathjax
                       COMMAND ${CMAKE_COMMAND}
                         -DTARFILE_DEST=${CMAKE_CURRENT_BINARY_DIR}
                         -DINSTALL_DIR=${CMAKE_BINARY_DIR}/doxygen
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/get_mathjax.cmake
                       COMMENT "Installing MathJax...")
    # in principle this is not needed, but it allows to run Doxygen and install
    # MathJax at the same time
    add_custom_target(install-MathJax DEPENDS ${CMAKE_BINARY_DIR}/doxygen/mathjax)
    add_dependencies(doc install-MathJax)
    set(MATHJAX_RELPATH "../mathjax")
  else()
    set(MATHJAX_RELPATH "http://cdn.mathjax.org/mathjax/latest")
  endif()

  # massage some version numbers
  string(REGEX REPLACE "_python.*" "" Boost_version "${Boost_config_version}")
  string(REGEX REPLACE "\\." "_" Boost_url_version "${Boost_version}")
  string(REGEX REPLACE "\\..*" "" XercesC_major_version "${XercesC_config_version}")

  # copy the template files
  foreach(template Doxyfile mainpage.md externaldocs.md)
    configure_file(${template}.in ${CMAKE_CURRENT_BINARY_DIR}/${template} @ONLY)
  endforeach()

endif()
