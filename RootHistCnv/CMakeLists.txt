gaudi_subdir(RootHistCnv)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist Matrix Tree Thread MathCore Net)
find_package(Boost)

# If AIDA is missing, don't build this module:
if( NOT AIDA_FOUND )
   return()
endif()

# Hide some ROOT, Boost compile time warnings
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(RootHistCnv src/*.cpp
                 LINK_LIBRARIES GaudiKernel ROOT
                 INCLUDE_DIRS Boost ROOT AIDA)
