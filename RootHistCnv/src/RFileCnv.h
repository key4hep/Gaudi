// $Id: RFileCnv.h,v 1.7 2006/11/30 20:51:35 mato Exp $
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
  class RFileCnv : public RDirectoryCnv
  {

  public:

    /// Initialise
    virtual StatusCode initialize();

  public:

    /// Create the transient representation of an object.
    virtual StatusCode createObj( IOpaqueAddress* pAddress, DataObject*& refpObject );
    /// Convert the transient object to the requested representation.
    virtual StatusCode createRep( DataObject* pObject, IOpaqueAddress*& refpAddress );
    /// Convert the transient object to the requested representation.
    virtual StatusCode updateRep( IOpaqueAddress* pAddress, DataObject* pObject );

  public:

    /// Inquire class type
    static const CLID& classID()    {
      return CLID_NTupleFile;
    }

    /// Standard constructor
    RFileCnv( ISvcLocator* svc );

    /// Standard destructor
    virtual ~RFileCnv();

  protected:

    TFile *rfile;            ///< Pointer to ROOT file
    std::string m_compLevel; ///< Compression setting

  };
}    // namespace RootHistCnv
#endif    // RootHistCnv_RFileCNV_H
