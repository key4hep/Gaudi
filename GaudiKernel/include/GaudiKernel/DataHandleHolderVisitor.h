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
#ifndef GAUDIKERNEL_DATAHANDLEHOLDERVISTOR
#define GAUDIKERNEL_DATAHANDLEHOLDERVISTOR 1

#include "GaudiKernel/DataObjID.h"
#include "GaudiKernel/IDataHandleHolder.h"
#include <map>
#include <set>
#include <string>
#include <vector>

class DHHVisitor : public IDataHandleVisitor {
public:
  DHHVisitor( DataObjIDColl& ido, DataObjIDColl& odo );

  const DataObjIDColl& ignoredInpKeys() const { return m_ign_i; }
  const DataObjIDColl& ignoredOutKeys() const { return m_ign_o; }

  void visit( const IDataHandleHolder* visitee ) override;

  const std::vector<std::pair<DataObjID, const IDataHandleHolder*>>& all() const { return m_all; }

  const std::map<DataObjID, std::set<const IDataHandleHolder*>>& src_i() const { return m_src_i; }
  const std::map<DataObjID, std::set<const IDataHandleHolder*>>& src_o() const { return m_src_o; }

private:
  DataObjIDColl &m_ido, &m_odo;
  DataObjIDColl  m_ign_i, m_ign_o;

  std::vector<std::pair<DataObjID, const IDataHandleHolder*>> m_all;
  std::map<DataObjID, std::set<const IDataHandleHolder*>>     m_src_i, m_src_o;
};

#endif
