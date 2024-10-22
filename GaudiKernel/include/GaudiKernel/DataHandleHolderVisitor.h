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
#ifndef GAUDIKERNEL_DATAHANDLEHOLDERVISTOR
#define GAUDIKERNEL_DATAHANDLEHOLDERVISTOR 1

#include <GaudiKernel/DataObjID.h>
#include <GaudiKernel/IDataHandleHolder.h>
#include <map>
#include <set>
#include <string>
#include <vector>

class DHHVisitor : public IDataHandleVisitor {
public:
  using Owners = std::vector<const IDataHandleHolder*>;

  DHHVisitor( DataObjIDColl& ido, DataObjIDColl& odo );

  const DataObjIDColl& ignoredInpKeys() const { return m_ign_i; }
  const DataObjIDColl& ignoredOutKeys() const { return m_ign_o; }

  void visit( const IDataHandleHolder* visitee ) override;

  const std::map<DataObjID, std::set<const IDataHandleHolder*>>& owners() const { return m_owners; }

  std::vector<const IDataHandleHolder*> owners_of( const DataObjID& id ) const;

  std::vector<std::string> owners_names_of( const DataObjID& id, bool with_main = false ) const;

  /// return true if no DataHandle was found
  bool empty() const;

private:
  DataObjIDColl &m_ido, &m_odo;
  DataObjIDColl  m_ign_i, m_ign_o;

  std::map<DataObjID, std::set<const IDataHandleHolder*>> m_owners;

  std::string m_initialName;

  MsgStream& report( MsgStream& stream ) const;

  friend MsgStream& operator<<( MsgStream& stream, const DHHVisitor& visitor ) { return visitor.report( stream ); }
};

#endif
