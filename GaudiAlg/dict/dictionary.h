/***********************************************************************************\
* (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations *
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

#include <GaudiPython/AlgDecorators.h>
#include <GaudiPython/PyAlg.h>
#ifdef AIDA_FOUND
#  include <GaudiPython/HistoDecorator.h>
#  ifdef CLHEP_FOUND
#    include <GaudiPython/TupleDecorator.h>
#  endif // CLHEP_FOUND
#endif   // AIDA_FOUND

#ifdef AIDA_FOUND
#  include <GaudiAlg/GaudiHistoAlg.h>
#  include <GaudiAlg/GaudiTupleAlg.h>
#  include <GaudiAlg/Print.h>
// Added to avoid warnings about inlined functions never implemented.
#  include <GaudiAlg/GaudiHistos.icpp>
#endif // AIDA_FOUND
#include <GaudiAlg/GaudiAlgorithm.h>
#include <GaudiAlg/GaudiCommon.h>
#include <GaudiAlg/GaudiHistoID.h>
#include <GaudiAlg/IErrorTool.h>
#include <GaudiAlg/IGenericTool.h>
#include <GaudiAlg/IHistoTool.h>
#include <GaudiAlg/ISequencerTimerTool.h>
#include <GaudiAlg/ITupleTool.h>
#include <GaudiAlg/Tuple.h>
