#ifndef DETECTORDATASVC_DETDATASVC_H
#define DETECTORDATASVC_DETDATASVC_H

// Base classes
//#include "GaudiKernel/DataSvc.h"
// D. Piparo: Change to the new thread safe version
#include "GaudiKernel/TsDataSvc.h"
#include "GaudiKernel/IDetDataSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Time.h"

// Forward declarations
class StatusCode;
class IAddressCreator;

///---------------------------------------------------------------------------
/** @class DetDataSvc DetDataSvc.h DetectorDataSvc/DetDataSvc.h

    A DataSvc specialized in detector data.
    Now inherits from TsDataSvc, a thread safe version of the DataSvc, which
    allows concurrent retrieval of DataObjects.

    @author Marco Clemencic (previous author unknown)
    @author Danilo Piparo 

*///--------------------------------------------------------------------------

class DetDataSvc  : public extends2<TsDataSvc, IDetDataSvc, IIncidentListener>
{

  // unhides DataSvc updateObject methods
  using TsDataSvc::updateObject;

public:

  // Overloaded DataSvc methods

  /// Initialize the service
  StatusCode initialize() override;

  /// Initialize the service
  StatusCode reinitialize() override;

  /// Finalize the service
  StatusCode finalize() override;

  /// Remove all data objects in the data store.
  StatusCode clearStore() override;

  /// Update object
  StatusCode updateObject( DataObject* toUpdate ) override;

  /// Standard Constructor
  DetDataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  ~DetDataSvc() override = default;

private:
  /// Deal with Detector Description initialization
  StatusCode setupDetectorDescription();

public:

  // Implementation of the IDetDataSvc interface

  /// Check if the event time has been set.
  /// Kept for backward compatibility, returns always true.
  bool validEventTime() const override;

  /// Get the event time
  const Gaudi::Time& eventTime() const override;

  /// Set the new event time
  void setEventTime( const Gaudi::Time& time ) override;

public:

  // Implementation of the IIncidentListener interface

  /// Inform that a new incident has occured
  void handle( const Incident& ) override;

private:

  /// Detector Data Persistency Storage type
  int              m_detStorageType = XML_StorageType;

  /// Location of detector Db (filename,URL)
  std::string      m_detDbLocation = "empty";

  /// Name of the root node of the detector
  std::string      m_detDbRootName = "dd";

  /// Name of the persistency service.
  std::string      m_persistencySvcName = "DetectorPersistencySvc";

  /// Flag to control if the persistency is required
  bool             m_usePersistency = false;

  /// Current event time
  Gaudi::Time        m_eventTime = 0;

  /// Address Creator to be used
  SmartIF<IAddressCreator> m_addrCreator = nullptr;

};

#endif // DETECTORDATASVC_DETDATASVC_H
