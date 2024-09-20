/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#include "RHistogramCnv.h"

#include <AIDA/IProfile1D.h>
#include <TProfile.h>
namespace RootHistCnv {
  typedef RHistogramCnv<TProfile, TProfile, Gaudi::HistogramBase> RootHistCnvP1DCnv;
  template <>
  const CLID& RootHistCnvP1DCnv::classID() {
    return CLID_ProfileH;
  }
  DECLARE_CONVERTER( RootHistCnvP1DCnv )
} // namespace RootHistCnv

#include <AIDA/IProfile2D.h>
#include <TProfile2D.h>
namespace RootHistCnv {
  typedef RHistogramCnv<TProfile2D, TProfile2D, Gaudi::HistogramBase> RootHistCnvP2DCnv;
  template <>
  const CLID& RootHistCnvP2DCnv::classID() {
    return CLID_ProfileH2;
  }
  DECLARE_CONVERTER( RootHistCnvP2DCnv )
} // namespace RootHistCnv

#include <AIDA/IHistogram1D.h>
#include <TH1D.h>
namespace RootHistCnv {
  typedef RHistogramCnv<TH1D, TH1, Gaudi::HistogramBase> RootHistCnvH1DCnv;
  template <>
  const CLID& RootHistCnvH1DCnv::classID() {
    return CLID_H1D;
  }
  DECLARE_CONVERTER( RootHistCnvH1DCnv )
} // namespace RootHistCnv

#include <AIDA/IHistogram2D.h>
#include <TH2D.h>
namespace RootHistCnv {
  typedef RHistogramCnv<TH2D, TH2, Gaudi::HistogramBase> RootHistCnvH2DCnv;
  template <>
  const CLID& RootHistCnvH2DCnv::classID() {
    return CLID_H2D;
  }
  DECLARE_CONVERTER( RootHistCnvH2DCnv )
} // namespace RootHistCnv

#include <AIDA/IHistogram3D.h>
#include <TH3D.h>
namespace RootHistCnv {
  typedef RootHistCnv::RHistogramCnv<TH3D, TH3, Gaudi::HistogramBase> RootHistCnvH3DCnv;
  template <>
  const CLID& RootHistCnvH3DCnv::classID() {
    return CLID_H3D;
  }
  DECLARE_CONVERTER( RootHistCnvH3DCnv )
} // namespace RootHistCnv
