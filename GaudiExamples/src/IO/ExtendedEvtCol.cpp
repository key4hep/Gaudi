// ============================================================================
// Include files
// =============================================================================
// STD& STL
// =============================================================================
#include <cmath>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <functional>
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
namespace Gaudi
{
  namespace Examples
  {
    /** @class ExtendedEvtCol
     *  Simple class to illustrate the usage of extended event
     *  tag collections: storing "Arbitrary" objects in N-tuples.
     *  @see Tuples::TupleObj::put
     *  @author Vanya BELYAEV ibelyaev@physics.syr.edu
     *  @date 2007-04-08
     */
    class ExtendedEvtCol : public GaudiTupleAlg
    {
    public:
      /// the only one essential method: execute the algorithm
      virtual StatusCode execute () ;
    public:
      /** standard constructor
       *  @param name algorithm instance name
       *  @param pSvc pointer to Servcoe Locator
       */
      ExtendedEvtCol
      ( const std::string& name ,
        ISvcLocator*       pSvc )
        : GaudiTupleAlg ( name , pSvc )
        , m_tracks ( "MyTracks" )
      {
        declareProperty ( "Tracks" , m_tracks ) ;
        /// redefine the default values for various properties
        setProperty ( "NTupleProduce"    , "false" ).ignore() ;
        setProperty ( "NTuplePrint"      , "false" ).ignore() ;
        setProperty ( "HistogramProduce" , "false" ).ignore() ;
        setProperty ( "HistogramPrint"   , "false" ).ignore() ;
        setProperty ( "PropertiesPrint"  , "true"  ).ignore() ;
        setProperty ( "StatPrint"        , "true"  ).ignore() ;
        setProperty ( "TypePrint"        , "false" ).ignore() ;
        setProperty ( "EvtColsProduce"   , "true"  ).ignore() ;
        setProperty ( "EvtColsPrint"     , "true"  ).ignore() ;
      }
      /// virtual destructor
      virtual ~ExtendedEvtCol() {}
    private:
      // default constructor is desabled
      ExtendedEvtCol() ;                                    ///< no default constructor
      // copy constructor is desabled
      ExtendedEvtCol           ( const  ExtendedEvtCol& ) ; ///< no copy constructor
      // assignement operator is desabled
      ExtendedEvtCol& operator=( const  ExtendedEvtCol& ) ; ///< no assignement
    private:
      std::string  m_tracks ;
    } ;
  } // end of namespace Examples
} // end of namespace Gaudi
// ============================================================================
/// MANDATORY factory for instantiation
// ============================================================================
using Gaudi::Examples::ExtendedEvtCol;
DECLARE_COMPONENT(ExtendedEvtCol)
// ============================================================================
/// local anonymous namespace to hide some technicalities
// ============================================================================
namespace
{
  /// local function for evaluation of Tracks's momentum
  inline double trkMomentum ( const Gaudi::Examples::MyTrack* track )
  {
    if ( !track ) { return 0 ; }
    return ::sqrt ( track -> px () * track -> px () +
                    track -> py () * track -> py () +
                    track -> pz () * track -> pz () );
  }
}
// ============================================================================
/// the only one essential method: execute the algorithm
// ============================================================================
StatusCode Gaudi::Examples::ExtendedEvtCol::execute ()
{
  // get the event
  auto event  = get<DataObject> ( "/Event" ) ;
  // get the tracks
  auto tracks = get<MyTrackVector> ( m_tracks ) ;

  // book/retreieve the Event Tag Collection:
  auto tuple = evtCol ( TupleID("MyCOL1") , "Trivial Event Tag Collection" ) ;

  /// put the event address into the event tag collection:
  tuple -> column ( "Address" , event->registry()->address() ) ;

  /// put the information about the tracks
  tuple -> farray ( "TrkMom" , trkMomentum  ,
                    "px"     , &MyTrack::px ,
                    "py"     , &MyTrack::py ,
                    "pz"     , &MyTrack::pz ,
                    tracks -> begin () ,
                    tracks -> end   () ,
                    "Ntrack" , 5000   ) ;

  // evaluate the total energy of all tracks:
  double energy = std::accumulate
    ( tracks -> begin () , tracks -> end   () , 0.0 ,
      [&](double e, const Gaudi::Examples::MyTrack* track)
      { return e + trkMomentum(track); } );

  tuple -> column ("Energy" , energy ) ;

  // put a track into Event Tag Collection
  tuple -> put ( "Track"  , !tracks->empty() ? *(tracks->begin()) :  nullptr ) ;

  tuple -> write() ;

  return StatusCode::SUCCESS ;

}
// ============================================================================
/// The END
// ============================================================================

