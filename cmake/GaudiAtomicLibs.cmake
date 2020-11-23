#####################################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

#[========================================================================[.rst:
Gaudi Atomic Library Setup
--------------------------

This module sets up the `GAUDI_ATOMIC_LIBS` variable to (optionally) point at
the library providing atomic function calls for the compiled code.

On certain platforms, like armv7l, GCC needs to be instructed explicitly to
link against `libatomic.so`, to build code that uses atomic function calls.
The code tries to smartly figure out whether it needs to use `libatomic.so` or
not, and where it should take it from. But in case it fails, it is possible to
override the `GAUDI_ATOMIC_LIB` cache variable to force the usage of a
particular library. (Or no library at all.)

The code here was taken from ROOT's CMake configuration. Which itself was based
on LLVM's CMake configuration.
#]========================================================================]

# Include the necessary module(s).
include(CheckCXXSourceCompiles)

# Check whether 32 and 64-bit atomic operations can be performed without using
# an extra library on the build platform.
check_cxx_source_compiles("
#include <atomic>
#include <cstdint>
int main() {
  std::atomic<int> a1;
  int a1val = a1.load();
  (void)a1val;
  std::atomic<uint64_t> a2;
  uint64_t a2val = a2.load(std::memory_order_relaxed);
  (void)a2val;
  return 0;
}
" GAUDI_HAVE_CXX_ATOMICS_WITHOUT_LIB)

# Set up the GAUDI_ATOMIC_LIBS variable.
set(GAUDI_ATOMIC_LIBS)
if(NOT GAUDI_HAVE_CXX_ATOMICS_WITHOUT_LIB)
  find_library(GAUDI_ATOMIC_LIB NAMES atomic
    HINTS ENV LD_LIBRARY_PATH
    DOC "Path to the atomic library to use during the build")
  mark_as_advanced(GAUDI_ATOMIC_LIB)
  if(GAUDI_ATOMIC_LIB)
    set(GAUDI_ATOMIC_LIBS ${GAUDI_ATOMIC_LIB})
  endif()
endif()
