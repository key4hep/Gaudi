#ifndef GAUDIGSL_GAUDIGSLMATH_H 
#define GAUDIGSL_GAUDIGSLMATH_H 1

// this trick defines the macro '__cling__' only when this header is processed
// by cling parser, while it is not set when we compile the .cpp of the
// dictionary (which is processed by the actual compiler)
#ifndef G__DICTIONARY
#define __cling__
#endif

// ============================================================================
// Include files
// ============================================================================
#include "GaudiGSL/GaudiGSL.h"
#include "GaudiGSL/GslError.h"
#include "GaudiGSL/GslErrorHandlers.h"
#include "GaudiGSL/IEqSolver.h"
#include "GaudiGSL/IFuncMinimum.h"
#include "GaudiGSL/IGslErrorHandler.h"
#include "GaudiGSL/IGslSvc.h"
// ============================================================================
#include "GaudiMath/GaudiMath.h"
// ============================================================================

#ifndef G__DICTIONARY
#undef __cling__
#endif

// ============================================================================
// The 
// ============================================================================
#endif // GAUDIGSL_GAUDIGSLMATH_H
// ============================================================================
