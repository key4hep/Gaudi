include(Configuration)


set(POOL_native_version ${POOL_config_version})
set(POOL_base ${LCG_releases}/POOL/${POOL_native_version})
set(POOL_home ${POOL_base}/${LCG_platform})

set(POOL_FOUND 1)
set(POOL_INCLUDE_DIRS ${POOL_base}/include)
set(POOL_LIBRARY_DIRS ${POOL_home}/lib)

set(POOL_COMPONENTS POOLCore DataSvc PersistencySvc StorageSvc FileCatalog CollectionBase Collection RelationalAccess)
foreach(component ${POOL_COMPONENTS})
  set(POOL_${component}_LIBRARY lcg_${component})
endforeach()

set(POOL_LIBRARIES ${POOL_POOLCore_LIBRARY} ${POOL_DataSvc_LIBRARY} ${POOL_PersistencySvc_LIBRARY} ${POOL_StorageSvc_LIBRARY})
set(POOL_Collections_LIBRARIES ${POOL_CollectionBase_LIBRARY} ${POOL_Collection_LIBRARY} )
set(POOL_Relational_LIBRARIES ${POOL_RelationalAccess_LIBRARY} )


if(WIN32)
  SET(POOL_environment PATH+=${POOL_home}/lib)
else()
  SET(POOL_environment LD_LIBRARY_PATH+=${POOL_home}/lib)
endif()

