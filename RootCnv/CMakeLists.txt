require(GaudiKernel GaudiUtils)

find_package(ROOT)

gaudi_install_headers(RootCnv)

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS __POOL_COMPATIBILITY)

gaudi_linker_library(RootCnvLib
                     *.cpp
                     LIBRARIES GaudiKernel GaudiUtilsLib
                               ${ROOT_RIO_LIBRARY} ${ROOT_Tree_LIBRARY}
                               ${ROOT_TreePlayer_LIBRARY} ${ROOT_MathCore_LIBRARY}
                     USE_HEADERS GaudiKernel GaudiUtilsLib)

gaudi_component_library(RootCnv
                        components/Components.cpp
                        LIBRARIES RootCnvLib)

# Note: the set_property above is needed because that's the way REFLEX_BUILD_LIBRARIES
#       takes the "-D" flags, and it make the following lines unneeded.
#set_property(TARGET RootCnvLib RootCnv RootCnvDict
#             APPEND PROPERTY COMPILE_DEFINITIONS __POOL_COMPATIBILITY)

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(RootCnv dict/RootCnv_dict.h  dict/RootCnv_dict.xml LIBRARIES RootCnvLib)

add_executable(gaudi_merge merge/merge.cpp)
target_link_libraries(gaudi_merge RootCnvLib)
