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
#ifndef __GAUDI_INTERFACES_IVALIDITY_H__
#define __GAUDI_INTERFACES_IVALIDITY_H__

#include "GaudiKernel/Kernel.h"
namespace Gaudi {
  class Time;
}

/// @class IValidity IValidity.h GaudiKernel/IValidity.h
/// Interface for objects with a validity
class GAUDI_API IValidity {
public:
  /// Check if the object is valid (it can be always invalid).
  virtual bool isValid() const = 0;

  /// Check if the object is valid for the specified point in time.
  virtual bool isValid( const Gaudi::Time& ) const = 0;

  /// Returns the start of validity of the object.
  virtual const Gaudi::Time& validSince() const = 0;

  /// Returns the end of validity of the object.
  virtual const Gaudi::Time& validTill() const = 0;

  /// Set the interval of validity of the object.
  virtual void setValidity( const Gaudi::Time&, const Gaudi::Time& ) = 0;

  /// Sets the start of validity of the object.
  virtual void setValiditySince( const Gaudi::Time& ) = 0;

  /// Sets the end of validity of the object.
  virtual void setValidityTill( const Gaudi::Time& ) = 0;

  /// destructor
  virtual ~IValidity() = default;
};

#endif //  __GAUDI_INTERFACES_IVALIDITY_H__
