gaudi_subdir(GaudiUtils v5r0)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost)
find_package(AIDA)
set (XMLIO_PACKAGE_NAME XMLIO)
if (${APPLE})
  set (XMLIO_PACKAGE_NAME "")
endif (${APPLE})
find_package(ROOT COMPONENTS RIO Hist ${XMLIO_PACKAGE_NAME} Thread Matrix MathCore)
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
