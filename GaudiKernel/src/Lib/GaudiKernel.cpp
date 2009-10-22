// $Id: GaudiKernel.cpp,v 1.3 2006/04/20 16:42:58 hmd Exp $
// Dirty trick in order to go back to the old (gcc 2.95) behavior of
// dynamic_cast.

#if defined (__GNUC__) 
#  undef __GXX_WEAK__
//#    include "../gcc/tinfo.cc"
#  define __GXX_WEAK__ 1
#endif
