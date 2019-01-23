//      ====================================================================
//  EvtCollection.Write.h
//      --------------------------------------------------------------------
//
//      Package   : GaudiExamples/EvtCollection
//
//      Author    : Markus Frank
//
//      ====================================================================
#ifndef EVTCOLLECTION_WRITE_H
#define EVTCOLLECTION_WRITE_H

// Framework include files
#include "GaudiKernel/Algorithm.h" // Required for inheritance
#include "GaudiKernel/NTuple.h"

// Forward declarations
class INtupleSvc;

namespace Gaudi
{
  namespace Examples
  {
    class MyTrack;
  }
}
/** User example objects: EvtCollectionWrite

    Description:
    A small class creating an event collection.

    Author:  M.Frank
    Version: 1.0
*/
class EvtCollectionWrite : public Algorithm
{

  Gaudi::Property<int> m_nMCcut{this, "NumMcTracks", 50, ""};

  NTuple::Item<int>                       m_ntrkColl;
  NTuple::Item<float>                     m_eneColl;
  NTuple::Item<Gaudi::Examples::MyTrack*> m_trackItem;
  NTuple::Array<float>                    m_trkMom;
  NTuple::Array<float>                    m_trkMomFixed;
  NTuple::Item<IOpaqueAddress*>           m_evtAddrColl;
  NTuple::Item<IOpaqueAddress*>           m_evtAddrCollEx;
  INTupleSvc*                             m_evtTupleSvc = nullptr;

public:
  /// Constructor: A constructor of this form must be provided.
  using Algorithm::Algorithm;
  /// Initialize
  StatusCode initialize() override;
  /// Finalize
  StatusCode finalize() override { return StatusCode::SUCCESS; }
  /// Event callback
  StatusCode execute() override;
};

#endif // EVTCOLLECTION_WRITE_H
