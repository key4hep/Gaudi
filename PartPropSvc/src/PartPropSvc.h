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

  virtual StatusCode initialize();
  virtual StatusCode reinitialize();
  virtual StatusCode finalize();

  // The table
  HepPDT::ParticleDataTable *PDT();

  void setUnknownParticleHandler( HepPDT::ProcessUnknownID*,
				  const std::string&);

  PartPropSvc( const std::string& name, ISvcLocator* svc );

  // Destructor.
  virtual ~PartPropSvc();

private:


  StatusCode createTable();
  std::vector< std::pair<std::string,
			 bool(*) (std::istream&,HepPDT::TableBuilder&)> > m_inputs;

  StringProperty m_pdtFiles;
  HepPDT::ProcessUnknownID* m_upid;
  std::string m_upid_name;

  HepPDT::ParticleDataTable *m_pdt;

  bool (*parseTableType(std::string&))(std::istream&, HepPDT::TableBuilder&);

  mutable MsgStream m_log;

  bool m_upid_local;


};

#endif
