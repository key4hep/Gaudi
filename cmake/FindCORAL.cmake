INCLUDE(Configuration)


SET(CORAL_native_version ${CORAL_config_version})
SET(CORAL_base ${LCG_releases}/CORAL/${CORAL_native_version})
SET(CORAL_home ${CORAL_base}/${LCG_platform})

SET(CORAL_FOUND 1)
SET(CORAL_INCLUDE_DIRS ${CORAL_base}/include)
SET(CORAL_LIBRARY_DIRS ${CORAL_home}/lib)
SET(CORAL_LIBRARIES lcg_CoralBase )

SET(CORAL_environment LD_LIBRARY_PATH+=${CORAL_home}/lib)

