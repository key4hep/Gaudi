############################################################################
# CMakeLists.txt file for building GaudiMonitor
# @author Pere Mato, CERN (semi-automaticaly generated)
############################################################################
require(GaudiKernel)

if(WIN32)
  set(winsocklib ws2_32.lib)
endif()
#---Libraries---------------------------------------------------------------
GAUDI_COMPONENT_LIBRARY(GaudiMonitor DataListenerSvc/*.cpp
                        LIBRARIES GaudiKernel ${winsocklib})

if(WIN32 AND HIDE_WARNINGS)
  # These are needed because Windows compiler doesn't like standard C and POSIX functions,
  # but this package containes some imported (external) C file that is compiled here in C++.
  set_property(TARGET GaudiMonitor
               PROPERTY COMPILE_DEFINITIONS _CRT_SECURE_NO_WARNINGS;_SCL_SECURE_NO_WARNINGS)
  set_property(TARGET GaudiMonitor
               PROPERTY COMPILE_FLAGS /wd4996)
  # Disable precision warnings
  set_property(SOURCE src/DataListenerSvc/ApMon.cpp
               PROPERTY COMPILE_FLAGS /wd4244)
endif()
