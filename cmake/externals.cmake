# Configuration of some useful (possibly embedded) externals

option(GAUDI_USE_SYSTEM_LIBRARIES
       "Global option to disable/enable use of internally built externals"
       YES)

# C++ Guidelines Support Library,
# see http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
# and https://its.cern.ch/jira/browse/GAUDI-1265
option(GAUDI_USE_SYSTEM_CPP_GSL
       "If to use the C++ Guidelines Support Library from the system or providing it through Gaudi"
       ${GAUDI_USE_SYSTEM_LIBRARIES})
if(NOT GAUDI_USE_SYSTEM_CPP_GSL)
  set(CPP_GSL_URL https://github.com/Microsoft/GSL.git)
  set(CPP_GSL_VERSION b07383ea)
  message(STATUS "Using and shipping ${CPP_GSL_URL} version ${CPP_GSL_VERSION}")
  set(CPP_GSL_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/GSL)
  if(NOT EXISTS ${CPP_GSL_DIR})
    execute_process(COMMAND git clone ${CPP_GSL_URL} ${CPP_GSL_DIR})
  endif()
  execute_process(COMMAND git checkout ${CPP_GSL_VERSION}
                  WORKING_DIRECTORY ${CPP_GSL_DIR})
  install(DIRECTORY ${CPP_GSL_DIR}/gsl DESTINATION include)
  install(FILES ${CPP_GSL_DIR}/LICENSE DESTINATION include/gsl)
  set(CPPGSL_INCLUDE_DIR NAMES ${CPP_GSL_DIR})
else()
  find_package(cppgsl)
endif()
if(CPPGSL_INCLUDE_DIR)
  set(CPPGSL_INCLUDE_DIRS ${CPP_GSL_INCLUDE_DIR})
  include_directories(SYSTEM ${CPPGSL_INCLUDE_DIRS})
  set(CPPGSL_FOUND TRUE)
  set_property(GLOBAL APPEND PROPERTY GAUDI_REQUIRED_PATHS ${CPPGSL_INCLUDE_DIRS})
else()
  message(WARNING "C++ Guidelines Support Library not available")
  set(CPPGSL_FOUND FALSE)
endif()

# Ranges v3,
# see https://its.cern.ch/jira/browse/GAUDI-1266
# see https://github.com/ericniebler/range-v3
option(GAUDI_USE_SYSTEM_RANGES_V3
       "If to use the Ranges-v3 library from the system or providing it through Gaudi"
       ${GAUDI_USE_SYSTEM_LIBRARIES})
if(NOT GAUDI_USE_SYSTEM_RANGES_V3)
  set(RANGES_V3_URL https://github.com/ericniebler/range-v3.git)
  set(RANGES_V3_VERSION 7c2b10f0)
  message(STATUS "Using and shipping ${RANGES_V3_URL} version ${RANGES_V3_VERSION}")
  set(RANGES_V3_DIR ${CMAKE_CURRENT_BINARY_DIR}/ext/range-v3)
  if(NOT EXISTS ${RANGES_V3_DIR})
    execute_process(COMMAND git clone ${RANGES_V3_URL} ${RANGES_V3_DIR})
  endif()
  execute_process(COMMAND git checkout ${RANGES_V3_VERSION}
                  WORKING_DIRECTORY ${RANGES_V3_DIR})
  install(DIRECTORY ${RANGES_V3_DIR}/include/ DESTINATION include)
  set(RANGEV3_INCLUDE_DIR NAMES ${RANGES_V3_DIR}/include)
else()
  find_package(Rangev3 REQUIRED)
endif()
if(RANGEV3_INCLUDE_DIR)
  set(RANGEV3_INCLUDE_DIRS ${RANGEV3_INCLUDE_DIR})
  include_directories(SYSTEM ${RANGEV3_INCLUDE_DIRS})
  set(RANGEV3_FOUND TRUE)
  set_property(GLOBAL APPEND PROPERTY GAUDI_REQUIRED_PATHS ${RANGEV3_INCLUDE_DIRS})
endif()
