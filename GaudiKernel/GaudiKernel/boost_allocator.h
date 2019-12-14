/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_BOOST_ALLOCATOR_H
#define GAUDIKERNEL_BOOST_ALLOCATOR_H 1

/// Include file needed to use Boost singleton pool.

#ifdef _WIN32
// Avoid conflicts between Windows' headers and MSG.
#  ifndef NOMSG
#    define NOMSG
#    ifndef NOGDI
#      define NOGDI
#    endif
#  endif
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#endif

#include <boost/pool/singleton_pool.hpp>

#endif // GAUDIKERNEL_BOOST_ALLOCATOR_H
