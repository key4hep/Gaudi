INCLUDE(Configuration)

SET(CLHEP_native_version ${CLHEP_config_version})
SET(CLHEP_home ${LCG_external}/clhep/${CLHEP_native_version}/${LCG_system})

SET(CLHEP_FOUND 1)
SET(CLHEP_INCLUDE_DIRS ${CLHEP_home}/include)
SET(CLHEP_LIBRARY_DIRS ${CLHEP_home}/lib)
SET(CLHEP_LIBRARIES  
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

SET(CLHEP_environment LD_LIBRARY_PATH+=${CLHEP_home}/lib )
 

