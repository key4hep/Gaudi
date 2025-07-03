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
#pragma once

#include <Math/Vector4D.h>

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi {

  // NB : Please remember to give a simple doxygen comment for each typedef

  typedef ROOT::Math::PxPyPzEVector   XYZTVector;      ///< Cartesian 4 Vector
  typedef ROOT::Math::PxPyPzEVector   LorentzVector;   ///< Cartesian 4 Vector
  typedef ROOT::Math::XYZTVectorF     XYZTVectorF;     ///< Cartesian 4 Vector (float)
  typedef ROOT::Math::XYZTVectorF     LorentzVectorF;  ///< Cartesian 4 Vector (float)
  typedef ROOT::Math::PtEtaPhiEVector PtEtaPhiEVector; ///< Polar 4 Vector
} // namespace Gaudi
