#ifndef ROOTHISTCNV_RNTUPLECNV_H
#define ROOTHISTCNV_RNTUPLECNV_H 1

// Include files
#include "RConverter.h"

// Forward declarations
class ISvcLocator;
class NTUPLEINFO;
class INTupleItem;
class INTupleSvc;
class INTuple;
class TTree;

namespace RootHistCnv
{

  /** @class RootHistCnv::RNTupleCnv RNTupleCnv.h
      NTuple converter class definition

      Package    : RootHistCnv
      Author     : Charles Leggett

  */
  class RNTupleCnv : public RConverter
  {
  public:
    /// Initialize the converter
    StatusCode initialize() override;
    /// Finalize the converter
    StatusCode finalize() override;

    /// Create the transient representation of an object.
    StatusCode createObj( IOpaqueAddress* pAddr, DataObject*& refpObj ) override;
    /// Update the transient object from the other representation.
    StatusCode updateObj( IOpaqueAddress* pAddr, DataObject* refpObj ) override;
    /// Convert the transient object to the requested representation.
    StatusCode createRep( DataObject* pObj, IOpaqueAddress*& refpAddr ) override;
    /// Update the converted representation of a transient object.
    StatusCode updateRep( IOpaqueAddress* pAddr, DataObject* pObj ) override;

  protected:
    /// Standard constructor
    RNTupleCnv( ISvcLocator* svc, const CLID& clid );

    /// Create the transient representation of an object.
    virtual StatusCode load( TTree* tree, INTuple*& refpObj ) = 0;
    /// Book a new N tuple
    virtual StatusCode book( const std::string& desc, INTuple* pObj, TTree*& tree ) = 0;
    /// Write N tuple data
    virtual StatusCode writeData( TTree* rtree, INTuple* pObj ) = 0;
    /// Read N tuple data
    virtual StatusCode readData( TTree* rtree, INTuple* pObj, long ievt ) = 0;

  protected:
    /// Reference to N tuple service
    SmartIF<INTupleSvc> m_ntupleSvc;

    virtual std::string rootVarType( int );
  };

  /// Add an item of a given type to the N tuple
  template <class TYP>
  INTupleItem* createNTupleItem( NTUPLEINFO& tags, long i, INTuple* tuple, TYP minimum, TYP maximum, long& size );

  template <class TYP>
  INTupleItem* createNTupleItem( std::string itemName, std::string blockName, std::string indexName, int indexRange,
                                 int arraySize, TYP minimum, TYP maximum, INTuple* tuple );

  bool parseName( const std::string& full, std::string& blk, std::string& var );

} // namespace RootHistCnv

#endif // RootHistCnv_RNTupleCnv_H
