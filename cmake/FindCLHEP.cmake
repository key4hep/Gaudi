include(Configuration)

set(CLHEP_native_version ${CLHEP_config_version})
set(CLHEP_home ${LCG_external}/clhep/${CLHEP_native_version}/${LCG_system})

set(CLHEP_FOUND 1)
set(CLHEP_INCLUDE_DIRS ${CLHEP_home}/include)
set(CLHEP_LIBRARY_DIRS ${CLHEP_home}/lib)
set(CLHEP_LIBRARIES  
        CLHEP-Cast-${CLHEP_native_version}
        CLHEP-Evaluator-${CLHEP_native_version}
        CLHEP-Exceptions-${CLHEP_native_version}
        CLHEP-GenericFunctions-${CLHEP_native_version}
        CLHEP-Geometry-${CLHEP_native_version}
        CLHEP-Random-${CLHEP_native_version}
        CLHEP-RandomObjects-${CLHEP_native_version}
        CLHEP-RefCount-${CLHEP_native_version}
        CLHEP-Vector-${CLHEP_native_version}
        CLHEP-Matrix-${CLHEP_native_version} )

if(WIN32)
  set(CLHEP_environment PATH+=${CLHEP_home}/lib)
else()
  set(CLHEP_environment LD_LIBRARY_PATH+=${CLHEP_home}/lib )
endif()
 

