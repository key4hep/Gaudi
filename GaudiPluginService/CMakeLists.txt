# Provide a simple and generic plugin system.
# author: Marco Clemencic <marco.clemencic@cern.ch>

gaudi_subdir(GaudiPluginService)

# Local build options
option(GAUDI_REFLEX_COMPONENT_ALIASES
       "allow use of old style (Reflex) component names"
       ON)
if(GAUDI_REFLEX_COMPONENT_ALIASES)
  add_definitions(-DGAUDI_REFLEX_COMPONENT_ALIASES)
endif()

# Library
gaudi_add_library(GaudiPluginService src/PluginService.cpp src/capi_pluginservice.cpp
                  LINK_LIBRARIES ${CMAKE_DL_LIBS}
                  PUBLIC_HEADERS Gaudi)

# Disable generation of ConfUserDB (must be done before gaudi_install_python_modules)
set_directory_properties(PROPERTIES CONFIGURABLE_USER_MODULES None)
# python modules
gaudi_install_python_modules()

# Application
gaudi_add_executable(listcomponents src/listcomponents.cpp)
target_link_libraries(listcomponents GaudiPluginService ${CMAKE_DL_LIBS})

# Hide some Boost compile time warnings
find_package(Boost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

gaudi_add_unit_test(Test_GaudiPluginService_UseCases tests/src/UseCases.cpp
                    LINK_LIBRARIES GaudiPluginService
                    TYPE Boost)

gaudi_add_test(listcomponents.usage
               COMMAND $<TARGET_FILE_NAME:listcomponents>
               PASSREGEX "Usage:")
gaudi_add_test(listcomponents.help1
               COMMAND $<TARGET_FILE_NAME:listcomponents> -h
               PASSREGEX "Options:")
gaudi_add_test(listcomponents.help2
               COMMAND $<TARGET_FILE_NAME:listcomponents> --help
               PASSREGEX "Options:")
gaudi_add_test(listcomponents.wrong_args
               COMMAND $<TARGET_FILE_NAME:listcomponents> -o
               PASSREGEX "ERROR: missing argument")
