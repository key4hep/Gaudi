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
#pragma once

// Include files
#include "Math/AxisAngle.h"
#include "Math/EulerAngles.h"
#include "Math/Quaternion.h"
#include "Math/Rotation3D.h"
#include "Math/RotationX.h"
#include "Math/RotationY.h"
#include "Math/RotationZ.h"
#include "Math/Transform3D.h"
#include "Math/Vector3D.h"

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi {
  typedef ROOT::Math::Rotation3D  Rotation3D;     ///< 3D rotation
  typedef ROOT::Math::EulerAngles EulerAngles;    ///< 3D Euler Angles
  typedef ROOT::Math::Quaternion  Quaternion;     ///< 3D Quaternion
  typedef ROOT::Math::AxisAngle   AxisAngle;      ///< 3D Axis Angle
  typedef ROOT::Math::RotationX   RotationX;      ///< Rotation about X axis
  typedef ROOT::Math::RotationY   RotationY;      ///< Rotation about Y axis
  typedef ROOT::Math::RotationZ   RotationZ;      ///< Rotation about Z axis
  typedef ROOT::Math::Transform3D Transform3D;    ///< General 3D transformation (rotation+translation)
  typedef ROOT::Math::XYZVector   TranslationXYZ; ///< 3D translation
} // namespace Gaudi
