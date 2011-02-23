// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/Selector.cpp,v 1.2 2000/12/13 12:57:23 mato Exp $
//====================================================================
//  NTuple name space: Selector class implementation
//--------------------------------------------------------------------
//
//  Package    : Gaudi/NTupleSvc ( The LHCb Offline System)
//  Author     : M.Frank
//
//  +---------+----------------------------------------------+--------+
//  |    Date |                 Comment                      | Who    |
//  +---------+----------------------------------------------+--------+
//  | 10/10/00| Initial version.                             | MF     |
//  +---------+----------------------------------------------+--------+
//====================================================================
#define GAUDI_NTUPLEITEMS_CPP 1

#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/Selector.h"

/// Selector Initialisation
StatusCode NTuple::Selector::initialize(NTuple::Tuple* /* nt */ )    {
  return StatusCode::SUCCESS;
}

/// Overloaded callback from SelectStatement
bool NTuple::Selector::operator()(void* nt)   {
  DataObject* p = (DataObject*)nt;
  bool result = false;
  try   {
    NTuple::Tuple* tuple = dynamic_cast<NTuple::Tuple*>(p);
    if ( 0 != tuple )   {
      if ( m_firstCall )    {
        m_status = initialize(tuple);
        m_firstCall = false;
      }
      if ( m_status.isSuccess() )  {
        result = this->operator()(tuple);
      }
    }
  }
  catch(...)    {
  }
  return result;
}

/// Specialized overload for N-tuples
bool NTuple::Selector::operator()(NTuple::Tuple* /* nt */ )  {
  return true;
}

