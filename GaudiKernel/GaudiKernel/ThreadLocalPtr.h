#ifndef GAUDIKERNEL_THREADLOCALPTR_H
#define GAUDIKERNEL_THREADLOCALPTR_H

#ifdef __GNUC__
  #if __GNUC__ >=4 && (__GNUC_MINOR__ < 8)
    #define THREAD_LOCAL_PTR __thread
  #else
    #define THREAD_LOCAL_PTR thread_local
  #endif
#else
  #define THREAD_LOCAL_PTR thread_local
#endif

#endif
