include(Configuration)

set(COOL_native_version ${COOL_config_version})
set(COOL_base ${LCG_releases}/COOL/${COOL_native_version})
set(COOL_home ${COOL_base}/${LCG_platform})

set(COOL_FOUND 1)
set(COOL_INCLUDE_DIRS ${COOL_base}/include)
set(COOL_LIBRARY_DIRS ${COOL_home}/lib)
SET(COOL_LIBRARIES lcg_CoolKernel )
