#ifndef __PARTPROPSVC_H__
#define __PARTPROPSVC_H__

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "HepPDT/ParticleDataTable.hh"
#include "HepPDT/TableBuilder.hh"

#include <vector>
#include <string>
#include <utility>
#include <iostream>
#include <memory>

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

class PartPropSvc: public extends1<Service, IPartPropSvc> {
public:

  PartPropSvc( const std::string& name, ISvcLocator* svc );

  StatusCode initialize() override;
  StatusCode finalize() override;

  // The table
  HepPDT::ParticleDataTable *PDT() override;

  void setUnknownParticleHandler( HepPDT::ProcessUnknownID*,
                                  const std::string&) override;

  // Destructor.
  ~PartPropSvc() override = default;

private:

  using inputFunPtr = bool(*)(std::istream&,HepPDT::TableBuilder&);

  StatusCode createTable();
  std::vector<std::pair<std::string,inputFunPtr>> m_inputs;

  StringProperty m_pdtFiles;
  HepPDT::ProcessUnknownID* m_upid = nullptr;
  std::string m_upid_name;

  std::unique_ptr<HepPDT::ParticleDataTable> m_pdt;

  inputFunPtr parseTableType(const std::string&);

  bool m_upid_local = false;

};

#endif
