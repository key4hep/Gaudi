// $Id: RecordDataSvc.h,v 1.1 2008/12/10 18:37:36 marcocle Exp $
#ifndef DATASVC_RECORDDATASVC_H
#define DATASVC_RECORDDATASVC_H

#include "GaudiKernel/DataSvc.h"
#include "GaudiKernel/IIncidentListener.h"
class IConversionSvc;


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
class RecordDataSvc  : public extends1<DataSvc, IIncidentListener> {
 protected:
  typedef std::vector<std::string> StringV;
 public:
  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();

  /// IIncidentListener override: Inform that a new incident has occured
  virtual void handle(const Incident& incident);

  /// Standard Constructor
  RecordDataSvc(const std::string& name, ISvcLocator* svc);

  /// Standard Destructor
  virtual ~RecordDataSvc();

 protected:
  /// Load new record into the data store if necessary
  void registerRecord(const std::string& data, IOpaqueAddress* pAddr);

  /// Load dependent records into memory
  void loadRecords(IRegistry* pReg);

 protected:
  /// Property: autoload of records (default: true)
  bool            m_autoLoad;
  ///Property:  name of incident to be fired if new record arrives
  std::string     m_incidentName;
  /// Property: name of the "save" incident
  std::string     m_saveIncidentName;
  /// Property: name of the persistency service
  std::string     m_persSvcName;
  /// Pending new file incidents
  StringV         m_incidents;
  /// Reference to the main data conversion service
  IConversionSvc* m_cnvSvc;
};
#endif // DATASVC_RECORDDATASVC_H
