INCLUDE(Configuration)

SET(XercesC_native_version ${XercesC_config_version})
SET(XercesC_home ${LCG_external}/XercesC/${XercesC_native_version}/${LCG_system})

SET(XercesC_FOUND 1)
SET(XercesC_INCLUDE_DIRS ${XercesC_home}/include)
SET(XercesC_LIBRARY_DIRS ${XercesC_home}/lib)
SET(XercesC_LIBRARIES xerces-c xerces-depdom pthread)

SET(XercesC_environment LD_LIBRARY_PATH+=${XercesC_home}/lib)
