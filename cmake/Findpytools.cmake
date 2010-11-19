include(Configuration)

set(pytools_native_version ${pytools_config_version}_python${Python_config_version_twodigit})
set(pytools_home ${LCG_external}/pytools/${pytools_native_version}/${LCG_system})

set(pytools_FOUND 1)

set(pytools_environment PYTHONPATH+=${pytools_home}/lib/python${Python_config_version_twodigit}/site-packages)
