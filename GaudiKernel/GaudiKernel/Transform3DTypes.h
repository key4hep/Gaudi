
//---------------------------------------------------------------------------------
/** @file Transform3DTypes.h
 *
 *  3D transformation typedefs
 *
 *  CVS Log :-
 *  $Id: Transform3DTypes.h,v 1.1 2006/12/08 15:00:02 hmd Exp $
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_TRANSFORM3DTYPES_H
#define GAUDIKERNEL_TRANSFORM3DTYPES_H 1

#ifdef __INTEL_COMPILER         // Disable ICC remark from Math headers
  #pragma warning(push)
  #pragma warning(disable:1572) // Floating-point equality and inequality comparisons are unreliable
#endif

// Include files
#include "Math/Rotation3D.h"
#include "Math/RotationX.h"
#include "Math/RotationY.h"
#include "Math/RotationZ.h"
#include "Math/AxisAngle.h"
#include "Math/Transform3D.h"
#include "Math/EulerAngles.h"
#include "Math/Quaternion.h"
#include "Math/Vector3D.h"

#ifdef __INTEL_COMPILER
  #pragma warning(pop)
#endif

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi
{

  // NB : Please remember to give a simple doxygen comment for each tyedef

  typedef ROOT::Math::Rotation3D      Rotation3D;      ///< 3D rotation
  typedef ROOT::Math::EulerAngles     EulerAngles;     ///< 3D Euler Angles
  typedef ROOT::Math::Quaternion      Quaternion;      ///< 3D Quaternion
  typedef ROOT::Math::AxisAngle       AxisAngle;       ///< 3D Axis Angle
  typedef ROOT::Math::RotationX       RotationX;       ///< Rotation about X axis
  typedef ROOT::Math::RotationY       RotationY;       ///< Rotation about Y axis
  typedef ROOT::Math::RotationZ       RotationZ;       ///< Rotation about Z axis
  typedef ROOT::Math::Transform3D     Transform3D;     ///< General 3D transformation (rotation+translation)
  typedef ROOT::Math::XYZVector       TranslationXYZ;  ///< 3D translation

}

#endif // EVENT_TRANSFORM3DTYPES_H
