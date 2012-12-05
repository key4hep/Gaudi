gaudi_subdir(RootHistCnv v11r2)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(AIDA)
find_package(ROOT COMPONENTS RIO Hist Matrix Tree)
find_package(Boost)

#---Libraries---------------------------------------------------------------
gaudi_add_module(RootHistCnv src/*.cpp
                 LINK_LIBRARIES GaudiKernel ROOT
                 INCLUDE_DIRS Boost ROOT AIDA)
