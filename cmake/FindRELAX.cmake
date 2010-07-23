INCLUDE(Configuration)

SET(RELAX_native_version ${RELAX_config_version})
SET(RELAX_base ${LCG_releases}/RELAX/${RELAX_native_version})
SET(RELAX_home ${RELAX_base}/${LCG_platform})

SET(RELAX_FOUND 1)

SET(RELAX_environment LD_LIBRARY_PATH+=${RELAX_home}/lib)
