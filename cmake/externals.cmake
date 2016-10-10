# Configuration of some useful (possibly embedded) externals

# C++ Guidelines Support Library,
# see http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
# and https://its.cern.ch/jira/browse/GAUDI-1265
option(GAUDI_USE_SYSTEM_CPP_GSL
       "If to use the C++ Guidelines Support Library from the system or providing it through Gaudi"
       NO)
if(NOT GAUDI_USE_SYSTEM_CPP_GSL)
  set(CPP_GSL_URL https://github.com/Microsoft/GSL)
  set(CPP_GSL_VERSION 32ca283d)
  message(STATUS "Using and shipping ${CPP_GSL_URL} version ${CPP_GSL_VERSION}")
  set(CPP_GSL_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/GSL)
  if(NOT EXISTS ${CPP_GSL_DIR})
    execute_process(COMMAND git clone ${CPP_GSL_URL} ${CPP_GSL_DIR})
  endif()
  execute_process(COMMAND git checkout ${CPP_GSL_VERSION}
                  WORKING_DIRECTORY ${CPP_GSL_DIR})
  install(DIRECTORY ${CPP_GSL_DIR}/gsl DESTINATION include)
  install(FILES ${CPP_GSL_DIR}/LICENSE DESTINATION include/gsl)
  set(CPP_GSL_INCLUDE_DIR NAMES ${CPP_GSL_DIR})
else()
  find_path(CPP_GSL_INCLUDE_DIR NAMES gsl/gsl)
endif()
if(CPP_GSL_INCLUDE_DIR)
  set(CPP_GSL_INCLUDE_DIRS ${CPP_GSL_INCLUDE_DIR})
  include_directories(SYSTEM ${CPP_GSL_INCLUDE_DIRS})
  set(CPP_GSL_FOUND TRUE)
else()
  message(WARNING "C++ Guidelines Support Library not available")
  set(CPP_GSL_FOUND FALSE)
endif()
