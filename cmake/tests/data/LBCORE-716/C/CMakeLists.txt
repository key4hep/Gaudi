cmake_minimum_required(VERSION 3.6)
find_package(GaudiProject)
# prevent irrelevant problems on SLC6
set(GAUDI_CXX_STANDARD "c++98"
    CACHE STRING "Version of the C++ standard to be used.")
gaudi_project(C HEAD USE A HEAD)
