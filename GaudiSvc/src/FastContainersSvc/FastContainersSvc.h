// $Id: FastContainersSvc.h,v 1.3 2006/12/04 17:15:14 mato Exp $
#ifndef FASTCONTAINERSSVC_FASTCONTAINERSSVC_H
#define FASTCONTAINERSSVC_FASTCONTAINERSSVC_H 1

// Include files
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IDataStoreAgent.h"
#include "GaudiKernel/TransientFastContainer.h"
#include "GaudiKernel/IRegistry.h"

/** @class FastContainersSvc FastContainersSvc.h FastContainersSvc/FastContainersSvc.h
 *
 *  Data service for objects of type TransientFastContainer.
 *
 *  Pointers to objects in this transient store are always valid. The instances of TransientFastContainer are cleared before each
 *  event.
 *
 *  @author Marco Clemencic
 *  @date   2006-05-03
 */
class FastContainersSvc : public extends1<DataSvc, IIncidentListener> {
protected:
  /// Standard service constructor
  FastContainersSvc(const std::string& name, ISvcLocator* svc);

  virtual ~FastContainersSvc(); ///< Destructor

public:
  // Overloaded DataSvc methods

  /// Initialize the service
  virtual StatusCode initialize();

  /// Initialize the service
  virtual StatusCode reinitialize();

  /// Finalize the service
  virtual StatusCode finalize();

  /// Reset all the instances of TransientFastContainerBase in the store.
  virtual void resetStore();

public:

  // Implementation of the IIncidentListener interface

  /// Handle the new incident.
  virtual void handle( const Incident& incident );

private:

  friend class SvcFactory<FastContainersSvc>;

  /// Name of the root node (property "RootDirName").
  std::string m_rootDirName;

  /// Internal data store agent used to find and clear objects of type TransientFastContainerBase.
  class StoreResetter: public IDataStoreAgent
  {
    virtual bool analyse(IRegistry *pRegistry, int /* level */) {
      TransientFastContainerBase* fc =
        dynamic_cast<TransientFastContainerBase*>(pRegistry->object());
      if ( 0 != fc ) fc->clear();
      return true;
    }
  } m_storeResetter;

};
#endif // FASTCONTAINERSSVC_FASTCONTAINERSSVC_H
