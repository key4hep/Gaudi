//====================================================================
//	OutputStreamAgent.cpp
//--------------------------------------------------------------------
//
//	Package    :  (The LHCb PersistencySvc service)
//
//  Description: Implementation of the OutputStream Agent
//
//	Author     : M.Frank
//  Created    : 13/1/99
//	Changes    :
//
//====================================================================
#define PERSISTENCYSVC_OUTPUTSTREAMAGENT_CPP

// Framework includes
#include "OutputStreamAgent.h"
#include "GaudiKernel/IDataManagerSvc.h"
#include "OutputStream.h"

/// Standard Constructor
OutputStreamAgent::OutputStreamAgent( OutputStream* OutputStream ) : m_OutputStream( OutputStream ) {}

/// Analysis callback
bool OutputStreamAgent::analyse( IRegistry* pRegistry, int level )
{
  return m_OutputStream->collect( pRegistry, level );
}
