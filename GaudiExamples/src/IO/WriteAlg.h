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
#ifndef GAUDIEXAMPLE_WRITEALG_H
#define GAUDIEXAMPLE_WRITEALG_H

// Framework include files
#include "GaudiExamples/Counter.h"
#include "GaudiKernel/Algorithm.h"

/** @class WriteAlg WriteAlg.h

    WriteAlg class for the RootIOExample

    @author Markus Frank
*/

class WriteAlg : public Algorithm {
protected:
  /// Reference to run records data service
  SmartIF<IDataProviderSvc> m_recordSvc;

  /// Reference to event counter
  Gaudi::Examples::Counter* m_evtCount = nullptr;

  /// Register data leaf
  StatusCode put( IDataProviderSvc* s, const std::string& path, DataObject* pObj );

public:
  /// Constructor: A constructor of this form must be provided.
  WriteAlg( const std::string& name, ISvcLocator* pSvcLocator ) : Algorithm( name, pSvcLocator ) {}
  /// Initialize
  StatusCode initialize() override;
  /// Finalize
  StatusCode finalize() override;
  /// Event callback
  StatusCode execute() override;
};

#endif // GAUDIEXAMPLE_WRITEALG_H
