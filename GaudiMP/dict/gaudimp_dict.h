// redefined anyway in features.h by _GNU_SOURCE
#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE

// ============================================================================
// Python must always be the first.
#ifndef __APPLE__
#include <Python.h>
#endif // not __APPLE__

#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/Chrono.h"
#include "GaudiKernel/ChronoEntity.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/Property.h"
#include "GaudiKernel/Range.h"
#include "GaudiKernel/SerializeSTL.h"
#include "GaudiKernel/Stat.h"
#include "GaudiKernel/StatEntity.h"
#include "GaudiKernel/StringKey.h"

#ifdef _WIN32
#include "GaudiKernel/GaudiHandle.h"
#endif

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#pragma warning( disable : 177 )
// disable icc warning #1125: function "C::X()" is hidden by "Y::X" -- virtual function override intended?
#pragma warning( disable : 1125 )
#endif

#include "GaudiMP/PyROOTPickle.h"
#include "GaudiMP/TESSerializer.h"

#ifdef _WIN32
#pragma warning( disable : 4345 )
#pragma warning( disable : 4624 )
#endif

#ifdef __ICC
// disable icc warning #191: type qualifier is meaningless on cast type
// ... a lot of noise produced by the dictionary
#pragma warning( disable : 191 )
#endif
