# - Locate fftw library
# Defines:
#
#  fftw_FOUND
#  fftw_INCLUDE_DIR
#  fftw_INCLUDE_DIRS (not cached)
#  fftw_LIBRARIES

find_path(fftw_INCLUDE_DIR fftw3.h)
find_library(fftw_LIBRARIES NAMES fftw3)

set(fftw_INCLUDE_DIRS ${fftw_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set fftw_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(fftw DEFAULT_MSG fftw_INCLUDE_DIR fftw_LIBRARIES)

mark_as_advanced(fftw_FOUND fftw_INCLUDE_DIR fftw_LIBRARIES)
