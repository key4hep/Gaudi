gaudi_subdir(GaudiAlg)

gaudi_depends_on_subdirs(GaudiUtils GaudiKernel)

find_package(AIDA)
find_package(Boost COMPONENTS regex)
find_package(ROOT COMPONENTS RIO)

# Hide some Boost/ROOT compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiAlgLib src/lib/*.cpp
                  LINK_LIBRARIES GaudiUtilsLib Boost
                  INCLUDE_DIRS Boost AIDA ${RANGES_V3_INCLUDE_DIRS}
                  PUBLIC_HEADERS GaudiAlg)
gaudi_add_module(GaudiAlg src/components/*.cpp
                 LINK_LIBRARIES GaudiAlgLib GaudiKernel ROOT
                 INCLUDE_DIRS Boost AIDA)
