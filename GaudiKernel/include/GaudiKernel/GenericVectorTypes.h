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

//---------------------------------------------------------------------------------
/** @file GenericVectorTypes.h
 *
 *  General vectors
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#pragma once

// Include files
#include <Math/SVector.h>

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi {

  // NB : Please remember to give a simple doxygen comment for each typedef

  typedef ROOT::Math::SVector<double, 1> Vector1; ///< 1D Vector (double)
  typedef ROOT::Math::SVector<double, 2> Vector2; ///< 2D Vector (double)
  typedef ROOT::Math::SVector<double, 3> Vector3; ///< 3D Vector (double)
  typedef ROOT::Math::SVector<double, 4> Vector4; ///< 4D Vector (double)
  typedef ROOT::Math::SVector<double, 5> Vector5; ///< 5D Vector (double)
  typedef ROOT::Math::SVector<double, 6> Vector6; ///< 6D Vector (double)
  typedef ROOT::Math::SVector<double, 7> Vector7; ///< 7D Vector (double)
  typedef ROOT::Math::SVector<double, 8> Vector8; ///< 8D Vector (double)
  typedef ROOT::Math::SVector<double, 9> Vector9; ///< 9D Vector (double)

  typedef ROOT::Math::SVector<float, 1> Vector1F; ///< 1D Vector (float)
  typedef ROOT::Math::SVector<float, 2> Vector2F; ///< 2D Vector (float)
  typedef ROOT::Math::SVector<float, 3> Vector3F; ///< 3D Vector (float)
  typedef ROOT::Math::SVector<float, 4> Vector4F; ///< 4D Vector (float)
  typedef ROOT::Math::SVector<float, 5> Vector5F; ///< 5D Vector (float)
  typedef ROOT::Math::SVector<float, 6> Vector6F; ///< 6D Vector (float)
  typedef ROOT::Math::SVector<float, 7> Vector7F; ///< 7D Vector (float)
  typedef ROOT::Math::SVector<float, 8> Vector8F; ///< 8D Vector (float)
  typedef ROOT::Math::SVector<float, 9> Vector9F; ///< 9D Vector (float)
} // namespace Gaudi
