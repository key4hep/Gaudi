/***********************************************************************************\
* (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
#pragma once

// Include files
#include "RDirectoryCnv.h"
#include <GaudiKernel/IIncidentSvc.h>
#include <GaudiKernel/NTuple.h>

class TFile;

namespace RootHistCnv {

  /** @class RootHistCnv::RFileCnv RFileCnv.h

      NTuple converter class definition

      - Major cleanup and debugging for Gaudi v9
      - Removal of all static storage

      @author Charles Leggett
      @author Markus Frank
  */
  class RFileCnv : public RDirectoryCnv {

  public:
    /// Initialise
    StatusCode initialize() override;

  public:
    /// Create the transient representation of an object.
    StatusCode createObj( IOpaqueAddress* pAddress, DataObject*& refpObject ) override;
    /// Convert the transient object to the requested representation.
    StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress ) override;
    /// Convert the transient object to the requested representation.
    StatusCode updateRep( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  public:
    /// Inquire class type
    static const CLID& classID() { return CLID_NTupleFile; }

    /// Standard constructor
    RFileCnv( ISvcLocator* svc );

  protected:
    TFile*                rfile{ nullptr }; ///< Pointer to ROOT file
    std::string           m_compLevel;      ///< Compression setting, property RFileCnv.GlobalCompression
    SmartIF<IIncidentSvc> m_incSvc;
  };
} // namespace RootHistCnv
