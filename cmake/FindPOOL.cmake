# - Try to find POOL
# Defines:
#
#  POOL_FOUND
#  POOL_INCLUDE_DIR
#  POOL_INCLUDE_DIRS (not cached)
#  POOL_<component>_LIBRARY
#  POOL_<component>_FOUND
#  POOL_LIBRARIES (not cached)
#  POOL_Collections_LIBRARIES (not cached)
#  POOL_Relational_LIBRARIES (not cached)

set(_POOL_COMPONENTS POOLCore DataSvc PersistencySvc StorageSvc FileCatalog CollectionBase Collection RelationalAccess)
foreach(component ${_POOL_COMPONENTS})
  find_library(POOL_${component}_LIBRARY NAMES lcg_${component})
  if (POOL_${component}_LIBRARY)
    set(POOL_${component}_FOUND 1)
  else()
    set(POOL_${component}_FOUND 0)
  endif()
endforeach()

foreach(component POOLCore DataSvc PersistencySvc StorageSvc)
  if(POOL_${component}_LIBRARY)
    list(APPEND POOL_LIBRARIES ${POOL_${component}_LIBRARY})
  endif()
endforeach()

foreach(component CollectionBase Collection)
  if(POOL_${component}_LIBRARY)
    list(APPEND POOL_Collections_LIBRARIES ${POOL_${component}_LIBRARY})
  endif()
endforeach()

set(POOL_Relational_LIBRARIES ${POOL_RelationalAccess_LIBRARY})

find_path(POOL_INCLUDE_DIR POOLCore/PContainer.h)

set(POOL_INCLUDE_DIRS ${POOL_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set POOL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(POOL DEFAULT_MSG POOL_INCLUDE_DIR POOL_LIBRARIES POOL_Collections_LIBRARIES POOL_Relational_LIBRARIES)

mark_as_advanced(POOL_FOUND POOL_INCLUDE_DIR)
