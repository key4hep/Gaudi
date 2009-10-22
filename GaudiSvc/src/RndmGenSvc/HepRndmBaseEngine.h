//====================================================================
//	CLHEP Basic Random Engine definition file
//--------------------------------------------------------------------
//
//	Package    : HepRndm ( The LHCb Offline System)
//	Author     : M.Frank
//  History    :
// +---------+----------------------------------------------+---------
// |    Date |                 Comment                      | Who     
// +---------+----------------------------------------------+---------
// | 29/10/99| Initial version                              | MF
// +---------+----------------------------------------------+---------
//
//====================================================================
#ifndef HEPRNDM_HEPRNDMBASEENGINE_H
#define HEPRNDM_HEPRNDMBASEENGINE_H 1

// Framework include files
#include "RndmEngine.h"

// Forward declarations
namespace CLHEP {
  class HepRandomEngine;
}

namespace HepRndm  {

  class BaseEngine : public RndmEngine {
  protected:
    CLHEP::HepRandomEngine*  m_hepEngine;
  public:
    BaseEngine(const std::string& name, ISvcLocator* loc)
    : RndmEngine( name, loc ), m_hepEngine(0)  {
    }
    virtual ~BaseEngine() {
    }
    CLHEP::HepRandomEngine*  hepEngine()   {
      return m_hepEngine;
    }
  };
}
#endif // HEPRNDM_HEPRNDMBASEENGINE_H
