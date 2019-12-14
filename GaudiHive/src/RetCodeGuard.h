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
#ifndef GAUDIHIVE_RETCODEGUARD_H
#define GAUDIHIVE_RETCODEGUARD_H

#include "GaudiKernel/AppReturnCode.h"
#include "GaudiKernel/IProperty.h"

/// Helper class to set the application return code in case of early exit
/// (e.g. exception).
class RetCodeGuard {
public:
  inline RetCodeGuard( const SmartIF<IProperty>& appmgr, int retcode ) : m_appmgr( appmgr ), m_retcode( retcode ) {}
  inline void ignore() { m_retcode = Gaudi::ReturnCode::Success; }
  inline ~RetCodeGuard() {
    if ( UNLIKELY( Gaudi::ReturnCode::Success != m_retcode ) ) { Gaudi::setAppReturnCode( m_appmgr, m_retcode ); }
  }

private:
  SmartIF<IProperty> m_appmgr;
  int                m_retcode;
};

#endif
