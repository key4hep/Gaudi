include(Configuration)

set(AIDA_native_version ${AIDA_config_version})
set(AIDA_home ${LCG_external}/AIDA/${AIDA_native_version}/share)

set(AIDA_FOUND 1)
set(AIDA_INCLUDE_DIRS ${AIDA_home}/src/cpp)

