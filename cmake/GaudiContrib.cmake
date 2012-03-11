#---------------------------------------------------------------------------------------------------
#---REFLEX_GENERATE_DICTIONARY( dictionary headerfiles selectionfile OPTIONS opt1 opt2 ...)
#---------------------------------------------------------------------------------------------------
macro(REFLEX_GENERATE_DICTIONARY dictionary _headerfiles _selectionfile)
  find_package(GCCXML)
  if(NOT GCCXML)
    message(FATAL_ERROR "GCCXML not found, cannot generate Reflex dictionaries.")
  endif()
  find_package(ROOT)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "OPTIONS" ${ARGN})
  if( IS_ABSOLUTE ${_selectionfile})
   set( selectionfile ${_selectionfile})
  else()
   set( selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${_selectionfile})
  endif()
  if( IS_ABSOLUTE ${_headerfiles})
    set( headerfiles ${_headerfiles})
  else()
    set( headerfiles ${CMAKE_CURRENT_SOURCE_DIR}/${_headerfiles})
  endif()

  set(gensrcdict ${dictionary}_dict.cpp)

  if(MSVC)
    set(gccxmlopts "--gccxmlopt=\"--gccxml-compiler cl\"")
  else()
    #set(gccxmlopts "--gccxmlopt=\'--gccxml-cxxflags -m64 \'")
    set(gccxmlopts)
  endif()

  set(rootmapname ${dictionary}Dict.rootmap)
  if (NOT WIN32)
    set(libprefix lib)
  else()
    set(libprefix)
  endif()
  set(rootmapopts --rootmap=${rootmapname} --rootmap-lib=${libprefix}${dictionary}Dict)

  set(include_dirs -I${CMAKE_CURRENT_SOURCE_DIR})
  get_directory_property(_incdirs INCLUDE_DIRECTORIES)
  foreach( d ${_incdirs})
   set(include_dirs ${include_dirs} -I${d})
  endforeach()

  get_directory_property(_defs COMPILE_DEFINITIONS)
  foreach( d ${_defs})
   set(definitions ${definitions} -D${d})
  endforeach()

  get_filename_component(GCCXML_home ${GCCXML} PATH)
  add_custom_command(
    OUTPUT ${gensrcdict} ${rootmapname}
    COMMAND ${ROOT_genreflex_cmd}
    ARGS ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
         --gccxmlpath=${GCCXML_home} ${ARG_OPTIONS} ${include_dirs} ${definitions}
    DEPENDS ${headerfiles} ${selectionfile})

  # Creating this target at ALL level enables the possibility to generate dictionaries (genreflex step)
  # well before the dependent libraries of the dictionary are build
  add_custom_target(${dictionary}Gen ALL DEPENDS ${gensrcdict} ${rootmapname})

endmacro()

#---------------------------------------------------------------------------------------------------
#---REFLEX_BUILD_DICTIONARY( dictionary headerfiles selectionfile OPTIONS opt1 opt2 ...  LIBRARIES lib1 lib2 ... )
#---------------------------------------------------------------------------------------------------
function(REFLEX_BUILD_DICTIONARY dictionary headerfiles selectionfile )
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LIBRARIES;OPTIONS" ${ARGN})
  REFLEX_GENERATE_DICTIONARY(${dictionary} ${headerfiles} ${selectionfile} OPTIONS ${ARG_OPTIONS})
  add_library(${dictionary}Dict MODULE ${gensrcdict})
  target_link_libraries(${dictionary}Dict ${ARG_LIBRARIES} ${ROOT_Reflex_LIBRARY})
  # ensure that *Gen and *Dict are not built at the same time
  add_dependencies(${dictionary}Dict ${dictionary}Gen)
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedDictRootmap_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname})
  set_property(GLOBAL APPEND PROPERTY MergedDictRootmap_DEPENDS ${dictionary}Gen)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${dictionary}Dict LIBRARY DESTINATION ${lib})
endfunction()

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
		             ${ROOT_genmap_cmd} -i ${libname} -o ${rootmapfile}
                     DEPENDS ${library})
  add_custom_target(${library}Rootmap ALL DEPENDS ${rootmapfile})
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedRootmap_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${library}.rootmap)
  set_property(GLOBAL APPEND PROPERTY MergedRootmap_DEPENDS ${library}Rootmap)
endfunction()
