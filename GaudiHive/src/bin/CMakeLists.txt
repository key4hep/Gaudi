gaudi_subdir(bin v1r0)

find_package(Boost REQUIRED COMPONENTS program_options)

gaudi_depends_on_subdirs(GaudiKernel)


#---Util Executables--------------------------------------------------------
#set(CMAKE_BUILD_TYPE Release)
gaudi_add_executable(makeThesis makeThesis.cpp )
gaudi_add_executable(concurrentRun concurrentRun.cpp 
                     LINK_LIBRARIES GaudiKernel ${Boost_LIBRARIES}
                    )
