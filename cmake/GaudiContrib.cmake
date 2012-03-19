#---------------------------------------------------------------------------------------------------
#---SET_RUNTIME_PATH( var [LD_LIBRARY_PATH | PATH] )
#---------------------------------------------------------------------------------------------------
function(SET_RUNTIME_PATH var pathname)
  set( dirs ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR})
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  get_property(found_projects GLOBAL PROPERTY PROJECTS_FOUND)
  foreach( package ${found_projects} ${found_packages} )
     foreach( env ${${package}_environment})
         if(env MATCHES "^${pathname}[+]=.*")
            string(REGEX REPLACE "^${pathname}[+]=(.+)" "\\1"  val ${env})
            set(dirs ${dirs} ${val})
         endif()
     endforeach()
  endforeach()
  if(WIN32)
    string(REPLACE ";" "[:]" dirs "${dirs}")
  else()
    string(REPLACE ";" ":" dirs "${dirs}")
  endif()
  set(${var} "${dirs}" PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_ROOTMAP( library )
#---------------------------------------------------------------------------------------------------
function(GAUDI_GENERATE_ROOTMAP library)
  find_package(ROOT QUIET)
  set(rootmapfile ${library}.rootmap)

  if(TARGET ${library})
    get_property(libname TARGET ${library} PROPERTY LOCATION)
  else()
    set(libname ${library})
  endif()
  add_custom_command(OUTPUT ${rootmapfile}
                     COMMAND ${env_cmd}
                       -p ${ld_library_path}=${ROOT_LIBRARY_DIRS}
		             ${ROOT_genmap_CMD} -i ${libname} -o ${rootmapfile}
                     DEPENDS ${library})
  add_custom_target(${library}Rootmap ALL DEPENDS ${rootmapfile})
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedRootmap_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${library}.rootmap)
  set_property(GLOBAL APPEND PROPERTY MergedRootmap_DEPENDS ${library}Rootmap)
endfunction()
