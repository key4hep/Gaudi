INCLUDE(Configuration)

SET(GSL_native_version ${GSL_config_version})
SET(GSL_home ${LCG_external}/GSL/${GSL_native_version}/${LCG_system})

SET(GSL_FOUND 1)
SET(GSL_INCLUDE_DIRS ${GSL_home}/include)
SET(GSL_LIBRARY_DIRS ${GSL_home}/lib)
SET(GSL_LIBRARIES gsl gslcblas)

SET(GSL_environment LD_LIBRARY_PATH+=${GSL_home}/lib)
