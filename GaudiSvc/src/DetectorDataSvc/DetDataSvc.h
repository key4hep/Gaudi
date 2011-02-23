// $Id: DetDataSvc.h,v 1.14 2008/10/27 19:22:21 marcocle Exp $
#ifndef DETECTORDATASVC_DETDATASVC_H
#define DETECTORDATASVC_DETDATASVC_H

// Base classes
#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IDetDataSvc.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/Time.h"

// Forward declarations
class StatusCode;
class IAddressCreator;

///---------------------------------------------------------------------------
/** @class DetDataSvc DetDataSvc.h DetectorDataSvc/DetDataSvc.h

    A DataSvc specialized in detector data.

    @author Marco Clemencic (previous author unknown)

*///--------------------------------------------------------------------------

class DetDataSvc  : public extends2<DataSvc, IDetDataSvc, IIncidentListener>
{

  friend class SvcFactory<DetDataSvc>;

  // unhides DataSvc updateObject methods
  using DataSvc::updateObject;

public:

  // Overloaded DataSvc methods

  /// Initialize the service
  virtual StatusCode initialize();

  /// Initialize the service
  virtual StatusCode reinitialize();

  /// Finalize the service
  virtual StatusCode finalize();

  /// Remove all data objects in the data store.
  virtual StatusCode clearStore();

  /// Update object
  virtual StatusCode updateObject( DataObject* toUpdate );

  /// Standard Constructor
  DetDataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~DetDataSvc();

private:
  /// Deal with Detector Description initialization
  StatusCode setupDetectorDescription();

public:

  // Implementation of the IDetDataSvc interface

  /// Check if the event time has been set.
  /// Kept for backward compatibility, returns always true.
  virtual bool validEventTime() const ;

  /// Get the event time
  virtual const Gaudi::Time& eventTime() const ;

  /// Set the new event time
  virtual void setEventTime( const Gaudi::Time& time );

public:

  // Implementation of the IIncidentListener interface

  /// Inform that a new incident has occured
  virtual void handle( const Incident& );

private:

  /// Detector Data Persistency Storage type
  int              m_detStorageType;

  /// Location of detector Db (filename,URL)
  std::string      m_detDbLocation;

  /// Name of the root node of the detector
  std::string      m_detDbRootName;

  /// Name of the persistency service.
  std::string      m_persistencySvcName;

  /// Flag to control if the persistency is required
  bool             m_usePersistency;

  /// Current event time
  Gaudi::Time        m_eventTime;

  /// Address Creator to be used
  SmartIF<IAddressCreator> m_addrCreator;

};

#endif // DETECTORDATASVC_DETDATASVC_H
