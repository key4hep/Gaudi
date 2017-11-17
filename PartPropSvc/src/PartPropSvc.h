#ifndef __PARTPROPSVC_H__
#define __PARTPROPSVC_H__

#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/Service.h"
#include "HepPDT/ParticleDataTable.hh"
#include "HepPDT/TableBuilder.hh"

#include <iostream>
#include <memory>
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

class PartPropSvc : public extends<Service, IPartPropSvc>
{
public:
  using extends::extends;

  StatusCode initialize() override;
  StatusCode finalize() override;

  // The table
  HepPDT::ParticleDataTable* PDT() override;

  void setUnknownParticleHandler( HepPDT::ProcessUnknownID*, const std::string& ) override;

private:
  using inputFunPtr = bool ( * )( std::istream&, HepPDT::TableBuilder& );

  StatusCode createTable();
  std::vector<std::pair<std::string, inputFunPtr>> m_inputs;

  Gaudi::Property<std::string> m_pdtFiles{this, "InputFile", "PDGTABLE.MeV"};

  HepPDT::ProcessUnknownID* m_upid = nullptr;
  std::string m_upid_name;

  std::unique_ptr<HepPDT::ParticleDataTable> m_pdt;

  inputFunPtr parseTableType( const std::string& );

  bool m_upid_local = false;
};

#endif
