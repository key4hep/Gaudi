gaudi_subdir(GaudiMTTools)

gaudi_depends_on_subdirs(GaudiAlg)

find_package(Boost REQUIRED COMPONENTS thread system)
find_package(TBB)
find_package(AIDA)

# Without AIDA available, this package can't function.
if( NOT AIDA_FOUND )
   return()
endif()

# Hide some TBB compile time warnings
include_directories(SYSTEM ${TBB_INCLUDE_DIRS})

gaudi_add_module(GaudiMTTools *.cpp
                 LINK_LIBRARIES GaudiAlgLib Boost TBB
                 INCLUDE_DIRS Boost TBB)
