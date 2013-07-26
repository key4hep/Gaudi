gaudi_subdir(GaudiUtils v4r6)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost)
find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist XMLIO)
find_package(XercesC)
find_package(UUID)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiUtilsLib src/Lib/*.cpp
                  LINK_LIBRARIES GaudiKernel ROOT
                  INCLUDE_DIRS ROOT AIDA
                  PUBLIC_HEADERS GaudiUtils)
gaudi_add_module(GaudiUtils src/component/*.cpp
                 LINK_LIBRARIES GaudiUtilsLib XercesC UUID
                 INCLUDE_DIRS XercesC UUID)
