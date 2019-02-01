#ifndef ROOTHISTCNV_RFILECNV_H
#define ROOTHISTCNV_RFILECNV_H 1

// Include files
#include "GaudiKernel/NTuple.h"
#include "RDirectoryCnv.h"

class TFile;

namespace RootHistCnv {

  /** @class RootHistCnv::RFileCnv RFileCnv.h

      NTuple converter class definition

      - Major cleanup and debugging for Gaudi v9
      - Removal of all static storage

      @author Charles Leggett
      @author Markus Frank
  */
  class RFileCnv : public RDirectoryCnv {

  public:
    /// Initialise
    StatusCode initialize() override;

  public:
    /// Create the transient representation of an object.
    StatusCode createObj( IOpaqueAddress* pAddress, DataObject*& refpObject ) override;
    /// Convert the transient object to the requested representation.
    StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress ) override;
    /// Convert the transient object to the requested representation.
    StatusCode updateRep( IOpaqueAddress* pAddress, DataObject* pObject ) override;

  public:
    /// Inquire class type
    static const CLID& classID() { return CLID_NTupleFile; }

    /// Standard constructor
    RFileCnv( ISvcLocator* svc );

  protected:
    TFile*      rfile;       ///< Pointer to ROOT file
    std::string m_compLevel; ///< Compression setting, property RFileCnv.GlobalCompression
  };
} // namespace RootHistCnv
#endif // RootHistCnv_RFileCNV_H
