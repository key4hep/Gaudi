/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/

//---------------------------------------------------------------------------------
/** @file Point3DTypes.h
 *
 *  3D point typedefs
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_POINT3DTYPES_H
#define GAUDIKERNEL_POINT3DTYPES_H 1

// Include files
#include "Math/Point3D.h"

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi {

  // NB : Please remember to give a simple doxygen comment for each tyedef

  typedef ROOT::Math::XYZPoint       XYZPoint;       ///< 3D cartesian point (double)
  typedef ROOT::Math::Polar3DPoint   Polar3DPoint;   ///< 3D polar point (double)
  typedef ROOT::Math::RhoEtaPhiPoint RhoEtaPhiPoint; ///< 3D RhoEtaPhi point (double)
  typedef ROOT::Math::RhoZPhiPoint   RhoZPhiPoint;   ///< 3D RhoZPhi point (double)

  typedef ROOT::Math::XYZPointF       XYZPointF;       ///< 3D cartesian point (float)
  typedef ROOT::Math::Polar3DPointF   Polar3DPointF;   ///< 3D polar point (float)
  typedef ROOT::Math::RhoEtaPhiPointF RhoEtaPhiPointF; ///< 3D RhoEtaPhi point (float)
  typedef ROOT::Math::RhoZPhiPointF   RhoZPhiPointF;   ///< 3D RhoZPhi point (float)
} // namespace Gaudi
#endif // KERNEL_POINT3DTYPES_H
