#---------------------------------------------------------------------------------------------------
#  GaudiPolicy.cmake   (General flags and options for GAUDI projects -------------------------------
#---------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.4.6)
cmake_policy(SET CMP0003 NEW) # See "cmake --help-policy CMP0003" for more details
cmake_policy(SET CMP0011 NEW) # See "cmake --help-policy CMP0011" for more details
cmake_policy(SET CMP0009 NEW) # See "cmake --help-policy CMP0009" for more details


set(lib lib)
set(bin bin)
if(WIN32)
  set(ssuffix .bat)
  set(scomment rem)
  set(libprefix "")
else()
  set(ssuffix .csh)
  set(scomment \#)
  set(libprefix lib)
endif()

set(CMAKE_VERBOSE_MAKEFILES OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_SKIP_BUILD_RPATH TRUE)
#set(CMAKE_CXX_COMPILER g++)

include(CMakeMacroParseArguments)

#---Compilation Flags--------------------------------------------------------------------------------

if(MSVC90)
  add_definitions(/wd4275 /wd4251 /wd4351)
  add_definitions(-DBOOST_ALL_DYN_LINK -DBOOST_ALL_NO_LIB)
  add_definitions(/nologo)
  set(CMAKE_CXX_FLAGS_DEBUG "/D_NDEBUG /MD /Zi /Ob0 /Od /RTC1")
  if(GAUDI_CMT_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_C_FLAGS_RELEASE "/O2")
  endif()
else()
  set(CMAKE_CXX_FLAGS "-Dunix -pipe -ansi -Wall -Wextra -pthread  -Wno-deprecated -Wwrite-strings -Wpointer-arith -Wno-long-long")
  if(GAUDI_CMT_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    set(CMAKE_C_FLAGS_RELEASE "-O2")
  endif()
  add_definitions(-D_GNU_SOURCE)
endif()

if (CMAKE_SYSTEM_NAME MATCHES Linux)
  set(CMAKE_CXX_FLAGS "-Dlinux ${CMAKE_CXX_FLAGS}")
endif()


#---Gaudi Build Options---------------------------------------------------------
# Build options that map to compile time features
#
option(GAUDI_V21
       "disable backward compatibility hacks (implies all G21_* options)"
       OFF)
option(G21_HIDE_SYMBOLS
       "enable explicit symbol visibility on gcc-4"
       OFF)
option(G21_NEW_INTERFACES
       "disable backward-compatibility hacks in IInterface and InterfaceID"
       OFF)
option(G21_NO_ENDREQ
       "disable the 'endreq' stream modifier (use 'endmsg' instead)"
       OFF)
option(G21_NO_DEPRECATED
       "remove deprecated methods and functions"
       OFF)
option(G22_NEW_SVCLOCATOR
       "use (only) the new interface of the ServiceLocator"
       OFF)
option(GAUDI_V22
       "enable some API extensions"
       OFF)

option(GAUDI_CMT_RELEASE
       "use CMT deafult release flags instead of the CMake ones"
       OFF)

# Use the options
if ((GAUDI_V21 OR G21_HIDE_SYMBOLS) AND (comp MATCHES gcc4))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
endif()

if(NOT GAUDI_V21)
  if(GAUDI_V22)
    add_definitions(-DGAUDI_V22_API)
  else()
    add_definitions(-DGAUDI_V20_COMPAT)
  endif()
  # special case
  if(G21_HIDE_SYMBOLS AND (comp MATCHES gcc4))
    add_definitions(-DG21_HIDE_SYMBOLS)
  endif()
  #
  foreach (feature G21_NEW_INTERFACES G21_NO_ENDREQ G21_NO_DEPRECATED G22_NEW_SVCLOCATOR)
    if (${feature})
      add_definitions(-D${feature})
    endif()
  endforeach()
endif()

if(MSVC90)
  if(GAUDI_CMT_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
    set(CMAKE_C_FLAGS_RELEASE "/O2")
  endif()
else()
  if(GAUDI_CMT_RELEASE)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2")
    set(CMAKE_C_FLAGS_RELEASE "-O2")
  endif()
endif()

#---Link shared flags--------------------------------------------------------------------------------
if (CMAKE_SYSTEM_NAME MATCHES Linux)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000")
  set(CMAKE_MODULE_LINKER_FLAGS "-Wl,--as-needed -Wl,--no-undefined  -Wl,-z,max-page-size=0x1000")
endif()

if(APPLE)
   set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -flat_namespace -single_module -undefined dynamic_lookup")
   set(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS} -flat_namespace -single_module -undefined dynamic_lookup")
endif()

find_package(PythonInterp)


#--- commands required to build cached variable
# (python scripts are located as such but run through python)
set(hints ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts ${CMAKE_SOURCE_DIR}/GaudiKernel/scripts)

find_program(env_cmd env.py HINTS ${hints})
set(env_cmd ${PYTHON_EXECUTABLE} ${env_cmd})

find_program(merge_cmd merge_files.py HINTS ${hints})
set(merge_cmd ${PYTHON_EXECUTABLE} ${merge_cmd} --no-stamp)

find_program(versheader_cmd createProjVersHeader.py HINTS ${hints})
set(versheader_cmd ${PYTHON_EXECUTABLE} ${versheader_cmd})

find_program(genconfuser_cmd genconfuser.py HINTS ${hints})
set(genconfuser_cmd ${PYTHON_EXECUTABLE} ${genconfuser_cmd})

find_program(zippythondir_cmd ZipPythonDir.py HINTS ${hints})
set(zippythondir_cmd ${PYTHON_EXECUTABLE} ${zippythondir_cmd})


if(CMAKE_PROJECT_NAME STREQUAL GAUDI)
  set(genconf_cmd ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/genconf.exe)
  set(genwindef_cmd ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/genwindef.exe)
  set(gaudirun ${CMAKE_SOURCE_DIR}/Gaudi/scripts/gaudirun.py)
else()
  set(genconf_cmd ${GAUDI_binaryarea}/bin/genconf.exe)
  set(genwindef_cmd ${GAUDI_binaryarea}/bin/genwindef.exe)
  set(GAUDI_SOURCE_DIR ${GAUDI_installation})
  set(gaudirun ${GAUDI_installarea}/scripts/gaudirun.py)
endif()


#---------------------------------------------------------------------------------------------------
#---REFLEX_GENERATE_DICTIONARY( dictionary headerfiles selectionfile OPTIONS opt1 opt2 ...)
#---------------------------------------------------------------------------------------------------
macro(REFLEX_GENERATE_DICTIONARY dictionary _headerfiles _selectionfile)
  find_package(GCCXML)
  find_package(ROOT)
  PARSE_ARGUMENTS(ARG "OPTIONS" "" ${ARGN})
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

  add_custom_command(
    OUTPUT ${gensrcdict} ${rootmapname}
    COMMAND ${ROOT_genreflex_cmd}
    ARGS ${headerfiles} -o ${gensrcdict} ${gccxmlopts} ${rootmapopts} --select=${selectionfile}
         --gccxmlpath=${GCCXML_home}/bin ${ARG_OPTIONS} ${include_dirs} ${definitions}
    DEPENDS ${headerfiles} ${selectionfile})

  # Creating this target at ALL level enables the possibility to generate dictionaries (genreflex step)
  # well before the dependent libraries of the dictionary are build
  add_custom_target(${dictionary}Gen ALL DEPENDS ${gensrcdict} ${rootmapname})

endmacro()

#---------------------------------------------------------------------------------------------------
#---REFLEX_BUILD_DICTIONARY( dictionary headerfiles selectionfile OPTIONS opt1 opt2 ...  LIBRARIES lib1 lib2 ... )
#---------------------------------------------------------------------------------------------------
function(REFLEX_BUILD_DICTIONARY dictionary headerfiles selectionfile )
  PARSE_ARGUMENTS(ARG "LIBRARIES;OPTIONS" "" ${ARGN})
  REFLEX_GENERATE_DICTIONARY(${dictionary} ${headerfiles} ${selectionfile} OPTIONS ${ARG_OPTIONS})
  add_library(${dictionary}Dict MODULE ${gensrcdict})
  target_link_libraries(${dictionary}Dict ${ARG_LIBRARIES} ${ROOT_Reflex_LIBRARY})
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedDictRootmap_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname})
  set_property(GLOBAL APPEND PROPERTY MergedDictRootmap_DEPENDS ${dictionary}Gen)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${dictionary}Dict LIBRARY DESTINATION ${lib})
endfunction()

#---------------------------------------------------------------------------------------------------
#---SET_RUNTIME_PATH( var [LD_LIBRARY_PATH | PATH] )
#---------------------------------------------------------------------------------------------------
function( SET_RUNTIME_PATH var pathname)
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
  find_package(ROOT)
  set(rootmapfile ${library}.rootmap)

  if(WIN32)
    set(fulllibname ${library})
  else()
    set(fulllibname lib${library}.so)
  endif()
  SET_RUNTIME_PATH(path ${ld_library_path})
  add_custom_command( OUTPUT ${rootmapfile}
                      COMMAND ${env_cmd}
                        -p ${ld_library_path}=${path}
                        -p ${ld_library_path}=.
		              ${ROOT_genmap_cmd} -i ${fulllibname} -o ${rootmapfile}
                      DEPENDS ${library} )
  add_custom_target(${library}Rootmap ALL DEPENDS ${rootmapfile})
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedRootmap_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${library}.rootmap)
  set_property(GLOBAL APPEND PROPERTY MergedRootmap_DEPENDS ${library}Rootmap)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_CONFIGURABLES( library )
#---------------------------------------------------------------------------------------------------
function(GAUDI_GENERATE_CONFIGURABLES library)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(library_preload)  # TODO....
  set(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})
  file(MAKE_DIRECTORY ${outdir})
  set(confModuleName GaudiKernel.Proxy)
  set(confDefaultName Configurable.DefaultName)
  set(confAlgorithm ConfigurableAlgorithm)
  set(confAlgTool ConfigurableAlgTool)
  set(confAuditor ConfigurableAuditor)
  set(confService ConfigurableService)
  SET_RUNTIME_PATH(path ${ld_library_path})
  add_custom_command(
    OUTPUT ${outdir}/${library}_confDb.py ${outdir}/${library}Conf.py ${outdir}/__init__.py
		COMMAND ${env_cmd}
                  -p ${ld_library_path}=${path}
                  -p ${ld_library_path}=.
		        ${genconf_cmd} ${library_preload} -o ${outdir} -p ${package}
				--configurable-module=${confModuleName}
				--configurable-default-name=${confDefaultName}
				--configurable-algorithm=${confAlgorithm}
				--configurable-algtool=${confAlgTool}
				--configurable-auditor=${confAuditor}
				--configurable-service=${confService}
				-i lib${library}.so
		DEPENDS ${library} )
  add_custom_target( ${library}Conf ALL DEPENDS  ${outdir}/${library}_confDb.py )
  # Add dependencies on GaudiSvc and the genconf executable if they have to be built in the current project
  add_dependencies(${library}Conf genconf GaudiSvc)
  # Notify the project level target
  set_property(GLOBAL APPEND PROPERTY MergedConfDB_SOURCES ${outdir}/${library}_confDb.py)
  set_property(GLOBAL APPEND PROPERTY MergedConfDB_DEPENDS ${library}Conf)
  #----Installation details-------------------------------------------------------
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/genConf/
          DESTINATION python
          FILES_MATCHING PATTERN "*.py")
  GAUDI_INSTALL_PYTHON_INIT()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_GENERATE_CONFUSERDB([DEPENDS target1 target2])
#---------------------------------------------------------------------------------------------------
define_property(DIRECTORY
                PROPERTY CONFIGURABLE_USER_MODULES
                BRIEF_DOCS "ConfigurableUser modules"
                FULL_DOCS "List of Python modules containing ConfigurableUser specializations (default <package>/Config, 'None' to disable)." )
# Generate entries in the configurables database for ConfigurableUser specializations.
# By default, the python module supposed to contain ConfigurableUser's is <package>.Config,
# but different (or more) modules can be specified with the directory property
# CONFIGURABLE_USER_MODULES. If that property is set to None, there will be no
# search for ConfigurableUser's.
function(GAUDI_GENERATE_CONFUSERDB)
  # deduce the name of the package
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  get_directory_property(modules CONFIGURABLE_USER_MODULES)
  if( NOT (modules STREQUAL "None") ) # ConfUser enabled
    set(outdir ${CMAKE_CURRENT_BINARY_DIR}/genConf/${package})
    # get the optional dependencies from argument and properties
    PARSE_ARGUMENTS(ARG "DEPENDS" "" ${arguments})
    get_directory_property(PROPERTY_DEPENDS CONFIGURABLE_USER_DEPENDS)
    SET_RUNTIME_PATH(path PYTHONPATH)
    # TODO: this re-runs the genconfuser every time, because we cannot define the right dependencies
    add_custom_target(${package}ConfUserDB ALL
                      DEPENDS ${outdir}/${package}_user_confDb.py)
    if(${ARG_DEPENDS} ${PROPERTY_DEPENDS})
      add_dependencies(${package}ConfUserDB ${ARG_DEPENDS} ${PROPERTY_DEPENDS})
    endif()
    add_custom_command(OUTPUT ${outdir}/${package}_user_confDb.py
		COMMAND ${env_cmd}
                  -p PYTHONPATH=${path}
                  -p PYTHONPATH=${CMAKE_SOURCE_DIR}/GaudiKernel/python
                ${genconfuser_cmd}
		          -r ${CMAKE_CURRENT_SOURCE_DIR}/python
		          -o ${outdir}/${package}_user_confDb.py
		          ${package} ${modules})
    set_property(GLOBAL APPEND PROPERTY MergedConfDB_SOURCES ${outdir}/${package}_user_confDb.py)
    set_property(GLOBAL APPEND PROPERTY MergedConfDB_DEPENDS ${package}ConfUserDB)
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_LINKER_LIBRARY( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_LINKER_LIBRARY library)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  if(WIN32)
	add_library( ${library}-arc STATIC EXCLUDE_FROM_ALL ${lib_srcs})
    set_target_properties(${library}-arc PROPERTIES COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY )
    add_custom_command(
      OUTPUT ${library}.def
	  COMMAND ${genwindef_cmd} -o ${library}.def -l ${library} ${LIBRARY_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${library}-arc.lib
	  DEPENDS ${library}-arc genwindef)
	#---Needed to create a dummy source file to please Windows IDE builds with the manifest
	file( WRITE ${CMAKE_CURRENT_BINARY_DIR}/${library}.cpp "// empty file\n" )
    add_library( ${library} SHARED ${library}.cpp ${library}.def)
    target_link_libraries(${library} ${library}-arc ${ARG_LIBRARIES})
    set_target_properties(${library} PROPERTIES LINK_INTERFACE_LIBRARIES "${ARG_LIBRARIES}" )
  else()
    add_library( ${library} ${lib_srcs})
    set_target_properties(${library} PROPERTIES COMPILE_FLAGS -DGAUDI_LINKER_LIBRARY )
    target_link_libraries(${library} ${ARG_LIBRARIES})
  endif()
  if(TARGET ${library}Obj2doth)
    add_dependencies( ${library} ${library}Obj2doth)
  endif()
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION  ${lib})
  install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_COMPONENT_LIBRARY( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_COMPONENT_LIBRARY library)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${library} MODULE ${lib_srcs})
  GAUDI_GENERATE_ROOTMAP(${library})
  GAUDI_GENERATE_CONFIGURABLES(${library})
  target_link_libraries(${library} ${ROOT_Reflex_LIBRARY} ${ARG_LIBRARIES})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} LIBRARY DESTINATION ${lib})
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_PYTHON_MODULE( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_PYTHON_MODULE module)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(lib_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( lib_srcs ${lib_srcs} ${files})
    else()
      set( lib_srcs ${lib_srcs} ${fp})
    endif()
  endforeach()
  add_library( ${module} MODULE ${lib_srcs})
  if(win32)
    set_target_properties( ${module} PROPERTIES SUFFIX .pyd PREFIX "")
  else()
    set_target_properties( ${module} PROPERTIES SUFFIX .so PREFIX "")
  endif()
  target_link_libraries(${module} ${PYTHON_LIBRARIES} ${ARG_LIBRARIES})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${module} LIBRARY DESTINATION python/lib-dynload)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_EXECUTABLE( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_EXECUTABLE executable)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(exe_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  add_executable( ${executable} ${exe_srcs})
  target_link_libraries(${executable} ${ARG_LIBRARIES} )
  set_target_properties(${executable} PROPERTIES SUFFIX .exe)
  #----Installation details-------------------------------------------------------
  install(TARGETS ${executable} EXPORT ${CMAKE_PROJECT_NAME}Exports RUNTIME DESTINATION ${bin})
  install(EXPORT ${CMAKE_PROJECT_NAME}Exports DESTINATION cmake)

endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_UNIT_TEST( <name> source1 source2 ... LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_UNIT_TEST executable)
  PARSE_ARGUMENTS(ARG "LIBRARIES" "" ${ARGN})
  set(exe_srcs)
  foreach( fp ${ARG_DEFAULT_ARGS})
    file(GLOB files src/${fp})
    if(files)
      set( exe_srcs ${exe_srcs} ${files})
    else()
      set( exe_srcs ${exe_srcs} ${fp})
    endif()
  endforeach()
  if(BUILD_TESTS)
    add_executable( ${executable} ${exe_srcs})
    target_link_libraries(${executable} ${ARG_LIBRARIES} )
	SET_RUNTIME_PATH(path ${ld_library_path})
    add_test(${executable} ${env_cmd} -p ${ld_library_path}=${path} ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_CFG_INTDIR}/${executable}.exe )
    #----Installation details-------------------------------------------------------
    set_target_properties(${executable} PROPERTIES SUFFIX .exe)
    install(TARGETS ${executable} RUNTIME DESTINATION ${bin})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_FRAMEWORK_TEST( <name> conf1 conf2 ... ENVIRONMENT env=val ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_FRAMEWORK_TEST name)
  if(BUILD_TESTS)
    PARSE_ARGUMENTS(ARG "ENVIRONMENT" "" ${ARGN})
    foreach( optfile  ${ARG_DEFAULT_ARGS} )
      if( IS_ABSOLUTE ${optfile})
        set( optfiles ${optfiles} ${optfile})
      else()
        set( optfiles ${optfiles} ${CMAKE_CURRENT_SOURCE_DIR}/${optfile})
      endif()
    endforeach()
    add_test(${name} ${CMAKE_INSTALL_PREFIX}/scripts/testwrap${ssuffix} ${CMAKE_INSTALL_PREFIX}/setup${ssuffix} "." ${gaudirun} ${optfiles})
    set_property(TEST ${name} PROPERTY ENVIRONMENT
      LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}
      ${ARG_ENVIRONMENT})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_QMTEST_TEST( <name> TESTS qmtest1 qmtest2 ... ENVIRONMENT env=val ...)
#---------------------------------------------------------------------------------------------------
function(GAUDI_QMTEST_TEST name)
  if(BUILD_TESTS)
    PARSE_ARGUMENTS(ARG "TESTS;ENVIRONMENT" "" ${ARGN})
    foreach(arg ${ARG_TESTS})
	  set(tests ${tests} ${arg})
    endforeach()
    if( NOT tests )
      set(tests ${name})
    endif()
    GAUDI_USE_PACKAGE(QMtest)
    add_test(${name} ${CMAKE_INSTALL_PREFIX}/scripts/testwrap${ssuffix} ${CMAKE_INSTALL_PREFIX}/setup${ssuffix}
                     ${CMAKE_CURRENT_SOURCE_DIR}/tests/qmtest
                     qmtest run ${tests})
    set_property(TEST ${name} PROPERTY ENVIRONMENT
      LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}:$ENV{LD_LIBRARY_PATH}
      QMTEST_CLASS_PATH=${CMAKE_SOURCE_DIR}/GaudiPolicy/qmtest_classes
      ${ARG_ENVIRONMENT})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_HEADERS([dir1 dir2 ...])
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_HEADERS)
  if( ARGN )
    set( dirs ${ARGN} )
  else()
    get_filename_component(dirs ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  endif()
  foreach( inc ${dirs})
    install(DIRECTORY ${inc}
            DESTINATION include
            FILES_MATCHING
              PATTERN "*.h"
              PATTERN "*.icpp" )
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_MODULES( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_MODULES)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(DIRECTORY python/
          DESTINATION python
          FILES_MATCHING PATTERN "*.py")
  GAUDI_GENERATE_CONFUSERDB() # if there are Python modules, there may be ConfigurableUser's
  GAUDI_INSTALL_PYTHON_INIT()
endfunction()


#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_PYTHON_INIT( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_PYTHON_INIT)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(CODE "if (NOT EXISTS \"${CMAKE_INSTALL_PREFIX}/python/${package}/__init__.py\")
                  file(INSTALL DESTINATION \"${CMAKE_INSTALL_PREFIX}/python/${package}\"
                               TYPE FILE
                               FILES \"${GAUDI_SOURCE_DIR}/GaudiPolicy/cmt/fragments/__init__.py\"  )
                endif()" )
  GAUDI_ZIP_PYTHON_MODULES()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_ZIP_PYTHON_MODULES( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_ZIP_PYTHON_MODULES)
  install(CODE "execute_process(COMMAND  ${zippythondir_cmd} --quiet ${CMAKE_INSTALL_PREFIX}/python)")
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_SCRIPTS( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_SCRIPTS)
  install(DIRECTORY scripts/ DESTINATION scripts
          FILE_PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                           GROUP_EXECUTE GROUP_READ
          PATTERN ".svn" EXCLUDE
          PATTERN "*~" EXCLUDE
          PATTERN "*.pyc" EXCLUDE )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_INSTALL_JOBOPTIONS( )
#---------------------------------------------------------------------------------------------------
function(GAUDI_INSTALL_JOBOPTIONS)
  get_filename_component(package ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  install(FILES ${ARGN} DESTINATION jobOptions/${package})
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_PROJECT_VERSION_HEADER( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_PROJECT_VERSION_HEADER )
  set(project ${CMAKE_PROJECT_NAME})
  set(version ${${CMAKE_PROJECT_NAME}_VERSION})
  set(output  ${CMAKE_BINARY_DIR}/include/${project}_VERSION.h)
  add_custom_command(OUTPUT ${output}
                     COMMAND ${versheader_cmd} ${project} ${version} ${output})
  add_custom_target(${project}VersionHeader ALL
                    DEPENDS ${output})
  install(FILES ${output} DESTINATION include)
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_USE_PACKAGE( package )
#---------------------------------------------------------------------------------------------------
macro( GAUDI_USE_PACKAGE package )
  if( EXISTS ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt)
    include_directories( ${CMAKE_SOURCE_DIR}/${package} )
    file(READ ${CMAKE_SOURCE_DIR}/${package}/CMakeLists.txt file_contents)
    string( REGEX MATCHALL "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])+" vars ${file_contents})
    foreach( var ${vars})
      string(REGEX REPLACE "GAUDI_USE_PACKAGE[ ]*[(][ ]*([^ )])" "\\1" p ${var})
      GAUDI_USE_PACKAGE(${p})
    endforeach()
  else()
    find_package(${package})
    GET_PROPERTY(parent DIRECTORY PROPERTY PARENT_DIRECTORY)
    if(parent)
      set(${package}_environment  ${${package}_environment} PARENT_SCOPE)
    else()
      set(${package}_environment  ${${package}_environment} )
    endif()
    include_directories( ${${package}_INCLUDE_DIRS} )
    link_directories( ${${package}_LIBRARY_DIRS} )
  endif()
endmacro()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_SETUP( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_SETUP )
  GAUDI_BUILD_PROJECT_SETUP()
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  message("Found packages = ${found_packages}")
  foreach( package ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( "" ${package} "${${package}_environment}")
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_PROJECT_SETUP( )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PROJECT_SETUP )
  set( setup  ${CMAKE_BINARY_DIR}/setup${ssuffix} )
  file(WRITE  ${setup} "${scomment} ${CMAKE_PROJECT_NAME} Setup file\n")
  if(WIN32)
	file(APPEND ${setup} "@echo off\n")
    file(APPEND ${setup} "set PATH=${CMAKE_INSTALL_PREFIX}/${bin};${CMAKE_INSTALL_PREFIX}/${lib};${CMAKE_INSTALL_PREFIX}/scripts;%PATH%\n")
    file(APPEND ${setup} "set PYTHONPATH=${CMAKE_INSTALL_PREFIX}/python;%PYTHONPATH%\n")
  else()
    file(APPEND ${setup} "setenv PATH  ${CMAKE_INSTALL_PREFIX}/${bin}:${CMAKE_INSTALL_PREFIX}/scripts:\${PATH}\n")
    file(APPEND ${setup} "setenv LD_LIBRARY_PATH  ${CMAKE_INSTALL_PREFIX}/${lib}:\${LD_LIBRARY_PATH}\n")
    file(APPEND ${setup} "setenv PYTHONPATH  ${CMAKE_INSTALL_PREFIX}/python:\${PYTHONPATH}\n")
  endif()

  #----Get the setup fro each external package
  get_property(found_packages GLOBAL PROPERTY PACKAGES_FOUND)
  get_property(found_projects GLOBAL PROPERTY PROJECTS_FOUND)
  foreach( package ${found_projects} ${found_packages} )
    GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${package} "${${package}_environment}")
  endforeach()

  #---Get the setup for each package (directory)
  file(APPEND  ${setup} "\n${scomment} Standard variables for each package\n")
  file(GLOB_RECURSE cmakelist_files  ${CMAKE_SOURCE_DIR} CMakeLists.txt)
  foreach( file ${cmakelist_files} )
    GET_FILENAME_COMPONENT(path ${file} PATH)
    if (NOT path STREQUAL ${CMAKE_SOURCE_DIR})
      GET_FILENAME_COMPONENT(directory ${path} NAME)
      string(TOUPPER ${directory} DIRECTORY)
      set( ${directory}_environment ${${directory}_environment} ${DIRECTORY}ROOT=${path})
      GAUDI_BUILD_PACKAGE_SETUP( ${setup} ${directory} "${${directory}_environment}")
    endif()
  endforeach()
  #---Installation---------------------------------------------------------------------------------
  install(FILES ${setup}  DESTINATION .
                          PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
                                      GROUP_EXECUTE GROUP_READ
                                      WORLD_EXECUTE WORLD_READ )
endfunction()

#---------------------------------------------------------------------------------------------------
#---GAUDI_BUILD_PACKAGE_SETUP( setupfile package envlist )
#---------------------------------------------------------------------------------------------------
function( GAUDI_BUILD_PACKAGE_SETUP setup package envlist )
  if ( NOT setup )
    set( setup ${CMAKE_INSTALL_PREFIX}/${package}_setup.csh )
    file(WRITE  ${setup} "${scomment} Package ${package} setup file\n")
  else()
    file(APPEND  ${setup} "\n${scomment} Package ${package} setup file\n")
  endif()
  foreach( env ${envlist} )
    if(env MATCHES ".*[+]=.*")
      string(REGEX REPLACE "([^=+]+)[+]=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*[+]=(.+)" "\\1"  val ${env})
	  if(WIN32)
        file(APPEND ${setup} "set ${var}=${val};%${var}%\n")
	  else()
        file(APPEND ${setup} "if \$?${var} then\n")
        file(APPEND ${setup} "  setenv ${var} ${val}:\${${var}}\n")
        file(APPEND ${setup} "else\n")
        file(APPEND ${setup} "  setenv ${var} ${val}\n")
        file(APPEND ${setup} "endif\n")
	  endif()
    elseif ( env MATCHES ".*=.*")
      string(REGEX REPLACE "([^=+]+)=.*" "\\1" var ${env})
      string(REGEX REPLACE ".*=(.+)" "\\1"  val ${env})
	  if(WIN32)
        file(APPEND ${setup} "set ${var}=${val}\n")
	  else()
        file(APPEND ${setup} "setenv ${var} ${val}\n")
	  endif()
   endif()
  endforeach()
endfunction()
