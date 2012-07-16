gaudi_subdir(GaudiUtils v4r1)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost)
find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist XMLIO Thread Matrix MathCore)
find_package(XercesC)
find_package(uuid)

#---Libraries---------------------------------------------------------------
gaudi_add_library(GaudiUtilsLib Lib/*.cpp
                  LINK_LIBRARIES GaudiKernel ROOT
                  INCLUDE_DIRS ROOT AIDA
                  PUBLIC_HEADERS GaudiUtils)
gaudi_add_module(GaudiUtils component/*.cpp
                 LINK_LIBRARIES GaudiUtilsLib XercesC uuid
                 INCLUDE_DIRS XercesC uuid)
