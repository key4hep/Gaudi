// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiKernel/src/Lib/HistoryObj.cpp,v 1.1 2004/07/14 18:13:58 leggett Exp $
//====================================================================
//      History.cpp
//--------------------------------------------------------------------
//
//  Description: Implementation of HistoryObj base class
//
//  Author     : Charles Leggett
//====================================================================

#define GAUDIKERNEL_HISTORYOBJ_CPP

#include "GaudiKernel/HistoryObj.h"

HistoryObj::HistoryObj() {

}

const CLID& 
HistoryObj::classID() { 

  static CLID CLID_HistoryObj = 86452397;
  return CLID_HistoryObj; 

}
