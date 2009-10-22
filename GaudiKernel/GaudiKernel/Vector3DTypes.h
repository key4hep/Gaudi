
//---------------------------------------------------------------------------------
/** @file Vector3DTypes.h
 *
 *  3D vector typedefs
 *
 *  CVS Log :-
 *  $Id: Vector3DTypes.h,v 1.1 2006/12/08 15:00:03 hmd Exp $
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_VECTOR3DTYPES_H 
#define GAUDIKERNEL_VECTOR3DTYPES_H 1

// Include files
#include "Math/Vector3D.h"

/** @namespace Gaudi
 *  
 *  General Gaudi namepace
 * 
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi 
{

  // NB : Please remember to give a simple doxygen comment for each tyedef

  typedef ROOT::Math::XYZVector            XYZVector;        ///<  Cartesian 3D vector (double)
  typedef ROOT::Math::Polar3DVector        Polar3DVector;    ///<  Polar 3D vector (double)
  typedef ROOT::Math::RhoEtaPhiVector      RhoEtaPhiVector;  ///<  RhoEtaPhi 3D vector (double)
  typedef ROOT::Math::RhoZPhiVector        RhoZPhiVector;    ///<  RhoZPhi 3D vector (double)
  typedef ROOT::Math::XYZVectorF           XYZVectorF;       ///<  Cartesian 3D vector (float)
  typedef ROOT::Math::Polar3DVectorF       Polar3DVectorF;   ///<  Polar 3D vector (float)
  typedef ROOT::Math::RhoEtaPhiVectorF     RhoEtaPhiVectorF; ///<  RhoEtaPhi 3D vector (float)
  typedef ROOT::Math::RhoZPhiVectorF       RhoZPhiVectorF;   ///<  RhoZPhi 3D vector (float)

}
#endif // EVENT_VECTOR3DTYPES_H
