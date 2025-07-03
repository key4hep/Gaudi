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

#include <GaudiKernel/GaudiException.h>

/** @class UpdateManagerException UpdateManagerException.h GaudiKernel/UpdateManagerException.h
 *
 *  Exception thrown by the UpdateManagerSvc when something goes wrong.
 *
 *  @see GaudiException
 *
 *  @author Marco Clemencic
 *  @date   2005-12-14
 */
class GAUDI_API UpdateManagerException : public GaudiException {
public:
  UpdateManagerException( const std::string& Message = "unspecified exception",
                          const std::string& Tag = "*UpdateManagerSvc*", const StatusCode& Code = StatusCode::FAILURE )
      : GaudiException( Message, Tag, Code ) {}

  UpdateManagerException( const std::string& Message, const std::string& Tag, const StatusCode& Code,
                          const GaudiException& Ex )
      : GaudiException( Message, Tag, Code, Ex ) {}

  ~UpdateManagerException() throw() override = default;

  UpdateManagerException* clone() const override { return new UpdateManagerException( *this ); }
};
