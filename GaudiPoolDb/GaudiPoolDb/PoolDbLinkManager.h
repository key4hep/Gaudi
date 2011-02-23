// $Id: PoolDbLinkManager.h,v 1.1.1.1 2004/01/16 14:05:03 mato Exp $
//====================================================================
//	PoolDbLinkManager definition
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
//
//  Description:
//
//	Author     : M.Frank
//====================================================================
#ifndef POOLDB_POOLDBLINKMANAGER_H
#define POOLDB_POOLDBLINKMANAGER_H

// C++ include files
#include <vector>
#include <string>
#include "GaudiKernel/Kernel.h"

// Framework include files
namespace pool  {
  class Token;
}

/** @class PoolDbLinkManager PoolDbLinkManager.h GaudiPoolDb/PoolDbLinkManager.h
  *
  * Description:
  * PoolDbLinkManager class implementation definition.
  *
  * @author Markus Frank
  * @version 1.0
  */
class GAUDI_API PoolDbLinkManager {
protected:
  /// Directory links
  std::vector<pool::Token*> m_refs;
  /// Logical links
  std::vector<std::string>  m_links;

public:
  /// Standard constructor
  PoolDbLinkManager();

  /// Standard destructor
  virtual ~PoolDbLinkManager();

  /// Access to token array
  std::vector<pool::Token*>& references()  {
    return m_refs;
  }
  /// Access to link array
  std::vector<std::string>& links()  {
    return m_links;
  }
};
#endif // POOLDB_POOLDBLINKMANAGER_H
