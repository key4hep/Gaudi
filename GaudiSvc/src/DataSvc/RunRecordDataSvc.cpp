//====================================================================
//      RunRecordDataSvc.cpp
//--------------------------------------------------------------------
//
//      Package    : System ( The LHCb Offline System)
//
//  Description: implementation of the Transient event data service.
//
//  Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who
// +---------+----------------------------------------------+---------
// | 10/12/08| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#define  DATASVC_RUNRECORDDATASVC_CPP

#include "GaudiKernel/SvcFactory.h"
#include "RecordDataSvc.h"



/** @class RunRecordDataSvc
 *
 *   A RunRecordDataSvc is the base class for event services
 *
 *   When a new datafile is opened the data service retrieves an incident
 *   from the persistency area (currently only from POOL) together with an opaque
 *   address describing the run record. These history records are put onto the
 *   run-records datastore under the name of the logical file (FID):
 *   /RunRecords/<FID>/.....
 *
 *   Hence, each history records can be addresses the same way:
 *   /RunRecord/<FID>
 *
 *   @author  M.Frank
 *   @version 1.0
 */
class RunRecordDataSvc  : public RecordDataSvc   {
 public:

  /// Standard Constructor
  RunRecordDataSvc(const std::string& nam, ISvcLocator* svc) : RecordDataSvc(nam,svc) {
    m_cnvSvc = 0;
    m_rootName = "/RunRecords";
    m_incidentName = "NEW_RUN_RECORD";
    m_saveIncidentName = "SAVE_RUN_RECORD";
    m_persSvcName = "PersistencySvc/RecordPersistencySvc";
  }

  /// Standard Destructor
  virtual ~RunRecordDataSvc() {}

};

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_SERVICE_FACTORY(RunRecordDataSvc)

