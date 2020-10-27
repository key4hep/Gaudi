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
