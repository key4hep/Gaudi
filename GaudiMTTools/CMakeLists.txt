gaudi_subdir(GaudiMTTools v1r0)

gaudi_depends_on_subdirs(GaudiAlg)

find_package(Boost REQUIRED COMPONENTS thread system)

gaudi_add_module(GaudiMTTools *.cpp
                 LINK_LIBRARIES GaudiAlgLib Boost
                 INCLUDE_DIRS Boost)
