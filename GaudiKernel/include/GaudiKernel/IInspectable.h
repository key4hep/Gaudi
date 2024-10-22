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
#ifndef GAUDIKERNEL_IINSPECTABLE_H
#define GAUDIKERNEL_IINSPECTABLE_H

// Include Files
#include <GaudiKernel/Kernel.h>

// Forward declaration
class IInspector;
class IInspectable;

/** @class IInspectable IInspectable.h GaudiKernel/IInspectable.h

    Interface definition of an inspectable object.
    The IInspectable interface must be implemented by all objects that are to be inspected

*/
class GAUDI_API IInspectable {
public:
  /// destructor
  virtual ~IInspectable() = default;

  /** return FALSE if doesn't accept Inspecetor so that diagnostic
message can be printed. Calls the inspect member function of
the Inspector on all the members it wants to inspect.
*/
  virtual bool acceptInspector( IInspector* pI ) = 0;
  /** return FALSE if doesn't accept Inspecetor so that diagnostic
message can be printed. Calls the inspect member function of
the Inspector on all the members it wants to inspect.
*/
  virtual bool acceptInspector( IInspector* pI ) const = 0;
};
#endif // GAUDIKERNEL_IINSPECTABLE_H
