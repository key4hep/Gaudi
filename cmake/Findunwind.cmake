# - Locate unwind library
# Defines:
#
#  unwind_FOUND
#  unwind_INCLUDE_DIR
#  unwind_INCLUDE_DIRS (not cached)
#  unwind_LIBRARIES

find_path(unwind_INCLUDE_DIR unwind.h)
find_library(unwind_LIBRARIES NAMES unwind)

set(unwind_INCLUDE_DIRS ${unwind_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set unwind_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(unwind DEFAULT_MSG unwind_INCLUDE_DIR unwind_LIBRARIES)

mark_as_advanced(unwind_FOUND unwind_INCLUDE_DIR unwind_LIBRARIES)
