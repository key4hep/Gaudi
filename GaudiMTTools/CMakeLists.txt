gaudi_subdir(GaudiMTTools)

gaudi_depends_on_subdirs(GaudiAlg)

find_package(Boost REQUIRED COMPONENTS thread system)
find_package(TBB)

gaudi_add_module(GaudiMTTools *.cpp
                 LINK_LIBRARIES GaudiAlgLib Boost TBB
                 INCLUDE_DIRS Boost TBB)
