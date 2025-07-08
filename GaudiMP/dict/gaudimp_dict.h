/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// redefined anyway in features.h by _GNU_SOURCE
#undef _XOPEN_SOURCE
#undef _POSIX_C_SOURCE

// ============================================================================
// Python must always be the first.
#ifndef __APPLE__
#  include <Python.h>
#endif // not __APPLE__

#include <Gaudi/Property.h>
#include <GaudiKernel/Algorithm.h>
#include <GaudiKernel/Chrono.h>
#include <GaudiKernel/ChronoEntity.h>
#include <GaudiKernel/Range.h>
#include <GaudiKernel/SerializeSTL.h>
#include <GaudiKernel/Stat.h>
#include <GaudiKernel/StatEntity.h>
#include <GaudiKernel/StringKey.h>

#ifdef __ICC
// disable icc remark #177: declared but never referenced
#  pragma warning( disable : 177 )
// disable icc warning #1125: function "C::X()" is hidden by "Y::X" -- virtual function override intended?
#  pragma warning( disable : 1125 )
#endif

#include <GaudiMP/PyROOTPickle.h>
#include <GaudiMP/TESSerializer.h>

#ifdef __ICC
// disable icc warning #191: type qualifier is meaningless on cast type
// ... a lot of noise produced by the dictionary
#  pragma warning( disable : 191 )
#endif
