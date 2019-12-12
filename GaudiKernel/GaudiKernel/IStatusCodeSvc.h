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
#ifndef GAUDIKERNEL_ISTATUSCODESVC_H
#define GAUDIKERNEL_ISTATUSCODESVC_H

#include "GaudiKernel/IService.h"
#include <string>

class GAUDI_API IStatusCodeSvc : virtual public IService {

public:
  /// InterfaceID
  DeclareInterfaceID( IStatusCodeSvc, 2, 0 );

  virtual void regFnc( const std::string& fnc, const std::string& lib ) = 0;

  virtual void list() const = 0;

  virtual bool suppressCheck() const = 0;
};

#endif
