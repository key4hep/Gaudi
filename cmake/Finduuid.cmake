# - Locate uuid library
# Defines:
#
#  uuid_FOUND
#  uuid_INCLUDE_DIR
#  uuid_INCLUDE_DIRS (not cached)
#  uuid_LIBRARIES

find_path(uuid_INCLUDE_DIR uuid/uuid.h)
find_library(uuid_LIBRARIES NAMES uuid)

set(uuid_INCLUDE_DIRS ${uuid_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set uuid_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(uuid DEFAULT_MSG uuid_INCLUDE_DIR uuid_LIBRARIES)

mark_as_advanced(uuid_FOUND uuid_INCLUDE_DIR uuid_LIBRARIES)
