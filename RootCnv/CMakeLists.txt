require(GaudiKernel GaudiUtils)

find_package(ROOT COMPONENTS RIO Tree TreePlayer MathCore)

gaudi_install_headers(RootCnv)

set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS __POOL_COMPATIBILITY)

gaudi_add_library(RootCnvLib
                     *.cpp
                     LINK_LIBRARIES GaudiKernel GaudiUtilsLib ${ROOT_LIBRARIES}
                     INCLUDE_DIRS GaudiKernel GaudiUtilsLib)

gaudi_add_module(RootCnv
                        components/Components.cpp
                        LINK_LIBRARIES RootCnvLib)

# Note: the set_property above is needed because that's the way REFLEX_BUILD_LIBRARIES
#       takes the "-D" flags, and it make the following lines unneeded.
#set_property(TARGET RootCnvLib RootCnv RootCnvDict
#             APPEND PROPERTY COMPILE_DEFINITIONS __POOL_COMPATIBILITY)

#---Dictionaries------------------------------------------------------------
REFLEX_BUILD_DICTIONARY(RootCnv dict/RootCnv_dict.h  dict/RootCnv_dict.xml LIBRARIES RootCnvLib)

add_executable(gaudi_merge merge/merge.cpp)
target_link_libraries(gaudi_merge RootCnvLib)
