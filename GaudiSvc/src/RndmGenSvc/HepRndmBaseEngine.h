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
#include "CLHEP/Random/Random.h"
#include "RndmEngine.h"

// Forward declarations
namespace CLHEP
{
  class HepRandomEngine;
}

namespace HepRndm
{

  class BaseEngine : public RndmEngine
  {
  protected:
    std::unique_ptr<CLHEP::HepRandomEngine> m_hepEngine;

  public:
    BaseEngine( const std::string& name, ISvcLocator* loc ) : RndmEngine( name, loc ) {}
    ~BaseEngine() override = default;
    CLHEP::HepRandomEngine*       hepEngine() { return m_hepEngine.get(); }
    const CLHEP::HepRandomEngine* hepEngine() const { return m_hepEngine.get(); }
    // Retrieve single random number
    double rndm() const override { return m_hepEngine->flat(); }

    StatusCode finalize() override
    {
      if ( m_hepEngine ) CLHEP::HepRandom::setTheEngine( nullptr );
      m_hepEngine.reset();
      return RndmEngine::finalize();
    }

  protected:
    void                                            initEngine() { m_hepEngine = createEngine(); }
    virtual std::unique_ptr<CLHEP::HepRandomEngine> createEngine()             = 0;
  };
}
#endif // HEPRNDM_HEPRNDMBASEENGINE_H
