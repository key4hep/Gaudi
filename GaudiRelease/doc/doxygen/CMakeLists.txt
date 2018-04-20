cmake_minimum_required(VERSION 2.8.12)

if(std_library_path)
  # FindDoxygen.cmake needs to call the doxygen command at configuration time
  set(ENV{LD_LIBRARY_PATH} "${std_library_path}:$ENV{LD_LIBRARY_PATH}")
endif()

find_package(Doxygen)

if(DOXYGEN_FOUND)
  # make sure we have the right environment (for heptools installation)
  if(DOXYGEN_DOT_EXECUTABLE)
    get_filename_component(graphviz_bin "${DOXYGEN_DOT_EXECUTABLE}" DIRECTORY)
    set(graphviz_env -p PATH=${graphviz_bin} -p LD_LIBRARY_PATH=${graphviz_bin}/../lib)
  endif()

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
                    COMMAND ${env_cmd} --xml ${env_xml} ${graphviz_env}
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
    add_dependencies(run-doxygen install-MathJax)
    set(MATHJAX_RELPATH "../mathjax")
  else()
    set(MATHJAX_RELPATH "https://cdn.mathjax.org/mathjax/latest")
  endif()

  option(DOXYGEN_WITH_CPPREFERENCE_LINKS
         "Link C++ standard library classes to http://cppreference.com documentation."
         TRUE)

  set(DOXYGEN_TAGFILES)
  if(DOXYGEN_WITH_CPPREFERENCE_LINKS)
    # download Doxygen tags from cppreference.com
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml
                       COMMAND ${CMAKE_COMMAND}
                         -D DEST_DIR=${CMAKE_CURRENT_BINARY_DIR}
                         -P ${CMAKE_CURRENT_SOURCE_DIR}/get_cppreference_tags.cmake
                       COMMENT "Getting cppreference.com doxygen tags...")
    add_custom_target(get-ccpreference-tags DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml)
    add_dependencies(run-doxygen get-ccpreference-tags)
    set(DOXYGEN_TAGFILES
        "${DOXYGEN_TAGFILES} \"${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/\"")
  endif()

  # massage some version numbers
  string(REGEX REPLACE "_python.*" "" Boost_version "${Boost_config_version}")
  string(REGEX REPLACE "\\." "_" Boost_url_version "${Boost_version}")
  string(REGEX REPLACE "\\..*" "" XercesC_major_version "${XercesC_config_version}")

  # define the project version to show in the mainpage
  if(DEFINED ENV{CI_COMMIT_REF_NAME})
    string(SUBSTRING "$ENV{CI_COMMIT_SHA}" 0 8 CI_COMMIT_SHA)
    set(DOC_PROJECT_VERSION "$ENV{CI_COMMIT_REF_NAME} (${CI_COMMIT_SHA})")
  elseif(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    execute_process(COMMAND git describe --tags
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND git rev-parse HEAD
                    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                    OUTPUT_VARIABLE GIT_COMMIT_SHA
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(SUBSTRING "${GIT_COMMIT_SHA}" 0 8 GIT_COMMIT_SHA)
    set(DOC_PROJECT_VERSION "${GIT_DESCRIBE_VERSION} (${GIT_COMMIT_SHA})")
  else()
    set(DOC_PROJECT_VERSION "${CMAKE_PROJECT_VERSION}")
  endif()

  # copy the template files
  foreach(template Doxyfile mainpage.md externaldocs.md)
    configure_file(${template}.in ${CMAKE_CURRENT_BINARY_DIR}/${template} @ONLY)
  endforeach()

  # the cmake directory is not scanned
  foreach(doc_file README.md)
    configure_file(${CMAKE_SOURCE_DIR}/cmake/${doc_file}
                   ${CMAKE_CURRENT_BINARY_DIR}/cmake_doc/${doc_file} COPYONLY)
  endforeach()

endif()
