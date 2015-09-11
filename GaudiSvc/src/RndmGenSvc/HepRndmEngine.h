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
    // Seed table
    mutable std::vector<long>   m_seeds;
    // Other parameters
    int  m_row, m_col, m_lux;
    bool m_useTable, m_setSingleton;
  public: 
    /// Standard Constructor
    Engine(const std::string& name, ISvcLocator* loc);
    /// Standard Destructor
    ~Engine() override = default;
    /// Initialize the Engine
    StatusCode initialize() override;
    /// Finalize the Engine
    StatusCode finalize() override;
    /// Set seeds
    StatusCode setSeeds(const std::vector<long>& seed);
    /// Retrieve seeds
    StatusCode seeds(std::vector<long>& seed)   const;
  private:
    /// Create new HepEngine....
    std::unique_ptr<CLHEP::HepRandomEngine> createEngine() override;
  };
}

#endif // HEPRNDM_HEPRNDMENGINE_H
