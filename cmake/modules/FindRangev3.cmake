# Module locating the rangev3 headers.
#
# Defines:
#  - RANGEV3_FOUND
#  - RANGEV3_INCLUDE_DIR
#  - RANGEV3_INCLUDE_DIRS (not cached)

# Look for the header directory:
find_path( RANGEV3_INCLUDE_DIR
   NAMES range/v3/all.hpp
   HINTS $ENV{RANGEV3_ROOT_DIR}/include ${RANGEV3_ROOT_DIR}/include )
set( RANGEV3_INCLUDE_DIRS ${RANGEV3_INCLUDE_DIR} )

# Handle the regular find_package arguments:
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Rangev3 DEFAULT_MSG RANGEV3_INCLUDE_DIR )

# Mark the cached variables as "advanced":
mark_as_advanced( RANGEV3_FOUND RANGEV3_INCLUDE_DIR )
