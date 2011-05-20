# - Locate CppUnit library
# Defines:
#
#  CppUnit_FOUND
#  CppUnit_INCLUDE_DIR
#  CppUnit_INCLUDE_DIRS (not cached)
#  CppUnit_LIBRARIES

find_path(CppUnit_INCLUDE_DIR cppunit/Test.h)
find_library(CppUnit_LIBRARIES NAMES cppunit)

set(CppUnit_INCLUDE_DIRS ${CppUnit_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set CppUnit_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CppUnit DEFAULT_MSG CppUnit_INCLUDE_DIR CppUnit_LIBRARIES)

mark_as_advanced(CppUnit_FOUND CppUnit_INCLUDE_DIR CppUnit_LIBRARIES)
