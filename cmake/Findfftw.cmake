INCLUDE(Configuration)

SET(fftw_native_version ${fftw_config_version})
SET(fftw_home ${LCG_external}/fftw3/${fftw_native_version}/${LCG_system})
SET(Boost_compiler_version gcc43)


SET(fftw_FOUND 1)
SET(fftw_INCLUDE_DIRS ${fftw_home}/include)
SET(fftw_LIBRARY_DIRS ${fftw_home}/lib)
SET(fftw_LIBRARIES fftw3 )

SET(fftw_environment LD_LIBRARY_PATH+=${fftw_home}/lib)
