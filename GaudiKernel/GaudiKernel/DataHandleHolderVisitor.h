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
#include <set>

class DHHVisitor : public IDataHandleVisitor {
public:
  DHHVisitor( DataObjIDColl& ido, DataObjIDColl& odo );

  const DataObjIDColl& ignoredInpKeys() const { return m_ign_i; }
  const DataObjIDColl& ignoredOutKeys() const { return m_ign_o; }

  void visit( const IDataHandleHolder* visitee ) override;

private:
  DataObjIDColl &m_ido, &m_odo;
  DataObjIDColl  m_ign_i, m_ign_o;
};

#endif
