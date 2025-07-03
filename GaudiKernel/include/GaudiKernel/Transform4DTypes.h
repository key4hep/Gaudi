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

#include <Math/LorentzRotation.h>

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
