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

#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/DataObject.h>

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
  virtual ~HistoryObj() = default;

  const CLID&        clID() const override { return classID(); }
  static const CLID& classID();
  static std::string convert_string( const std::string& );

  virtual std::ostream& dump( std::ostream&, bool isXML = false, int indent = 0 ) const = 0;

  // Output stream.
  friend std::ostream& operator<<( std::ostream& lhs, const HistoryObj& rhs ) { return rhs.dump( lhs, false, 0 ); }

protected:
  virtual void indent( std::ostream&, int ) const;
};
