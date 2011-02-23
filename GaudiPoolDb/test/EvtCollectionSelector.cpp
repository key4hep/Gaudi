// $Header: /tmp/svngaudi/tmp.jEpFh25751/Gaudi/GaudiPoolDb/test/EvtCollectionSelector.cpp,v 1.3 2006/11/30 14:50:45 mato Exp $
//	====================================================================
//	EvtCollectionSelector.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#define EVTCOLLECTIONSELECTOR_CPP 1

// Include files
#include "GaudiKernel/ObjectFactory.h"
#include "GaudiKernel/Selector.h"
#include "GaudiKernel/NTuple.h"
/**

  Definition of a small class for further pre-selections
  when reading an event collection.

    Author:  M.Frank
    Version: 1.0
*/
class EvtCollectionSelector : public NTuple::Selector  {
  NTuple::Item<long>   m_ntrack;
  NTuple::Array<float> m_trkMom;
  long cut;
public:
  EvtCollectionSelector(IInterface* svc) : NTuple::Selector(svc)   {
    cut = 10;
  }
  virtual ~EvtCollectionSelector()   {
  }
  /// Initialization
  virtual StatusCode initialize(NTuple::Tuple* nt)    {
    StatusCode status = nt->item ("TrkMom", m_trkMom);
    if ( status.isSuccess() ) {
      return nt->item("Ntrack", m_ntrack);
    }
    return status;
  }
  /// Specialized callback for NTuples
  virtual bool operator()(NTuple::Tuple* /* nt */)    {
    //std::cout << "->Ntrack: " << m_ntrack << std::endl << " ->Track:";
    for (int i = 0; i < 5; ++i )  {
      //std::cout << "[" << i << "]=" << m_trkMom[i] << " ";
    }
    //std::cout << std::endl;
    return m_ntrack>cut;
  }
};
DECLARE_OBJECT_FACTORY(EvtCollectionSelector);
