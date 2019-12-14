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
/** @file Point4DTypes.h
 *
 *  4D point typedefs
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
//---------------------------------------------------------------------------------

#ifndef GAUDIKERNEL_POINT4DTYPES_H
#define GAUDIKERNEL_POINT4DTYPES_H 1

// Include files
#include "GaudiKernel/Vector4DTypes.h"

/** @namespace Gaudi
 *
 *  General Gaudi namespace
 *
 *  @author Juan PALACIOS
 *  @date   2005-11-21
 */
namespace Gaudi {

  /** 4D point (space-time)
   *  @todo use Point4D from MathCore
   */
  typedef LorentzVector XYZTPoint;
} // namespace Gaudi

#endif // KERNEL_POINT4DTYPES_H
