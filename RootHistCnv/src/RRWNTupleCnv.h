#ifndef ROOTHISTCNV_RRWNTUPLECNV_H
#define ROOTHISTCNV_RRWNTUPLECNV_H 1

// Include files
#include "RNTupleCnv.h"

// Forward declarations
class TTree;

namespace RootHistCnv {
  /** @class RootHistCnv::RRWNTupleCnv RRWNTupleCnv.h

      Row wise NTuple converter class definition
      @author Charles Leggett
  */

  class RRWNTupleCnv : public RNTupleCnv {
  public:
    /// Inquire class type
    static const CLID& classID() { return CLID_RowWiseTuple; }
    /// Standard constructor
    RRWNTupleCnv( ISvcLocator* svc ) : RNTupleCnv( svc, classID() ) {}

  protected:
    /// Create the transient representation of an object.
    StatusCode load( TTree* tree, INTuple*& refpObject ) override;

    /// Book the N tuple
    StatusCode book( const std::string& desc, INTuple* pObject, TTree*& tree ) override;
    /// Write N tuple data
    StatusCode writeData( TTree* rtree, INTuple* pObject ) override;
    /// Read N tuple data
    StatusCode readData( TTree* rtree, INTuple* pObject, long ievt ) override;
  };
} // namespace RootHistCnv

#endif // RootHistCnv_RRWNTUPLECNV_H
