include(Configuration)

set(Python_native_version ${Python_config_version})
set(Python_home ${LCG_external}/Python/${Python_native_version}/${LCG_system})

set(Python_FOUND 1)
if(WIN32)
  set(Python_INCLUDE_DIRS ${Python_home}/include)
  set(Python_LIBRARY_DIRS ${Python_home}/libs)
  string(REPLACE "." "" twodigit ${Python_config_version_twodigit})
  set(Python_LIBRARIES python${twodigit})
else()
  set(Python_INCLUDE_DIRS ${Python_home}/include/python${Python_config_version_twodigit})
  set(Python_LIBRARY_DIRS ${Python_home}/lib)
  set(Python_LIBRARIES python${Python_config_version_twodigit} util pthread)
endif()  

if(WIN32)
  set(Python_environment PATH+=${Python_home}/lib)
else()
  set(Python_environment LD_LIBRARY_PATH+=${Python_home}/lib
                         PATH+=${Python_home}/bin)
endif()