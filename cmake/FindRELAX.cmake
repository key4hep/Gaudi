include(Configuration)

set(RELAX_native_version ${RELAX_config_version})
set(RELAX_base ${LCG_releases}/RELAX/${RELAX_native_version})
set(RELAX_home ${RELAX_base}/${LCG_platform})

set(RELAX_FOUND 1)

if(WIN32)
  set(RELAX_environment PATH+=${RELAX_home}/lib)
else()
  set(RELAX_environment LD_LIBRARY_PATH+=${RELAX_home}/lib)
endif()