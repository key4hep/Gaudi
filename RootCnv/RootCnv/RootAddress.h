//====================================================================
//	RootAddress.h
//--------------------------------------------------------------------
//
//	Author     : M.Frank
//====================================================================
#ifndef GAUDIROOTCNV_ROOTADDRESS_H
#define GAUDIROOTCNV_ROOTADDRESS_H

// Framework include files
#include "GaudiKernel/GenericAddress.h"
#include "TTreeFormula.h"

// Forward declaration
class TTree;
class TTreeFormula;

/*
 * Gaudi namespace declaration
 */
namespace Gaudi {

  // Forward declarations
  class RootDataConnection;

  /** @class RootAddress RootAddress.h GaudiRoot/RootAddress.h
   *
   * Description:
   *
   * Definition of a transient link which is capable of locating
   * an object in the persistent storage.
   *
   * @author  M.Frank
   * @version 1.0
   */
  class GAUDI_API RootAddress : virtual public GenericAddress {
  public:
    /// Pointer to ROOT select statement (filled for N-tuples only)
    TTreeFormula* select = nullptr;
    /// Pointer to ROOT TTree (filled for N-tuples only)
    TTree* section = nullptr;

  public:
    /// Full constructor
    RootAddress( long svc, const CLID& clid, const std::string& p1 = "", const std::string& p2 = "",
                 unsigned long ip1 = 0, unsigned long ip2 = 0 )
        : GenericAddress( svc, clid, p1, p2, ip1, ip2 ) {}
    /// Standard Destructor
    virtual ~RootAddress() { delete select; }
  };
} // namespace Gaudi

#endif // GAUDIROOTCNV_ROOTADDRESS_H
