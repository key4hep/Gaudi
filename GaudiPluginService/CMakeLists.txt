# Provide a simple and generic plugin system.
# author: Marco Clemencic <marco.clemencic@cern.ch>

gaudi_subdir(GaudiPluginService v1r0)

# Library
gaudi_add_library(GaudiPluginService src/PluginService.cpp
                  LINK_LIBRARIES ${CMAKE_DL_LIBS}
                  PUBLIC_HEADERS Gaudi)
# Application
gaudi_add_executable(listcomponents src/listcomponents.cpp src/PluginService.cpp)
target_link_libraries(listcomponents ${CMAKE_DL_LIBS})

gaudi_add_unit_test(Test_GaudiPluginService_UseCases tests/src/UseCases.cpp
                    LINK_LIBRARIES GaudiPluginService
                    TYPE Boost)
