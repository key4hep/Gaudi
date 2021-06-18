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

# add the release notes of each package
set(rel_notes_main ${CMAKE_CURRENT_BINARY_DIR}/release_notes_main.md)
file(WRITE ${rel_notes_main} "Release Notes {#release_notes_main}
==

[Release notes of the project](release.notes.html)

Release notes of the packages:
")

# Write all 
foreach(package IN ITEMS GaudiPluginService GaudiPolicy GaudiKernel GaudiCoreSvc
                         GaudiUtils Gaudi GaudiAlg GaudiAud GaudiCommonSvc
                         GaudiHive GaudiMonitor GaudiMP GaudiPartProp
                         GaudiProfiling GaudiPython GaudiRelease GaudiSvc
                         PartPropSvc RootCnv RootHistCnv GaudiExamples)
  if(EXISTS ${PROJECT_SOURCE_DIR}/${package}/doc/release.notes)
    file(APPEND ${rel_notes_main} "* @subpage ${package}-notes\n")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${package}-notes.md
          "${package} {#${package}-notes}
==

\\verbinclude ${package}/doc/release.notes\n")
  endif()
endforeach()

# add extra files for the HTML pages
file(GLOB project_rel_notes ${CMAKE_CURRENT_SOURCE_DIR}/doc/release.notes*.html)
set(DOXYGEN_HTML_EXTRA_FILES "${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/issue_tracker_links.js;${project_rel_notes}")
string(REPLACE ";" " " DOXYGEN_HTML_EXTRA_FILES "${DOXYGEN_HTML_EXTRA_FILES}")

if(TARGET Doxygen::doxygen)
  # run Doxygen to generate the documentation
  add_custom_target(run-doxygen
                    COMMAND run $<TARGET_FILE:Doxygen::doxygen> ${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen/Doxyfile
                    COMMENT "Running Doxygen...")

  # 'doc' target
  add_custom_target(doc DEPENDS run-doxygen)
endif()

option(DOXYGEN_WITH_LOCAL_MATHJAX
        "Use a local copy of MathJax instead of taking it from cdn.mathjax.org"
        FALSE)

if(DOXYGEN_WITH_LOCAL_MATHJAX)
  # install MathJax Javascript math renderer
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../doxygen/mathjax
                      COMMAND ${CMAKE_COMMAND}
                        -DTARFILE_DEST=${CMAKE_CURRENT_BINARY_DIR}
                        -DINSTALL_DIR=${CMAKE_CURRENT_BINARY_DIR}/../doxygen
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_mathjax.cmake
                      COMMENT "Installing MathJax...")
  # in principle this is not needed, but it allows to run Doxygen and install
  # MathJax at the same time
  add_custom_target(install-MathJax DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../doxygen/mathjax)
  add_dependencies(run-doxygen install-MathJax)
  set(MATHJAX_RELPATH "../mathjax")
else()
  set(MATHJAX_RELPATH "https://cdn.mathjax.org/mathjax/latest")
endif()

option(DOXYGEN_WITH_CPPREFERENCE_LINKS
        "Link C++ standard library classes to http://cppreference.com documentation."
        TRUE)

if(DOXYGEN_WITH_CPPREFERENCE_LINKS)
  # download Doxygen tags from cppreference.com
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml
                      COMMAND ${CMAKE_COMMAND}
                        -D DEST_DIR=${CMAKE_CURRENT_BINARY_DIR}
                        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_cppreference_tags.cmake
                      COMMENT "Getting cppreference.com doxygen tags...")
  add_custom_target(get-ccpreference-tags DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml)
  add_dependencies(run-doxygen get-ccpreference-tags)
  set(DOXYGEN_TAGFILES
      "${DOXYGEN_TAGFILES} \"${CMAKE_CURRENT_BINARY_DIR}/cppreference-doxygen-web.tag.xml=http://en.cppreference.com/w/\"")
endif()

# define the project version to show in the mainpage
if(DEFINED ENV{CI_COMMIT_REF_NAME})
  string(SUBSTRING "$ENV{CI_COMMIT_SHA}" 0 8 CI_COMMIT_SHA)
  set(DOC_PROJECT_VERSION "$ENV{CI_COMMIT_REF_NAME} (${CI_COMMIT_SHA})")
elseif(EXISTS "${PROJECT_SOURCE_DIR}/.git")
  execute_process(COMMAND git describe --tags
                  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                  OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git rev-parse HEAD
                  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                  OUTPUT_VARIABLE GIT_COMMIT_SHA
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  string(SUBSTRING "${GIT_COMMIT_SHA}" 0 8 GIT_COMMIT_SHA)
  set(DOC_PROJECT_VERSION "${GIT_DESCRIBE_VERSION} (${GIT_COMMIT_SHA})")
else()
  set(DOC_PROJECT_VERSION "${PROJECT_VERSION}")
endif()

# copy the template files
foreach(template IN ITEMS Doxyfile mainpage.md externaldocs.md)
  configure_file(doc/doxygen/${template}.in ${CMAKE_CURRENT_BINARY_DIR}/doc/doxygen/${template} @ONLY)
endforeach()

# the cmake directory is not scanned
configure_file(${PROJECT_SOURCE_DIR}/cmake/README.md
               ${CMAKE_CURRENT_BINARY_DIR}/cmake_doc/README.md COPYONLY)
