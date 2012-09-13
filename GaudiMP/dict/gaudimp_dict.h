// $Id:
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.37 $
// ============================================================================
// Python must always be the first.
#include <Python.h>

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/PropertyCallbackFunctor.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiKernel/StringKey.h"
#include "GaudiKernel/Range.h"

#ifdef _WIN32
#include "GaudiKernel/GaudiHandle.h"
#endif

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#pragma warning(disable:177)
// disable icc warning #1125: function "C::X()" is hidden by "Y::X" -- virtual function override intended?
#pragma warning(disable:1125)
#endif

#include "GaudiMP/PyROOTPickle.h"
#include "GaudiMP/TESSerializer.h"

#ifdef _WIN32
#pragma warning ( disable : 4345 )
#pragma warning ( disable : 4624 )
#endif

#ifdef __ICC
// disable icc warning #191: type qualifier is meaningless on cast type
// ... a lot of noise produced by the dictionary
#pragma warning(disable:191)
#endif

// ============================================================================
// The END
// ============================================================================
