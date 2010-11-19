include(Configuration)

set(XercesC_native_version ${XercesC_config_version})
set(XercesC_home ${LCG_external}/XercesC/${XercesC_native_version}/${LCG_system})

set(XercesC_FOUND 1)
set(XercesC_INCLUDE_DIRS ${XercesC_home}/include)
set(XercesC_LIBRARY_DIRS ${XercesC_home}/lib)
if(WIN32)
  set(XercesC_LIBRARIES xerces-c_3.lib)
else()
  set(XercesC_LIBRARIES xerces-c pthread)
endif()

add_definitions(-DXERCESC_GE_31)

if(WIN32)
  set(XercesC_environment PATH+=${XercesC_home}/bin)
else()
  SET(XercesC_environment LD_LIBRARY_PATH+=${XercesC_home}/lib)
endif()

