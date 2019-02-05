#ifndef ROOTHISTCNV_RCWNTCNV_H
#define ROOTHISTCNV_RCWNTCNV_H 1

// Include files
#include "GaudiKernel/Converter.h"
#include "RNTupleCnv.h"
#include <string>

#include "TFile.h"
#include "TTree.h"

namespace RootHistCnv {

  /** @class RootHistCnv::RCWNTupleCnv RCWNTupleCnv.h
   *
   * Converter of Column-wise NTuple into ROOT format
   * @author Charles Leggett
   */

  class RCWNTupleCnv : public RNTupleCnv {
  public:
    /// Inquire class type
    static const CLID& classID() { return CLID_ColumnWiseTuple; }
    /// Standard constructor
    RCWNTupleCnv( ISvcLocator* svc ) : RNTupleCnv( svc, classID() ) {}

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

#endif // ROOTHISTCNV_RCWNTCNV_H
