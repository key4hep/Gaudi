include(Configuration)

set(fftw_FOUND 1)
set(fftw_INCLUDE_DIRS ${fftw_home}/include)
set(fftw_LIBRARY_DIRS ${fftw_home}/lib)
set(fftw_LIBRARIES fftw3 )

if(WIN32)
  set(fftw_environment PATH+=${fftw_home}/lib)
else()
  set(fftw_environment LD_LIBRARY_PATH+=${fftw_home}/lib)
endif()