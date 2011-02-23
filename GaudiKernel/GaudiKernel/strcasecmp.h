#ifndef GAUDIKERNEL_STRCASECMP_H
#define GAUDIKERNEL_STRCASECMP_H

#ifdef _WIN32
#include <cstring>
inline int strcasecmp(const char *s1, const char *s2) {
  return ::_stricmp(s1, s2);
}
inline int strncasecmp(const char *s1, const char *s2, size_t n) {
  return ::_strnicmp(s1, s2, n);
}
#else // Unix uses string.h
#include <string.h>
#endif

#endif // GAUDIKERNEL_STRCASECMP_H
