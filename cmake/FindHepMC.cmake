# - Locate HepMC library
# Defines:
#
#  HepMC_FOUND
#  HepMC_INCLUDE_DIR
#  HepMC_INCLUDE_DIRS (not cached)
#  HepMC_LIBRARIES

find_path(HepMC_INCLUDE_DIR HepMC/ParticleData.hh)
find_library(HepMC_LIBRARIES NAMES HepMC)

set(HepMC_INCLUDE_DIRS ${HepMC_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set HepMC_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HepMC DEFAULT_MSG HepMC_INCLUDE_DIR HepMC_LIBRARIES)

mark_as_advanced(HepMC_FOUND HepMC_INCLUDE_DIR HepMC_LIBRARIES)
