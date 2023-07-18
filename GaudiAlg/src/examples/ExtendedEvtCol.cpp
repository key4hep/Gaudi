/***********************************************************************************\
* (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations *
*                                                                                   *
* This software is distributed under the terms of the Apache version 2 licence,     *
* copied verbatim in the file "LICENSE".                                            *
*                                                                                   *
* In applying this licence, CERN does not waive the privileges and immunities       *
* granted to it by virtue of its status as an Intergovernmental Organization        *
* or submit itself to any jurisdiction.                                             *
\***********************************************************************************/
// ============================================================================
// Include files
// =============================================================================
// STD& STL
// =============================================================================
#include <algorithm>
#include <cmath>
#include <functional>
#include <math.h>
#include <numeric>
// =============================================================================
// GaudiKernel
// =============================================================================
#include "GaudiKernel/IRegistry.h"
// =============================================================================
// GaudiAlg
// =============================================================================
#include "GaudiAlg/GaudiTupleAlg.h"
// =============================================================================
// Local
// =============================================================================
#include "GaudiExamples/Event.h"
#include "GaudiExamples/MyTrack.h"
// =============================================================================
namespace Gaudi {
  namespace Examples {
    /** @class ExtendedEvtCol
     *  Simple class to illustrate the usage of extended event
     *  tag collections: storing "Arbitrary" objects in N-tuples.
     *  @see Tuples::TupleObj::put
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-04-08
     */
    class ExtendedEvtCol : public GaudiTupleAlg {
    public:
      /// the only one essential method: execute the algorithm
      StatusCode execute() override;

    public:
      /** standard constructor
       *  @param name algorithm instance name
       *  @param pSvc pointer to Servcoe Locator
       */
      ExtendedEvtCol( const std::string& name, ISvcLocator* pSvc ) : GaudiTupleAlg( name, pSvc ) {
        /// redefine the default values for various properties
        setProperty( "NTupleProduce", false ).ignore();
        setProperty( "NTuplePrint", false ).ignore();
        setProperty( "HistogramProduce", false ).ignore();
        setProperty( "HistogramPrint", false ).ignore();
        setProperty( "PropertiesPrint", true ).ignore();
        setProperty( "TypePrint", false ).ignore();
        setProperty( "EvtColsProduce", true ).ignore();
        setProperty( "EvtColsPrint", true ).ignore();
      }

    private:
      Gaudi::Property<std::string> m_tracks{ this, "Tracks", "MyTracks" };
    };
  } // end of namespace Examples
} // end of namespace Gaudi
// ============================================================================
/// MANDATORY factory for instantiation
// ============================================================================
using Gaudi::Examples::ExtendedEvtCol;
DECLARE_COMPONENT( ExtendedEvtCol )
// ============================================================================
/// local anonymous namespace to hide some technicalities
// ============================================================================
namespace {
  /// local function for evaluation of Tracks's momentum
  inline double trkMomentum( const Gaudi::Examples::MyTrack* track ) {
    if ( !track ) { return 0; }
    return ::sqrt( track->px() * track->px() + track->py() * track->py() + track->pz() * track->pz() );
  }
} // namespace
// ============================================================================
/// the only one essential method: execute the algorithm
// ============================================================================
StatusCode Gaudi::Examples::ExtendedEvtCol::execute() {
  // get the event
  auto event = get<DataObject>( "/Event" );
  // get the tracks
  auto tracks = get<MyTrackVector>( m_tracks );

  // book/retreieve the Event Tag Collection:
  auto tuple = evtCol( TupleID( "MyCOL1" ), "Trivial Event Tag Collection" );

  /// put the event address into the event tag collection:
  tuple->column( "Address", event->registry()->address() ).ignore();

  /// put the information about the tracks
  tuple
      ->farray( "TrkMom", trkMomentum, "px", &MyTrack::px, "py", &MyTrack::py, "pz", &MyTrack::pz, tracks->begin(),
                tracks->end(), "Ntrack", 5000 )
      .ignore();

  // evaluate the total energy of all tracks:
  double energy =
      std::accumulate( tracks->begin(), tracks->end(), 0.0,
                       [&]( double e, const Gaudi::Examples::MyTrack* track ) { return e + trkMomentum( track ); } );

  tuple->column( "Energy", energy ).ignore();

  // put a track into Event Tag Collection
  tuple->put( "Track", !tracks->empty() ? *( tracks->begin() ) : nullptr ).ignore();

  return tuple->write();
}
// ============================================================================
/// The END
// ============================================================================
