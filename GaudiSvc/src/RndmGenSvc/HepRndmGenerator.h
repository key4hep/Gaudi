//====================================================================
//	CLHEP Random Generator definition file
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
#ifndef HEPRNDM_HEPRNDMGENERATOR_H
#define HEPRNDM_HEPRNDMGENERATOR_H 1

// Framework include files
#include "RndmGen.h"
#include "RndmGenSvc.h"

#include "HepRndmBaseEngine.h"

namespace HepRndm  {

  template <class TYPE>
  class Generator : public RndmGen   {
  public:
    typedef TYPE Parameters;
  protected:
    CLHEP::HepRandomEngine* m_hepEngine;
    TYPE*            m_specs;
  public: 
    Generator(IInterface* engine);
    virtual ~Generator();
    /// Initialize the generator
    virtual StatusCode initialize(const IRndmGen::Param& par);
    /// Single shot
    virtual double shoot()  const;
  };

  template <class TYPE> Generator<TYPE>::Generator(IInterface* engine)
  : RndmGen (engine), m_hepEngine(0), m_specs(0)    {
  }

  template <class TYPE> Generator<TYPE>::~Generator()    {  
  }

  /// Initialize the generator
  template <class TYPE> StatusCode Generator<TYPE>::initialize(const IRndmGen::Param& par)   {
    StatusCode status = RndmGen::initialize(par);
    if ( status.isSuccess() )   {
      try   {
        m_specs = dynamic_cast<TYPE*>(m_params);
        if ( 0 != m_specs )  {
          BaseEngine* engine = dynamic_cast<BaseEngine*>(m_engine);
          if ( 0 != engine )    {
            m_hepEngine = engine->hepEngine();
            if ( 0 != m_hepEngine )   {
              return StatusCode::SUCCESS;
            }
          }
        }
      }
      catch (...)   {
      }
    }
    return StatusCode::FAILURE;
  }
}

#endif // HEPRNDM_HEPRNDMGENERATOR_H
