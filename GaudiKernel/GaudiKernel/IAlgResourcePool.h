// $Id: IAlgManager.h,v 1.7 2008/06/02 14:20:38 marcocle Exp $
#ifndef GAUDIKERNEL_IALGRESOURCEPOOL_H
#define GAUDIKERNEL_IALGRESOURCEPOOL_H

// Framework includes
#include "GaudiKernel/IInterface.h"

// C++ includes
#include <string>
#include <list>

// Forward class declaration
class IAlgorithm;

/** @class IAlgResourcePool IAlgResourcePool.h GaudiKernel/IAlgResourcePool.h

    The IAlgResourcePool is the interface for managing algorithm instances,
    in particular if clones of the same algorithm exist.
    It as well manages the shared resources between different algorithms and 
    ensures that not two algorithms with the same non-thread-safe resource 
    needs are made available. The actual creation and deletion of algorithm 
    instances is done through the IAlgManager service. 

    @author Benedikt Hegner
    @version 1.0
*/
class GAUDI_API IAlgResourcePool: virtual public IInterface {
public:
  /// InterfaceID
  DeclareInterfaceID(IAlgResourcePool,1,0);

  /// Acquire a certain algorithm using its name 
  virtual StatusCode acquireAlgorithm(const std::string& name,IAlgorithm*& algo) = 0;
  /// Release a certain algorithm 
  virtual StatusCode releaseAlgorithm(const std::string& name, IAlgorithm*& algo) = 0;

  /// Get the flat list of algorithms
  virtual std::list<IAlgorithm*> getFlatAlgList() = 0;
  
  /// Begin Run
  virtual StatusCode beginRun() = 0;
  
  /// End Run
  virtual StatusCode endRun() = 0;
    
  /// Acquire a certain resource
  virtual StatusCode acquireResource(const std::string& name) = 0;
  /// Release a certrain resource
  virtual StatusCode releaseResource(const std::string& name) = 0;

};

#endif  // GAUDIKERNEL_IALGRESOURCEPOOL_H
