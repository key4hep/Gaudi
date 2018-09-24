gaudi_subdir(Gaudi)

gaudi_depends_on_subdirs(GaudiKernel)

#---Executables-------------------------------------------------------------
gaudi_add_executable(Gaudi src/main.cpp LINK_LIBRARIES GaudiKernel)

#---Installation------------------------------------------------------------
gaudi_install_python_modules()
gaudi_install_scripts()

#---Tests-------------------------------------------------------------------
# FIXME: these variables must be unset in the tests for compatibility with CMT
gaudi_build_env(UNSET GAUDIAPPNAME
                UNSET GAUDIAPPVERSION)

gaudi_add_test(QMTest QMTEST
               ENVIRONMENT
                 JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests/pyjobopts:${CMAKE_CURRENT_SOURCE_DIR}/tests
                 PYTHONPATH+=${CMAKE_CURRENT_SOURCE_DIR}/tests/python)

gaudi_add_test(nose
               COMMAND nosetests --with-doctest -v
                  ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose
                  ${CMAKE_CURRENT_SOURCE_DIR}/python/Gaudi)

# Settings relating to sanitizer builds
if (SANITIZER_ENABLED)

  message(STATUS "Enabled sanitizer ${SANITIZER_ENABLED}")

  # Enable automatic LD_PRELOAD
  gaudi_env(      SET PRELOAD_SANITIZER_LIB ${SANITIZER_ENABLED})
  gaudi_build_env(SET PRELOAD_SANITIZER_LIB ${SANITIZER_ENABLED})

  # Address sanitizer runtime options
  set(GAUDI_ASAN_OPTS detect_leaks=0,alloc_dealloc_mismatch=0,halt_on_error=0,suppressions=\${GAUDIROOT}/job/Gaudi-ASan.supp 
      CACHE STRING "Runtime options for AddressSanitizer")
  message(STATUS "ASAN OPTIONS  ${GAUDI_ASAN_OPTS}")
  gaudi_env(      SET ASAN_OPTIONS ${GAUDI_ASAN_OPTS} )
  gaudi_build_env(SET ASAN_OPTIONS ${GAUDI_ASAN_OPTS} )

  # Leak sanitizer runtime options
  set(GAUDI_LSAN_OPTS print_suppressions=0,halt_on_error=0,suppressions=\${GAUDIROOT}/job/Gaudi-LSan.supp 
      CACHE STRING "Runtime options for LeakSanitizer")
  message(STATUS "LSAN OPTIONS  ${GAUDI_LSAN_OPTS}")
  gaudi_env(      SET LSAN_OPTIONS ${GAUDI_LSAN_OPTS} )
  gaudi_build_env(SET LSAN_OPTIONS ${GAUDI_LSAN_OPTS} )

  # Thread sanitizer runtime options
  # Note this sanitizer is known to prefer to be run with *everything* built using the
  # sanitizer, which is not possible here (i.e. LCG externals). See
  # https://github.com/google/sanitizers/wiki/ThreadSanitizerCppManual#non-instrumented-code
  # Using the options ignore_interceptors_accesses=1,ignore_noninstrumented_modules=1
  # seems to help reduce the noise a bit, but might remove real issues..
  set(GAUDI_TSAN_OPTS print_suppressions=0,halt_on_error=0,ignore_interceptors_accesses=1,ignore_noninstrumented_modules=1,suppressions=\${GAUDIROOT}/job/Gaudi-TSan.supp 
      CACHE STRING "Runtime options for ThreadSanitizer")
  message(STATUS "TSAN OPTIONS  ${GAUDI_TSAN_OPTS}")
  gaudi_env(      SET TSAN_OPTIONS ${GAUDI_TSAN_OPTS} )
  gaudi_build_env(SET TSAN_OPTIONS ${GAUDI_TSAN_OPTS} )

  # Undefined Behaviour sanitizer runtime options
  set(GAUDI_UBSAN_OPTS print_stacktrace=1,print_suppressions=0,halt_on_error=0,suppressions=\${GAUDIROOT}/job/Gaudi-UBSan.supp 
      CACHE STRING "Runtime options for UndefinedSanitizer")
  message(STATUS "UBSAN OPTIONS ${GAUDI_UBSAN_OPTS}")
  gaudi_env(      SET UBSAN_OPTIONS ${GAUDI_UBSAN_OPTS} )
  gaudi_build_env(SET UBSAN_OPTIONS ${GAUDI_UBSAN_OPTS} )

endif()
