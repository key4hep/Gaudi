#ifndef GAUDIGSL_GAUDIGSLMATH_H
#define GAUDIGSL_GAUDIGSLMATH_H 1

// this trick defines the macro '__cling__' only when this header is processed
// by cling parser, while it is not set when we compile the .cpp of the
// dictionary (which is processed by the actual compiler)
#if ! defined(G__DICTIONARY) && ! defined(__cling__)
#define __cling__
#define __cling__defined_in_GAUDIGSLMATH_H
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

#ifdef __cling__defined_in_GAUDIGSLMATH_H
#undef __cling__
#undef __cling__defined_in_GAUDIGSLMATH_H
#endif

// ============================================================================
// The
// ============================================================================
#endif // GAUDIGSL_GAUDIGSLMATH_H
// ============================================================================
