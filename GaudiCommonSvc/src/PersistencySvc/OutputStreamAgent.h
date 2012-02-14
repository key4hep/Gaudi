//====================================================================
//	OutputStreamAgent.h
//--------------------------------------------------------------------
//
//	Package    :  (The LHCb PersistencySvc service)
//
//  Description: Definition the OutputStream Agent
//
//	Author     : M.Frank
//  Created    : 13/1/99
//	Changes    : 
//
//====================================================================
#ifndef PERSISTENCYSVC_OUTPUTSTREAMAGENT_H
#define PERSISTENCYSVC_OUTPUTSTREAMAGENT_H

// Framework includes
#include "GaudiKernel/IDataStoreAgent.h"
// Foreward declarations
class IRegistry;
class OutputStream;

/** @name The OutputStreamAgent class.
 
  Data store Agent to traverse data store trees and select all
  items to be written to the output file.
 
  @author Markus Frank
 */
class OutputStreamAgent : virtual public IDataStoreAgent  {
  /// Reference to data writer
  OutputStream*  m_OutputStream;
public:
  /// Standard Constructor
  OutputStreamAgent(OutputStream* OutputStream);
  /// Standard Destructor
  virtual ~OutputStreamAgent();
  /// Analysis callback
  virtual bool analyse(IRegistry* dir, int level);
};
#endif // PERSISTENCYSVC_OUTPUTSTREAMAGENT_H
