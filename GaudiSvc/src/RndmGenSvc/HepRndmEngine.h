//====================================================================
//	CLHEP Random Engine definition file
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
#ifndef HEPRNDM_HEPRNDMENGINE_H
#define HEPRNDM_HEPRNDMENGINE_H 1

// Framework include files
#include "HepRndmBaseEngine.h"
#include <vector>

namespace HepRndm
{

  template <class TYPE>
  class Engine : public BaseEngine
  {
  protected:
    mutable Gaudi::Property<std::vector<long>> m_seeds{this, "Seeds", {}, "seed table"};

    Gaudi::Property<int>  m_col{this, "Column", 0};
    Gaudi::Property<int>  m_row{this, "Row", 1};
    Gaudi::Property<int>  m_lux{this, "Luxury", 3};
    Gaudi::Property<bool> m_useTable{this, "UseTable", false};
    Gaudi::Property<bool> m_setSingleton{this, "SetSingleton", false};

  public:
    /// Standard Constructor
    using BaseEngine::BaseEngine;
    /// Initialize the Engine
    StatusCode initialize() override;
    /// Finalize the Engine
    StatusCode finalize() override;
    /// Set seeds
    StatusCode setSeeds( const std::vector<long>& seed ) override;
    /// Retrieve seeds
    StatusCode seeds( std::vector<long>& seed ) const override;

  private:
    /// Create new HepEngine....
    std::unique_ptr<CLHEP::HepRandomEngine> createEngine() override;
  };
}

#endif // HEPRNDM_HEPRNDMENGINE_H
