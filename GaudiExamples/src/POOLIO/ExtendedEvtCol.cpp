// $Id: ExtendedEvtCol.cpp,v 1.3 2008/11/04 22:49:24 marcocle Exp $
// ============================================================================
// CVS tag $Name:  $, version $Revision: 1.3 $
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
#include "GaudiKernel/AlgFactory.h"
// =============================================================================
// GaudiAlg 
// =============================================================================
#include "GaudiAlg/GaudiTupleAlg.h"
// =============================================================================
// Local
// =============================================================================
#include "Event.h"
#include "MyTrack.h"
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
DECLARE_NAMESPACE_ALGORITHM_FACTORY(Gaudi::Examples,ExtendedEvtCol)
// ============================================================================
/// local anonymous namespace to hide some technicalities
// ============================================================================
namespace 
{
  /// local function for evaluation of Tracks's momentum 
  inline double trkMomentum ( const Gaudi::Examples::MyTrack* track )
  {
    if ( 0 == track ) { return 0 ; }
    return ::sqrt ( track -> px () * track -> px () +
                    track -> py () * track -> py () +
                    track -> pz () * track -> pz () );
  }
  /// local function for summation of Tracks's momenta
  inline double sumEnergy ( const double e , const Gaudi::Examples::MyTrack* track ) 
  { return trkMomentum ( track ) + e ; }
}
// ============================================================================
/// the only one essential method: execute the algorithm
// ============================================================================
StatusCode Gaudi::Examples::ExtendedEvtCol::execute () 
{
  // get the event 
  DataObject*     event = get<DataObject> ( "/Event" ) ;
  // get the tracks 
  MyTrackVector* tracks = get<MyTrackVector> ( m_tracks ) ;
  
  // book/retreieve the Event Tag Collection:
  Tuple tuple = evtCol ( TupleID("MyCOL1") , "Trivial Event Tag Collection" ) ;
  
  /// put the event address into the event tag collection:
  tuple -> column ( "Address" , event->registry()->address() ) ;
  
  /// put the information about the tracks
  tuple -> farray ( "TrkMom" , std::ptr_fun(trkMomentum)  , 
                    "px"     , std::mem_fun(&MyTrack::px) , 
                    "py"     , std::mem_fun(&MyTrack::py) , 
                    "pz"     , std::mem_fun(&MyTrack::pz) , 
                    tracks -> begin () , 
                    tracks -> end   () ,
                    "Ntrack" , 5000   ) ;
  
  // evaluate the total energy of all tracks:
  const double energy = std::accumulate 
    ( tracks -> begin () , tracks -> end   () , 0.0 , sumEnergy ) ;
  
  tuple -> column ("Energy" , energy ) ;
  
  // put a track into Event Tag Collection
  MyTrack* track = 0 ;
  if ( !tracks->empty() ) { track = *(tracks->begin()) ; }  
  tuple -> put ( "Track"  , track  ) ;
  
  tuple -> write() ;
  
  return StatusCode::SUCCESS ;
  
}
// ============================================================================
/// The END
// ============================================================================

