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
#ifndef GAUDIKERNEL_HISTORYOBJ_H
#define GAUDIKERNEL_HISTORYOBJ_H

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/DataObject.h"

#include <ostream>
#include <string>

class IVersHistoryObj;

/** @class HistoryObj HistoryObj.h GaudiKernel/HistoryObj.h

    Base class for History Objects

    @author Charles Leggett
    @date   Jul 13 2004

*/

class GAUDI_API HistoryObj : public DataObject {
public:
  HistoryObj()          = default;
  virtual ~HistoryObj() = default;

  const CLID&        clID() const override { return classID(); }
  static const CLID& classID();
  static std::string convert_string( const std::string& );

  virtual void dump( std::ostream&, const bool isXML = false, int indent = 0 ) const = 0;

protected:
  virtual void indent( std::ostream&, int ) const;
};

// Output stream.
GAUDI_API std::ostream& operator<<( std::ostream& lhs, const HistoryObj& rhs );

#endif
