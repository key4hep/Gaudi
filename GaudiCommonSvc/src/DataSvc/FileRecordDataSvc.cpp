/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
//====================================================================
//	FileRecordDataSvc.cpp
//--------------------------------------------------------------------
//
//	Package    : System ( The LHCb Offline System)
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
#define DATASVC_FILERECORDDATASVC_CPP

#include "RecordDataSvc.h"

/** @class FileRecordDataSvc
 *
 *   A FileRecordDataSvc is the base class for event services
 *
 *   When a new datafile is opened the data service retrieves an incident
 *   from the persistency area (currently only from POOL) together with an opaque
 *   address describing the run record. These history records are put onto the
 *   run-records datastore under the name of the logical file (FID):
 *   /FileRecords/<FID>/.....
 *
 *   Hence, each history records can be addresses the same way:
 *   /FileRecord/<FID>
 *
 *   @author  M.Frank
 *   @version 1.0
 */
class FileRecordDataSvc : public RecordDataSvc {
public:
  /// Standard Constructor
  FileRecordDataSvc( const std::string& nam, ISvcLocator* svc ) : RecordDataSvc( nam, svc ) {
    m_cnvSvc           = nullptr;
    m_rootName         = "/FileRecords";
    m_incidentName     = "NEW_FILE_RECORD";
    m_saveIncidentName = "SAVE_FILE_RECORD";
    m_persSvcName      = "PersistencySvc/FileRecordPersistencySvc";
  }
};

// Instantiation of a static factory class used by clients to create instances of this service
DECLARE_COMPONENT( FileRecordDataSvc )
