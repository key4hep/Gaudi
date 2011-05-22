# - Locate HepPDT library
# Defines:
#
#  HepPDT_FOUND
#  HepPDT_INCLUDE_DIR
#  HepPDT_INCLUDE_DIRS (not cached)
#  HepPDT_PDT_LIBRARY
#  HepPDT_PID_LIBRARY
#  HepPDT_LIBRARIES (not cached)

find_path(HepPDT_INCLUDE_DIR HepPDT/ParticleData.hh)
find_library(HepPDT_PDT_LIBRARY NAMES HepPDT)
find_library(HepPDT_PID_LIBRARY NAMES HepPID)

set(HepPDT_LIBRARIES ${HepPDT_PDT_LIBRARY} ${HepPDT_PID_LIBRARY})

set(HepPDT_INCLUDE_DIRS ${HepPDT_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set HepPDT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HepPDT DEFAULT_MSG HepPDT_INCLUDE_DIR HepPDT_LIBRARIES)

mark_as_advanced(HepPDT_FOUND HepPDT_INCLUDE_DIR HepPDT_LIBRARIES
                 HepPDT_PDT_LIBRARY HepPDT_PID_LIBRARY)
