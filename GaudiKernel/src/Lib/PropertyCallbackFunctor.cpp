// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/PropertyCallbackFunctor.cpp,v 1.2 2001/03/14 15:30:17 mato Exp $

/************************
 *This Class's Header
 *   Needed to USE class
 ************************/
#include "GaudiKernel/PropertyCallbackFunctor.h"


/*****************************
 * C++ DEFINITION SECTION
 *   Internal Sub-section
 *   Static Member Sub-section
 *****************************/

/* Static Data Member Definitions */
//  Instantiate an immutable null pointer for
//  SimplePropertyRefs that refer to private
//  memory

const PropertyCallbackFunctor* const PropertyCallbackFunctor::nullPropertyCallbackFunctorPointer = 0;
