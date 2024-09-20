/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_ICLASSINFO_H
#define GAUDIKERNEL_ICLASSINFO_H

// Experiment specific include files
#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/Kernel.h>

/** @class IClassInfo IClassInfo.h GaudiKernel/IClassInfo.h

    Data base class allowing to store persistent type information
*/
class GAUDI_API IClassInfo {
public:
  /// Standard Destructor
  virtual ~IClassInfo() = default;
  /// Retrieve Pointer to class definition structure
  virtual const CLID& clID() const = 0;
};

#endif // GAUDIKERNEL_ICLASSINFO_H
