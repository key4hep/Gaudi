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
#include <vector>
#include "HepRndmBaseEngine.h"

namespace HepRndm  {

  template <class TYPE>
  class Engine : public BaseEngine   {
  protected:
    mutable PropertyWithValue<std::vector<long>> m_seeds{this, "Seeds", {}, "seed table"};

    IntegerProperty       m_col {this, "Column",  0};
    IntegerProperty          m_row {this, "Row",  1};
    IntegerProperty       m_lux {this, "Luxury",  3};
    BooleanProperty     m_useTable     {this, "UseTable",  false};
    BooleanProperty m_setSingleton {this, "SetSingleton",  false};

  public:
    /// Standard Constructor
    using BaseEngine::BaseEngine;
    /// Standard Destructor
    ~Engine() override = default;
    /// Initialize the Engine
    StatusCode initialize() override;
    /// Finalize the Engine
    StatusCode finalize() override;
    /// Set seeds
    StatusCode setSeeds(const std::vector<long>& seed) override;
    /// Retrieve seeds
    StatusCode seeds(std::vector<long>& seed)   const override;
  private:
    /// Create new HepEngine....
    std::unique_ptr<CLHEP::HepRandomEngine> createEngine() override;
  };
}

#endif // HEPRNDM_HEPRNDMENGINE_H
