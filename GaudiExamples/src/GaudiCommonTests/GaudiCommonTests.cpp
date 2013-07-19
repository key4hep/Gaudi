// ============================================================================
// GaudiKernel
// ============================================================================
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/IRegistry.h"

// local
#include "GaudiCommonTests.h"

using namespace Gaudi::Examples ;

//-----------------------------------------------------------------------------
// Implementation file for class : GaudiHistoAlgorithm
//
// 2005-08-12 : Chris Jones
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_COMPONENT(GaudiCommonTests)

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
GaudiCommonTests::GaudiCommonTests( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
}
//=============================================================================
// Destructor
//=============================================================================
GaudiCommonTests::~GaudiCommonTests() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode GaudiCommonTests::initialize()
{
  // must be called first
  const StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  return sc;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode GaudiCommonTests::execute()
{
  info() << "Execute" << endmsg;

  const std::string loc1 = "/Event/"+name()+"/Tracks1";
  const std::string loc2 = "/Event/"+name()+"/Tracks2";
  const std::string loc3 = name()+"/Tracks3";
  const std::string loc4 = name()+"/Tracks4";

  const std::string loc5 = "/Event";
  const std::string loc6 = "";

  // tests creating data
  {

    MyTrackVector* Tracks1 = new MyTrackVector();
    MyTrackVector* Tracks2 = new MyTrackVector();
    MyTrackVector* Tracks3 = new MyTrackVector();
    MyTrackVector* Tracks4 = new MyTrackVector();

    // With /Event, with RootInTES
    put( Tracks1, loc1, UseRootInTES );
    // With /Event, without RootInTES
    put( Tracks2, loc2, IgnoreRootInTES );
    // Without /Event, with RootInTES
    put( Tracks3, loc3, UseRootInTES );
    // Without /Event, without RootInTES
    put( Tracks4, loc4, IgnoreRootInTES );

  }

  // tests retrieving data
  {

    MyTrackVector* Tracks1 = get<MyTrackVector>(loc1,UseRootInTES);
    MyTrackVector* Tracks2 = get<MyTrackVector>(loc2,IgnoreRootInTES);
    MyTrackVector* Tracks3 = get<MyTrackVector>(loc3,UseRootInTES);
    MyTrackVector* Tracks4 = get<MyTrackVector>(loc4,IgnoreRootInTES);
    DataObject*    Root1   = get<DataObject   >(loc5,UseRootInTES);
    DataObject*    Root2   = get<DataObject   >(loc5,IgnoreRootInTES);
    DataObject*    Root3   = get<DataObject   >(loc6,UseRootInTES);
    DataObject*    Root4   = get<DataObject   >(loc6,IgnoreRootInTES);
    // should never happen ... Errors should be caught above
    if ( !Tracks1 || !Tracks2 ||
         !Tracks3 || !Tracks4 ||
         !Root1   || !Root2 || !Root3 || !Root4
         ) return Error( "Problem getting data" );

    info() << "Loaded Objects:" << endmsg;
    info() << Tracks1->registry()->identifier() << endmsg;
    info() << Tracks2->registry()->identifier() << endmsg;
    info() << Tracks3->registry()->identifier() << endmsg;
    info() << Tracks4->registry()->identifier() << endmsg;
    info() << Root1->registry()->identifier() << endmsg;
    info() << Root2->registry()->identifier() << endmsg;
    info() << Root3->registry()->identifier() << endmsg;
    info() << Root4->registry()->identifier() << endmsg;
  }

  // Test get without checks
  {
    MyTrackVector* Tracks = NULL;

    Tracks = getIfExists<MyTrackVector>(loc1);
    if (!Tracks) return Error( "Problem getting data" );

    Tracks = getIfExists<MyTrackVector>("Nowhere/To/Be/Found");
    if (Tracks) return Error( "Expected null pointer" );
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode GaudiCommonTests::finalize()
{
  // must be called after all other actions
  return GaudiAlgorithm::finalize();
}

//=============================================================================
