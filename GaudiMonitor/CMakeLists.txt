gaudi_subdir(GaudiMonitor v3r3)

gaudi_depends_on_subdirs(GaudiKernel)

if(WIN32)
  set(winsocklib ws2_32.lib)
endif()
#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiMonitor src/*.cpp
                 LINK_LIBRARIES GaudiKernel ${winsocklib})

if(WIN32 AND GAUDI_HIDE_WARNINGS)
  # These are needed because Windows compiler doesn't like standard C and POSIX functions,
  # but this package containes some imported (external) C file that is compiled here in C++.
  set_property(TARGET GaudiMonitor
               PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS;_SCL_SECURE_NO_WARNINGS)
  set_property(TARGET GaudiMonitor
               PROPERTY COMPILE_FLAGS /wd4996)
endif()
