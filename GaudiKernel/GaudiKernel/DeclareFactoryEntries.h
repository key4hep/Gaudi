#ifndef GAUDIKERNEL_DECLAREFACTORYENTRIES_H
#define GAUDIKERNEL_DECLAREFACTORYENTRIES_H 1

#ifdef ATLAS

// Backwards compatibility dummies
 #define DECLARE_ALGORITHM(x)  /* dummy */
 #define DECLARE_NAMESPACE_ALGORITHM(n,x) /* dummy */

 #define DECLARE_AUDITOR(x)  /* dummy */
 #define DECLARE_NAMESPACE_AUDITOR(n,x)  /* dummy */

 #define DECLARE_GENERIC_CONVERTER(x) /* dummy */
 #define DECLARE_NAMESPACE_GENERIC_CONVERTER(n,x) /* dummy */
 #define DECLARE_CONVERTER(x) /* dummy */
 #define DECLARE_NAMESPACE_CONVERTER(n,x)   /* dummy */

 #define DECLARE_SERVICE(x)  /* dummy */
 #define DECLARE_NAMESPACE_SERVICE(n,x) /* dummy */

 #define DECLARE_ALGTOOL(x)             /* dummy */
 #define DECLARE_NAMESPACE_ALGTOOL(n,x) /* dummy */
 #define DECLARE_TOOL(x)                /* dummy */
 #define DECLARE_NAMESPACE_TOOL(n,x)    /* dummy */

 #define DECLARE_FACTORY_ENTRIES(x) void x##_load()

#else

 #ifdef GAUDI_NEW_PLUGIN_SERVICE
   #warning "obsolete empty header, please remove it"
 #endif

#endif

#endif // GAUDIKERNEL_DECLAREFACTORYENTRIES_H
