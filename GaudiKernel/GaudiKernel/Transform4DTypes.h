//---------------------------------------------------------------------------------
/** @file Transform4DTypes.h
 *
 *  4D transformation typedefs
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_TRANSFORM4DTYPES_H
#define GAUDIKERNEL_TRANSFORM4DTYPES_H 1

// Include files
#include "Math/LorentzRotation.h"

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-23
 */
namespace Gaudi {
  // NB : Please remember to give a simple doxygen comment for each tyedef

  typedef ROOT::Math::LorentzRotation LorentzRotation; ///< Lorentz rotation
} // namespace Gaudi
#endif // KERNEL_TRANSFORM4DTYPES_H
