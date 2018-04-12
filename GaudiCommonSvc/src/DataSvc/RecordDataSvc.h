#ifndef DATASVC_RECORDDATASVC_H
#define DATASVC_RECORDDATASVC_H

#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IConversionSvc.h"
#include "GaudiKernel/IIncidentListener.h"

/** @class RecordDataSvc RecordDataSvc.h
 *
 *   A RecordDataSvc is the base class for event services
 *
 *   When a new datafile is opened the data service retrieves an incident
 *   from the persistency area (currently only from POOL) together with an opaque
 *   address describing the record. These history records are put onto the
 *   run-records datastore under the name of the logical file (FID):
 *   /Records/<FID>/.....
 *
 *   Hence, each history records can be addresses the same way:
 *   /Record/<FID>  (/EOR)
 *
 *   @author  M.Frank
 *   @version 1.0
 */
class RecordDataSvc : public extends<DataSvc, IIncidentListener>
{
public:
  StatusCode initialize() override;
  StatusCode finalize() override;

  /// IIncidentListener override: Inform that a new incident has occured
  void handle( const Incident& incident ) override;

  /// Standard Constructor
  RecordDataSvc( const std::string& name, ISvcLocator* svc );

  /// Standard Destructor
  ~RecordDataSvc() override = default;

protected:
  /// Load new record into the data store if necessary
  void registerRecord( const std::string& data, IOpaqueAddress* pAddr );

  /// Load dependent records into memory
  void loadRecords( IRegistry* pReg );

protected:
  Gaudi::Property<bool>        m_autoLoad{this, "AutoLoad", true, "autoload of records"};
  Gaudi::Property<std::string> m_incidentName{this, "IncidentName", "",
                                              "name of incident to be fired if new record arrives"};
  Gaudi::Property<std::string> m_saveIncidentName{this, "SaveIncident", "SAVE_RECORD", "name of the 'save' incident"};
  Gaudi::Property<std::string> m_persSvcName{this, "PersistencySvc", "PersistencySvc/RecordPersistencySvc",
                                             "name of the persistency service"};

  /// Pending new file incidents
  std::vector<std::string> m_incidents;
  /// Reference to the main data conversion service
  SmartIF<IConversionSvc> m_cnvSvc;
};
#endif // DATASVC_RECORDDATASVC_H
