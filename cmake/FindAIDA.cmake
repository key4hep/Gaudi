# - Try to find AIDA
# Defines:
#
#  AIDA_FOUND
#  AIDA_INCLUDE_DIRS

if(DEFINED AIDA_home)
  set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${AIDA_home}/src/cpp)
endif()

find_path(AIDA_INCLUDE_DIRS AIDA/AIDA.h)

# handle the QUIETLY and REQUIRED arguments and set BZip2_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AIDA DEFAULT_MSG AIDA_INCLUDE_DIRS)

mark_as_advanced(AIDA_INCLUDE_DIRS)
