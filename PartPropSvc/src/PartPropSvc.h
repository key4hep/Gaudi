#ifndef __PARTPROPSVC_H__
#define __PARTPROPSVC_H__

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "HepPDT/ParticleDataTable.hh"

#include <vector>
#include <string>

// Forward declarations
template <class TYPE> class SvcFactory;

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
  virtual StatusCode finalize();

  // The table
  HepPDT::ParticleDataTable *PDT() { return m_pdt; };

protected:

  PartPropSvc( const std::string& name, ISvcLocator* svc );

  // Destructor.
  virtual ~PartPropSvc();

private:

  // Allow SvcFactory to instantiate the service.
  friend class SvcFactory<PartPropSvc>;

  std::string m_inputType;
  std::vector<std::string> m_pdtFiles;

  HepPDT::ParticleDataTable *m_pdt;

};

#endif
