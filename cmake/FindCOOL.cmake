INCLUDE(Configuration)

SET(COOL_native_version ${COOL_config_version})
SET(COOL_base ${LCG_releases}/COOL/${COOL_native_version})
SET(COOL_home ${COOL_base}/${LCG_platform})

SET(COOL_FOUND 1)
SET(COOL_INCLUDE_DIRS ${COOL_base}/include)
SET(COOL_LIBRARY_DIRS ${COOL_home}/lib)
SET(COOL_LIBRARIES lcg_CoolKernel )
