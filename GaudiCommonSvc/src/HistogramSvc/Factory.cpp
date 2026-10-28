/***********************************************************************************\
* (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

// Framework includes
#include <GaudiKernel/ObjectFactory.h>

#include <GaudiCommonSvc/H1D.h>
#include <GaudiCommonSvc/H2D.h>
#include <GaudiCommonSvc/H3D.h>
#include <GaudiCommonSvc/HistogramSvc.h>
#include <GaudiCommonSvc/P1D.h>
#include <GaudiCommonSvc/P2D.h>

namespace {
  using H1D = Gaudi::Histogram1D;
  using P1D = Gaudi::Profile1D;
  using H2D = Gaudi::Histogram2D;
  using P2D = Gaudi::Profile2D;
  using H3D = Gaudi::Histogram3D;
} // namespace

DECLARE_COMPONENT( HistogramSvc )

DECLARE_DATAOBJECT_FACTORY( H1D )
DECLARE_DATAOBJECT_FACTORY( P1D )
DECLARE_DATAOBJECT_FACTORY( H2D )
DECLARE_DATAOBJECT_FACTORY( P2D )
DECLARE_DATAOBJECT_FACTORY( H3D )
