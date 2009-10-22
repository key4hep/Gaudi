#ifndef GAUDIKERNEL_LISTITEM_H
#define GAUDIKERNEL_LISTITEM_H

// Include files
#if !defined(GAUDI_V22_API) || defined(G22_NO_DEPRECATED)
#include "GaudiKernel/TypeNameString.h"
typedef Gaudi::Utils::TypeNameString ListItem;
#else
#error "Deprecated header file, use TypeNameString.h instead"
#endif

#endif /* GAUDIKERNEL_LISTITEM_H */
