// $Id: boost_allocator.h,v 1.1 2006/05/02 16:49:02 hmd Exp $
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
