// $Id: RRWNTupleCnv.h,v 1.6 2006/11/30 15:04:07 mato Exp $
#ifndef ROOTHISTCNV_RRWNTUPLECNV_H
#define ROOTHISTCNV_RRWNTUPLECNV_H 1

// Include files
#include "RNTupleCnv.h"

// Forward declarations
class TTree;
template <class TYPE> class CnvFactory;

namespace RootHistCnv {
/** @class RootHistCnv::RRWNTupleCnv RRWNTupleCnv.h

    Row wise NTuple converter class definition
    @author Charles Leggett
*/

  class RRWNTupleCnv : public RNTupleCnv   {

    /// Creator needs access to constructor/destructor
    friend class CnvFactory<RRWNTupleCnv>;

  public:
    /// Inquire class type
    static const CLID& classID()    {
      return CLID_RowWiseTuple;
    }

    /// Standard constructor
    RRWNTupleCnv( ISvcLocator* svc )  : RNTupleCnv(svc, classID())    {
    }
    /// Standard destructor
    virtual ~RRWNTupleCnv()   {
    }
  protected:
    /// Create the transient representation of an object.
    virtual StatusCode load(TTree* tree, INTuple*& refpObject);

    /// Book the N tuple
    virtual StatusCode book(const std::string& desc, INTuple* pObject, TTree*& tree);
    /// Write N tuple data
    virtual StatusCode writeData(TTree* rtree, INTuple* pObject);
    /// Read N tuple data
    virtual StatusCode readData(TTree* rtree, INTuple* pObject, long ievt);
  };
}    // namespace RootHistCnv

#endif    // RootHistCnv_RRWNTUPLECNV_H
