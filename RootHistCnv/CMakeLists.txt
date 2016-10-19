gaudi_subdir(RootHistCnv)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist Matrix Tree)
find_package(Boost)

# Hide some ROOT compile time warnings
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS})

#---Libraries---------------------------------------------------------------
gaudi_add_module(RootHistCnv src/*.cpp
                 LINK_LIBRARIES GaudiKernel ROOT
                 INCLUDE_DIRS Boost ROOT AIDA)
