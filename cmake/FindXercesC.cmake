# - Locate XercesC library
# Defines:
#
#  XercesC_FOUND
#  XercesC_INCLUDE_DIR
#  XercesC_INCLUDE_DIRS (not cached)
#  XercesC_LIBRARIES

find_path(XercesC_INCLUDE_DIR xercesc/util/XercesVersion.hpp)
find_library(XercesC_LIBRARIES NAMES xerces-c)

set(XercesC_INCLUDE_DIRS ${XercesC_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set XercesC_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XercesC DEFAULT_MSG XercesC_INCLUDE_DIR XercesC_LIBRARIES)

mark_as_advanced(XercesC_FOUND XercesC_INCLUDE_DIR XercesC_LIBRARIES)
