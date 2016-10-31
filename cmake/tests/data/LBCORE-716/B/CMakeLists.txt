CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)
find_package(GaudiProject)
# prevent irrelevant problems on SLC6
set(GAUDI_CXX_STANDARD "c++98"
    CACHE STRING "Version of the C++ standard to be used.")
gaudi_project(B HEAD USE A HEAD)
