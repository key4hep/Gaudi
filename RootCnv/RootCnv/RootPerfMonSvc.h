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
//	RootPerfMonSvc definition
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
#ifndef GAUDIROOTCNV_GAUDIROOTPERFMONSVC_H
#define GAUDIROOTCNV_GAUDIROOTPERFMONSVC_H

#include <memory>
// Framework include files
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/Service.h"

#include "TFile.h"
#include "TTree.h"

#include "RootCnv/SysProcStat.h"

// C/C++ include files
#include <set>

/*
 * Gaudi namespace declaration
 */
namespace Gaudi {

  /** @class RootPerfMonSvc RootPerfMonSvc.h src/RootPerfMonSvc.h
   *
   * Description:
   *
   * RootPerfMonSvc class implementation definition.
   *
   *  @author  Markus Frank, Ivan Valencik
   *  @version 1.0
   *  @date    20/12/2009
   */
  class GAUDI_API RootPerfMonSvc : public extends<Service, IIncidentListener> {
  protected:
    Gaudi::Property<std::string> m_ioPerfStats{this, "IOPerfStats", "",
                                               "Enable TTree IOperfStats if not empty; otherwise perf stat file name"};
    // Passed parameters
    Gaudi::Property<std::string> m_setStreams{this, "Streams", "", ""};
    Gaudi::Property<std::string> m_basketSize{this, "BasketSize", "", ""};
    Gaudi::Property<std::string> m_bufferSize{this, "BufferSize", "", ""};
    Gaudi::Property<std::string> m_splitLevel{this, "SplitLevel", "", ""};

    /// Reference to incident service
    SmartIF<IIncidentSvc> m_incidentSvc;
    /// Message streamer
    std::unique_ptr<MsgStream> m_log;
    // Reference to a tree with statistics
    TTree* m_perfTree;
    // Reference to a file where statistics are persisted
    std::unique_ptr<TFile> m_perfFile;
    // Reference to all connected output files
    std::set<std::string> m_outputs;
    // The newest observed values
    Int_t   m_eventType;
    ULong_t m_utime, m_stime, m_vsize;
    Long_t  m_eventNumber, m_rss, m_time;

    // Types of records
    enum EventType { EVENT = 1, FSR = 2 };

    // Adding newest observed values to the tree with statistics
    virtual void record( EventType eventType );

    /// Helper: Use message streamer
    MsgStream& log() const { return *m_log; }

  public:
    /// Standard constructor
    using extends::extends;

    /** Standard way to print errors. after the printout an exception is thrown.
     * @param      msg      [IN]     Message string to be printed.
     *
     * @return     Status code returning failure.
     */
    StatusCode error( const std::string& msg );

    /// Service overload: initialize the service
    StatusCode initialize() override;

    /// Service overload: Finalize the service
    StatusCode finalize() override;

    /// IIncidentListener override: Inform that a new incident has occurred
    void handle( const Incident& incident ) override;

    // Service overload: Stop the service
    StatusCode stop() override;
  };
} // namespace Gaudi

#endif // GAUDIROOTCNV_GAUDIROOTPERFMONSVC_H
