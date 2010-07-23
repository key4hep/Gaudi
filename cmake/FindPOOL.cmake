INCLUDE(Configuration)


SET(POOL_native_version ${POOL_config_version})
SET(POOL_base ${LCG_releases}/POOL/${POOL_native_version})
SET(POOL_home ${POOL_base}/${LCG_platform})

SET(POOL_FOUND 1)
SET(POOL_INCLUDE_DIRS ${POOL_base}/include)
SET(POOL_LIBRARY_DIRS ${POOL_home}/lib)
SET(POOL_LIBRARIES lcg_POOLCore lcg_DataSvc lcg_PersistencySvc lcg_StorageSvc )

SET(POOL_environment LD_LIBRARY_PATH+=${POOL_home}/lib)

#tag POOL_HAS_SERVICES    POOL_HAS_CORE
#tag POOL_HAS_FILEMGT     POOL_HAS_SERVICES
#tag POOL_HAS_COLLECTIONS POOL_HAS_CORE
#tag POOL_HAS_RELATIONAL  POOL_HAS_CORE
#tag POOL_HAS_PERSISTENCY POOL_HAS_COLLECTIONS POOL_HAS_FILEMGT
#tag POOL_HAS_ALL         POOL_HAS_PERSISTENCY POOL_HAS_RELATIONAL

#apply_tag POOL_HAS_PERSISTENCY

# core libset
#macro POOL_Core_libset           "" \
#      POOL_HAS_CORE&target-unix  "-llcg_POOLCore" \
#      POOL_HAS_CORE&target-winxp "lcg_POOLCore.lib"

# services libset
#macro POOL_Services_libset           "" \
#      POOL_HAS_SERVICES&target-unix  "-llcg_DataSvc -llcg_PersistencySvc -llcg_StorageSvc" \
#      POOL_HAS_SERVICES&target-winxp "lcg_DataSvc.lib lcg_PersistencySvc.lib lcg_StorageSvc.lib"

# filemgt libset
#macro POOL_FileMgt_libset           "" \
#      POOL_HAS_FILEMGT&target-unix  "-llcg_FileCatalog" \
#      POOL_HAS_FILEMGT&target-winxp "lcg_FileCatalog.lib"

# collections libset
#macro POOL_Collections_libset           "" \
#      POOL_HAS_COLLECTIONS&target-unix  "-llcg_CollectionBase -llcg_Collection" \
#      POOL_HAS_COLLECTIONS&target-winxp ""

# relational libset
#macro POOL_Relational_libset           "" \
#      POOL_HAS_RELATIONAL&target-unix  "-llcg_RelationalAccess" \
#      POOL_HAS_RELATIONAL&target-winxp "lcg_RelationalAccess.lib" 
