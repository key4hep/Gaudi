//====================================================================
//	SystemBase.h
//--------------------------------------------------------------------
//
//	Package    : System (The LHCb System service)
//
//  Description: Definition of Systems internals
//
//	Author     : M.Frank
//  Created    : 13/11/00
//	Changes    : 
//====================================================================
#ifndef GAUDI_SYSTEM_SYSTEMBASE_H
#define GAUDI_SYSTEM_SYSTEMBASE_H

namespace System    {
  /// Enumeration for fetching information
  enum InfoType   {   NoFetch, 
                      RemainTime, 
                      Times, 
                      ProcessBasics, 
                      PriorityBoost, 
                      Memory, 
                      Quota,
                      System, 
                      Modules, 
                      IO };
}
#endif // GAUDI_SYSTEM_SYSTEMBASE_H
