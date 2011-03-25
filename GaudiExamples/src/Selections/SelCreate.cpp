// $Id: SelCreate.cpp,v 1.1 2008/10/10 08:06:33 marcocle Exp $
// ============================================================================
// Include files
// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
// ============================================================================
// GaudiAlg
// ============================================================================
#include "GaudiAlg/GaudiAlgorithm.h"
// ============================================================================
// Local
// ============================================================================
#include "MyTrack.h"
// ============================================================================
#ifdef __ICC
// disable icc remark #2259: non-pointer conversion from "double" to "float" may lose significant bits
#pragma warning(disable:2259)
#elif defined(WIN32)
// disable warning
//   C4244: 'argument' : conversion from 'double' to 'float', possible loss of data
#pragma warning(disable:4244)
#endif

namespace Gaudi
{
  namespace Examples
  {
    /** @class SelCreate
     *  Simple class to create few "containers" in TES
     *  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
     *  @date 2008-07-23
     */
    class SelCreate : public GaudiAlgorithm
    {
      // ======================================================================
      /// friend factory for instantiation
      friend class AlgFactory<Gaudi::Examples::SelCreate> ;
      // ======================================================================
    public:
      // ======================================================================
      /// the only one essential method
      virtual StatusCode execute()
      {
        // some random number generators, just to provide the numbers
        static Rndm::Numbers  gauss   ( randSvc () , Rndm::Gauss   (   0.0 ,   1.0 ) ) ;
        static Rndm::Numbers  flat    ( randSvc () , Rndm::Flat    (  20.0 , 100.0 ) ) ;

        // create the data
        Gaudi::Examples::MyTrack::Container* tracks =
          new Gaudi::Examples::MyTrack::Container() ;

        // register the container in TES
        put ( tracks , name() ) ;


        for ( int i = 0 ; i < 100 ; ++i )
        {
          // create new track
          Gaudi::Examples::MyTrack* track = new Gaudi::Examples::MyTrack() ;

          // fill it with some "data"
          track -> setPx ( gauss ()           ) ;
          track -> setPy ( gauss ()           ) ;
          track -> setPz ( gauss () + flat () ) ;

          // insert it into the container
          tracks -> insert ( track ) ;
        }

        typedef Gaudi::NamedRange_<Gaudi::Examples::MyTrack::ConstVector> Range ;
        if ( !exist<Range> ( name()  ) )
        { err () << "No Range is available at location " << name() << endmsg ; }

        // test "get-or-create":

        Range r1 = getOrCreate<Range,Gaudi::Examples::MyTrack::Container> ( name() + "_1" ) ;
        r1.empty(); // avoid icc remark #177: variable "X" was declared but never referenced
        Range r2 = getOrCreate<Range,Gaudi::Examples::MyTrack::Selection> ( name() + "_2" ) ;
        r2.empty(); // avoid icc remark #177: variable "X" was declared but never referenced

        getOrCreate<Gaudi::Examples::MyTrack::Container,
          Gaudi::Examples::MyTrack::Container> ( name() + "_3" ) ;
        getOrCreate<Gaudi::Examples::MyTrack::Selection,
          Gaudi::Examples::MyTrack::Selection> ( name() + "_4" ) ;


        return StatusCode::SUCCESS ;
      }
      // ======================================================================
    protected:
      // ======================================================================
      /** standard constructor
       *  @param name the algorithm instance name
       *  @param pSvc pointer to Service Locator
       */
      SelCreate ( const std::string& name ,   //    the algorithm instance name
                  ISvcLocator*       pSvc )   // pointer to the Service Locator
        : GaudiAlgorithm ( name , pSvc )
      {}
      /// virtual (and protected) destructor
      virtual ~SelCreate() {}
      // ======================================================================
    private:
      // ======================================================================
      /// the default constructor is disabled
      SelCreate () ;                                  // no default constructor
      /// copy constructor is disabled
      SelCreate ( const SelCreate& ) ;                   // no copy constructor
      /// assignement operator is disabled
      SelCreate& operator=( const SelCreate& ) ;     // no assignement operator
      // ======================================================================
    } ;
    // ========================================================================
  } // end of namespace Gaudi::Examples
} // end of namespace Gaudi
// ============================================================================
/// The factory (needed for instantiation)
DECLARE_NAMESPACE_ALGORITHM_FACTORY(Gaudi::Examples,SelCreate)
// ============================================================================
// The END
// ============================================================================

