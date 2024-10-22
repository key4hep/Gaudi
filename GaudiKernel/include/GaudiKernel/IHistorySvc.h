/***********************************************************************************\
* (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#ifndef GAUDIKERNEL_IHISTORYSVC_H
#define GAUDIKERNEL_IHISTORYSVC_H

#include <GaudiKernel/ClassID.h>
#include <GaudiKernel/IService.h>

#include <set>
#include <string>

class JobHistory;

class ServiceHistory;
class IService;

namespace Gaudi {
  class Algorithm;
}
class AlgorithmHistory;

class IAlgTool;
class AlgToolHistory;

class DataHistory;
class HistoryObj;

/** @class IHistorySvc IHistorySvc.h GaudiKernel/IHistorySvc.h

    Definition of the IHistorySvc interface class

    @author Charles Leggett
*/

class GAUDI_API IHistorySvc : virtual public IService {
public:
  /// InterfaceID
  DeclareInterfaceID( IHistorySvc, 2, 0 );

  // Job level objects
  virtual StatusCode  registerJob()          = 0;
  virtual StatusCode  listProperties() const = 0;
  virtual JobHistory* getJobHistory() const  = 0;

  // Service level objects
  virtual StatusCode      registerSvc( const IService& )                        = 0;
  virtual StatusCode      listProperties( const IService& ) const               = 0;
  virtual ServiceHistory* getServiceHistory( const IService& ) const            = 0;
  virtual void            getServiceHistory( std::set<ServiceHistory*>& ) const = 0;

  // Algorithm level objects
  virtual StatusCode        registerAlg( const Gaudi::Algorithm& )              = 0;
  virtual StatusCode        listProperties( const Gaudi::Algorithm& ) const     = 0;
  virtual AlgorithmHistory* getAlgHistory( const Gaudi::Algorithm& ) const      = 0;
  virtual void              getAlgHistory( std::set<AlgorithmHistory*>& ) const = 0;

  // AlgTool level objects
  virtual StatusCode      registerAlgTool( const IAlgTool& )                    = 0;
  virtual StatusCode      listProperties( const IAlgTool& ) const               = 0;
  virtual AlgToolHistory* getAlgToolHistory( const IAlgTool& ) const            = 0;
  virtual void            getAlgToolHistory( std::set<AlgToolHistory*>& ) const = 0;

  // DataObj level objects
  virtual DataHistory* createDataHistoryObj( const CLID& id, const std::string& key, const std::string& storeName ) = 0;
  virtual DataHistory* getDataHistory( const CLID& id, const std::string& key, const std::string& storeName ) const = 0;
};

#endif // GAUDIKERNEL_IHISTORYSVC_H
