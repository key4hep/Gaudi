gaudi_subdir(bin)

find_package(Boost REQUIRED COMPONENTS program_options)

gaudi_depends_on_subdirs(GaudiKernel)

# Hide some Boost/ROOT compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

#---Util Executables--------------------------------------------------------
#set(CMAKE_BUILD_TYPE Release)
gaudi_add_executable(makeThesis makeThesis.cpp )
gaudi_add_executable(concurrentRun concurrentRun.cpp
                     LINK_LIBRARIES GaudiKernel ${Boost_LIBRARIES}
                    )
