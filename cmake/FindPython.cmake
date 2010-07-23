INCLUDE(Configuration)

SET(Python_native_version ${Python_config_version})
SET(Python_home ${LCG_external}/Python/${Python_native_version}/${LCG_system})

SET(Python_FOUND 1)
SET(Python_INCLUDE_DIRS ${Python_home}/include/python${Python_config_version_twodigit})
SET(Python_LIBRARY_DIRS ${Python_home}/lib)
SET(Python_LIBRARIES python${Python_config_version_twodigit} util pthread)

SET(Python_environment LD_LIBRARY_PATH+=${Python_home}/lib)
