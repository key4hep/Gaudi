#ifndef GAUDIKERNEL_LIKELY_H
#define GAUDIKERNEL_LIKELY_H 1

// Use compilier hinting to improve branch prediction for linux

#ifdef __GNUC__
 #define LIKELY(x)       __builtin_expect((x),1)
 #define UNLIKELY(x)     __builtin_expect((x),0)
#else
 #define LIKELY(x)       x
 #define UNLIKELY(x)     x
#endif

#endif

