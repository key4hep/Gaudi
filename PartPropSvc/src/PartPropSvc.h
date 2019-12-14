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
#ifndef __PARTPROPSVC_H__
#define __PARTPROPSVC_H__

#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/Service.h"
#include "HepPDT/ParticleDataTable.hh"
#include "HepPDT/TableBuilder.hh"

#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

//------------------------------------------------------------------
//
// ClassName:   PartPropSvc
//
// Description: This service provides access to particle properties.
//              Uses HepPDT as the underlying layer
//
// Author:      Charles Leggett
//
// Date:        3-8-2001
//
//------------------------------------------------------------------

class PartPropSvc : public extends<Service, IPartPropSvc> {
public:
  using extends::extends;

  StatusCode initialize() override;
  StatusCode finalize() override;

  // The table
  HepPDT::ParticleDataTable* PDT() override;

  void setUnknownParticleHandler( HepPDT::ProcessUnknownID*, const std::string& ) override;

private:
  using inputFunPtr = bool ( * )( std::istream&, HepPDT::TableBuilder& );

  StatusCode                                       createTable();
  std::vector<std::pair<std::string, inputFunPtr>> m_inputs;

  Gaudi::Property<std::string> m_pdtFiles{this, "InputFile", "PDGTABLE.MeV"};

  HepPDT::ProcessUnknownID* m_upid = nullptr;
  std::string               m_upid_name;

  std::optional<HepPDT::ParticleDataTable> m_pdt;

  inputFunPtr parseTableType( const std::string& );

  bool m_upid_local = false;
};

#endif
