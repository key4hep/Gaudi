include(Configuration)

set(RELAX_FOUND 1)

if(WIN32)
  set(RELAX_environment PATH+=${RELAX_home}/lib)
else()
  set(RELAX_environment LD_LIBRARY_PATH+=${RELAX_home}/lib)
endif()