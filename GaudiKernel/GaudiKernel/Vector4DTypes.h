
//---------------------------------------------------------------------------------
/** @file Vector4DTypes.h
 *
 *  4 vector typedefs
 *
 *  CVS Log :-
 *  $Id: Vector4DTypes.h,v 1.2 2007/01/25 18:30:27 hmd Exp $
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_VECTOR4DTYPES_H 
#define GAUDIKERNEL_VECTOR4DTYPES_H 1

// Include files
#include "Math/Vector4D.h"

/** @namespace Gaudi
 *  
 *  General Gaudi namespace
 * 
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi 
{

  // NB : Please remember to give a simple doxygen comment for each typedef

  typedef ROOT::Math::PxPyPzEVector     XYZTVector;     ///< Cartesian 4 Vector
  typedef ROOT::Math::PxPyPzEVector     LorentzVector;  ///< Cartesian 4 Vector
  typedef ROOT::Math::XYZTVectorF       XYZTVectorF;    ///< Cartesian 4 Vector (float)
  typedef ROOT::Math::XYZTVectorF       LorentzVectorF; ///< Cartesian 4 Vector (float)
  typedef ROOT::Math::PtEtaPhiEVector   PtEtaPhiEVector; ///< Polar 4 Vector

}
#endif // EVENT_VECTOR4DTYPES_H



