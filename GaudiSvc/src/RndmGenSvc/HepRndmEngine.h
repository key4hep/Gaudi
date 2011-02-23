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
    virtual ~Engine();
    /// Initialize the Engine
    virtual StatusCode initialize();
    /// Finalize the Engine
    virtual StatusCode finalize();
    /// Create/Initialize new HepEngine....
    virtual StatusCode initializeEngine();
    /// Single shot
    //// This was declared "virtual double rndm()" which causes
    //// compiler warnings since the signature if different than
    //// that for RndmEngine::rndm(). I've assumed that the difference
    //// is an error. If it is in fact a deliberate addition of a
    //// new function, remove the "const" below and uncomment the
    //// subsequent lines.
    virtual double rndm() const;
////    /// Single shot returning single random number
////    /// Repeating this here avoids compiler warnings. See ARM Page 210.
////     virtual double rndm() const { return BaseEngine::rndm( ); }
    /// Set seeds
    StatusCode setSeeds(const std::vector<long>& seed);
    /// Retrieve seeds
    StatusCode seeds(std::vector<long>& seed)   const;
  };
}

#endif // HEPRNDM_HEPRNDMENGINE_H
