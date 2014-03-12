// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/Tuples.h"
#include "GaudiAlg/Tuple.h"
#include "GaudiAlg/TupleID.h"
#include "GaudiAlg/GaudiTupleAlg.h"
// ============================================================================
/** @file
 *  Example of usage GaudiTupleAlg base class fro Event Tag Colections
 *  @see GaudiTupleAlg
 *  @see Tuples::TupleObj
 *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
 *  @date 2005-08-17
 *  @date 2007-04-09
 */
// ============================================================================
namespace Gaudi
{
  namespace Examples
  {
    /** @class EvtColAlg
     *  Example of usage GaudiTupleAlg base class
     *  @attention the action of Tuples::TupleObg::write uis restored again!
     *  @see GaudiTupleAlg
     *  @author Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr
     *  @date 2005-08-17
     */
    // ========================================================================
    class EvtColAlg : public GaudiTupleAlg
    {
    public:
      StatusCode initialize ()
      {
        StatusCode sc = GaudiTupleAlg::initialize() ;
        if ( sc.isFailure() ) { return sc ; }
        // check for random numbers service
        Assert (randSvc() != 0, "Random Service is not available!");
        //
        return StatusCode::SUCCESS ;
      };
      /// the only one essential methos
      StatusCode execute    ()  ;
      /** standard construtor
       *  @param name algorrithm instance name
       *  @param pSvc pointer to service locator
       */
      EvtColAlg ( const std::string& name ,
                  ISvcLocator*       pSvc )
        : GaudiTupleAlg ( name , pSvc )
      {
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
      } ;
      /// virtual and protected dectrustor
      virtual ~EvtColAlg(){}
    private:
      // default constructor is private
      EvtColAlg() ;
      // copy constructor is disabled
      EvtColAlg( const EvtColAlg& ) ;
      // assignement operator is disabled
      EvtColAlg& operator=( const EvtColAlg& ) ;
    };
    // ========================================================================
  } // end of namespace Gaudi::Examples
} // end of namespace Gaudi
// ============================================================================
/// the only one real method - algorithm execution
// ============================================================================
StatusCode Gaudi::Examples::EvtColAlg::execute()
{
  static int s_nEvt = 0 ;
  static int s_nRun = 0 ;

  if ( 1 == ++s_nEvt % 50  ) { ++s_nRun ; }

  DataObject*     event    = get<DataObject>( "/Event" ) ;
  if ( 0 == event    ) { return StatusCode::FAILURE ; }
  IRegistry*      registry = event->registry() ;
  if ( 0 == registry ) { return Error("IRegistry* point to NULL!"); }
  IOpaqueAddress* address  = registry->address () ;
  if ( 0 == address  ) { return Error("Address points to NULL!"); }

  Rndm::Numbers gauss   ( randSvc() , Rndm::Gauss       (   0.0 ,  1.0 ) ) ;
  Rndm::Numbers flat    ( randSvc() , Rndm::Flat        ( -10.0 , 10.0 ) ) ;
  Rndm::Numbers expo    ( randSvc() , Rndm::Exponential (   1.0        ) ) ;
  Rndm::Numbers breit   ( randSvc() , Rndm::BreitWigner (   0.0 ,  1.0 ) ) ;
  Rndm::Numbers poisson ( randSvc() , Rndm::Poisson     (   2.0        ) ) ;
  Rndm::Numbers binom   ( randSvc() , Rndm::Binomial    (   8   , 0.25 ) ) ;

  // get the event tag collection
  Tuple tuple = evtCol( TupleID("COL1") , "The most trivial Event Tag Collection" );

  // event address (the most important information)
  tuple -> column ( "Address" ,       address    ).ignore() ;
  // put 'event' and 'run' number
  tuple -> column ( "evtNum"  ,       s_nEvt     ).ignore() ;
  tuple -> column ( "runNum"  ,       s_nRun     ).ignore() ;
  // put some 'data'
  tuple -> column ( "gauss"   ,       gauss   () ).ignore() ;
  tuple -> column ( "flat"    ,       flat    () ).ignore() ;
  tuple -> column ( "expo"    ,       expo    () ).ignore() ;
  tuple -> column ( "breit"   ,       breit   () ).ignore() ;
  tuple -> column ( "poisson" , (int) poisson () ).ignore() ;
  tuple -> column ( "binom"   , (int) binom   () ).ignore() ;
  tuple -> column ( "flag"    ,   0 > gauss   () ).ignore() ; // boolean

  // The action of is restored again!
  tuple -> write  () ; ///< The action is restored again!           NB !!

  return StatusCode::SUCCESS ;
}
// ============================================================================
using Gaudi::Examples::EvtColAlg;
DECLARE_COMPONENT(EvtColAlg)
// ============================================================================
// The END
// ============================================================================

