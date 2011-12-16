require(GaudiKernel GaudiUtils)

find_package(ROOT)

gaudi_install_headers(RootCnv)

gaudi_linker_library(RootCnvLib
                     *.cpp
                     LIBRARIES GaudiKernel GaudiUtilsLib
                               ${ROOT_RIO_LIBRARY} ${ROOT_Tree_LIBRARY}
                               ${ROOT_TreePlayer_LIBRARY} ${ROOT_MathCore_LIBRARY}
                     USE_HEADERS GaudiKernel GaudiUtilsLib)

gaudi_component_library(RootCnv
                        components/Components.cpp
                        LIBRARIES RootCnvLib)

set_property(TARGET RootCnvLib RootCnv
             APPEND PROPERTY COMPILE_DEFINITIONS __POOL_COMPATIBILITY)

