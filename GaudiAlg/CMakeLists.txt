gaudi_subdir(GaudiAlg v14r1)

gaudi_depends_on_subdirs(GaudiUtils GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS regex)
find_package(ROOT COMPONENTS RIO)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiAlgLib lib/*.cpp
                  LINK_LIBRARIES GaudiUtilsLib Boost
                  INCLUDE_DIRS Boost AIDA
                  PUBLIC_HEADERS GaudiAlg)
gaudi_add_module(GaudiAlg components/*.cpp
                 LINK_LIBRARIES GaudiAlgLib GaudiKernel ROOT
                 INCLUDE_DIRS Boost AIDA)
