gaudi_subdir(GaudiPython)

gaudi_depends_on_subdirs(GaudiAlg GaudiUtils GaudiKernel)

find_package(AIDA)
find_package(CLHEP)
find_package(PythonLibs REQUIRED)
find_package(pytools)

# Decide whether to link against AIDA:
set( aida_lib )
if( AIDA_FOUND )
   set( aida_lib AIDA )
   add_definitions( -DAIDA_FOUND )
endif()

# Decide whether to link against CLHEP:
set( clhep_lib )
if( CLHEP_FOUND )
   set( clhep_lib CLHEP )
   add_definitions( -DCLHEP_FOUND )
endif()

# The list of sources to build into the library:
set( lib_sources src/Lib/AlgDecorators.cpp src/Lib/CallbackStreamBuf.cpp
   src/Lib/Algorithm.cpp src/Lib/Helpers.cpp )
if( CLHEP_FOUND )
   list( APPEND lib_sources src/Lib/TupleDecorator.cpp )
endif()
if( AIDA_FOUND )
   list( APPEND lib_sources src/Lib/HistoDecorator.cpp )
endif()

# Hide some ROOT and Boost compile time warnings
find_package(Boost)
find_package(ROOT)
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
if( CLHEP_FOUND )
   include_directories( SYSTEM ${CLHEP_INCLUDE_DIRS} )
endif()

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiPythonLib ${lib_sources}
                  LINK_LIBRARIES GaudiAlgLib PythonLibs ${clhep_lib} ${aida_lib}
                  INCLUDE_DIRS ${clhep_lib} ${aida_lib} PythonLibs
                  PUBLIC_HEADERS GaudiPython)
gaudi_add_module(GaudiPython src/Services/*.cpp
                 LINK_LIBRARIES GaudiPythonLib GaudiUtilsLib)

#---Dictionaries------------------------------------------------------------
gaudi_add_dictionary(GaudiPython dict/kernel.h dict/selection_kernel.xml
                     LINK_LIBRARIES GaudiPythonLib GaudiUtilsLib)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
