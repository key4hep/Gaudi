gaudi_subdir(Util)

gaudi_depends_on_subdirs(GaudiKernel GaudiPluginService)

find_package(Boost COMPONENTS program_options regex log log_setup REQUIRED)
find_package(ROOT REQUIRED)

# Hide some Boost compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#---Util Executables--------------------------------------------------------
#set(CMAKE_BUILD_TYPE Release)

# Needed to link against Boost log.
add_definitions(-DBOOST_LOG_DYN_LINK)

gaudi_add_executable(genconf genconf.cpp
                 LINK_LIBRARIES GaudiKernel GaudiPluginService
                                ${Boost_LIBRARIES} ${ROOT_LIBRARIES}
                 INCLUDE_DIRS Boost ROOT)
set_target_properties(genconf PROPERTIES ENABLE_EXPORTS TRUE)

if(WIN32)
  gaudi_add_executable(genwindef genwindef.cpp LibSymbolInfo.cpp)
endif()

gaudi_add_executable(instructionsetLevel instructionsetLevel.cpp
                 LINK_LIBRARIES GaudiKernel GaudiPluginService)
