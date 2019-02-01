// Include files
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"
#include <sstream>

#include "GaudiKernel/SerializeSTL.h"

#include "PropertyAlg.h"

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
#  pragma warning( disable : 1572 )
#endif

// Read Handler
//------------------------------------------------------------------------------
void PropertyAlg::readHandler( Gaudi::Details::PropertyBase& p ) {
  // do not print messages if we are created in genconf
  const std::string cmd = System::cmdLineArgs()[0];
  if ( cmd.find( "genconf" ) != std::string::npos ) return;

  info() << "Read handler called for property: " << p << endmsg;
}

// Update Handler
//------------------------------------------------------------------------------
void PropertyAlg::updateHandler( Gaudi::Details::PropertyBase& p ) {
  // avoid the readHandler (which writes to the _same_ MsgStream!) from writing
  // something in the middle of the printout of this message...
  std::ostringstream os;
  os << p;
  info() << "Update handler called for property: " << os.str() << endmsg;
}

// Constructor
//------------------------------------------------------------------------------
PropertyAlg::PropertyAlg( const std::string& name, ISvcLocator* ploc ) : Algorithm( name, ploc ) {
  //------------------------------------------------------------------------------
  // Associate read and update handlers

  p_double.declareUpdateHandler( &PropertyAlg::updateHandler, this );
  p_double.declareReadHandler( &PropertyAlg::readHandler, this );

  {
    // do not print messages if we are created in genconf
    const std::string cmd = System::cmdLineArgs()[0];
    if ( cmd.find( "genconf" ) != std::string::npos ) return;
  }

  info() << "Before Initialization......" << endmsg;

  info() << "Int    = " << m_int.value() << endmsg;
  info() << "Int64  = " << m_int64.value() << endmsg;
  info() << "UInt64 = " << m_uint64.value() << endmsg;
  info() << "Double = " << m_double.value() << endmsg;
  info() << "String = " << m_string.value() << endmsg;
  info() << "Bool   = " << m_bool.value() << endmsg;
  info() << "IntArray    = " << m_intarray.value() << endmsg;
  info() << "Int64Array  = " << m_int64array.value() << endmsg;
  info() << "UInt64Array = " << m_uint64array.value() << endmsg;
  info() << "DoubleArray = " << m_doublearray.value() << endmsg;
  info() << "StringArray = " << m_stringarray.value() << endmsg;
  info() << "BoolArray   = " << m_boolarray.value() << endmsg;
  info() << "EmptyArray  = " << m_emptyarray.value() << endmsg;
  info() << "IntPairArray  = " << u_intpairarray.value() << endmsg;
  info() << "DoublePairArray  = " << u_doublepairarray.value() << endmsg;

  info() << "PInt    = " << p_int << endmsg;
  std::ostringstream os;
  os << p_double; // avoid read handler from printing _during_ info()!
  info() << "PDouble = " << os.str() << endmsg;
  info() << "PString = " << p_string << endmsg;
  info() << "PBool   = " << p_bool << endmsg;
  info() << "PIntArray    = " << p_intarray << endmsg;
  info() << "PDoubleArray = " << p_doublearray << endmsg;
  info() << "PStringArray = " << p_stringarray << endmsg;
  info() << "PBoolArray   = " << p_boolarray << endmsg;
}

//------------------------------------------------------------------------------
StatusCode PropertyAlg::initialize() {
  //------------------------------------------------------------------------------

  //
  // Checking the JobOptions interface. Be able to set the properties
  //
  info() << "After Initialization having read the JobOptions file..." << endmsg;

  info() << "Int    = " << m_int.value() << endmsg;
  info() << "Int64  = " << m_int64.value() << endmsg;
  info() << "UInt64 = " << m_uint64.value() << endmsg;
  info() << "Double = " << m_double.value() << endmsg;
  info() << "String = " << m_string.value() << endmsg;
  info() << "Bool   = " << m_bool.value() << endmsg;
  info() << "IntArray    = " << m_intarray.value() << endmsg;
  info() << "Int64Array  = " << m_int64array.value() << endmsg;
  info() << "UInt64Array = " << m_uint64array.value() << endmsg;
  info() << "DoubleArray = " << m_doublearray.value() << endmsg;
  info() << "StringArray = " << m_stringarray.value() << endmsg;
  info() << "BoolArray   = " << m_boolarray.value() << endmsg;
  info() << "EmptyArray  = " << m_emptyarray.value() << endmsg;
  info() << "IntPairArray  = " << u_intpairarray.value() << endmsg;
  info() << "DoublePairArray  = " << u_doublepairarray.value() << endmsg;

  info() << "PInt    = " << p_int << endmsg;
  std::ostringstream os;
  os << p_double; // avoid read handler from printing _during_ info()!
  info() << "PDouble = " << os.str() << endmsg;
  info() << "PString = " << p_string << endmsg;
  info() << "PBool   = " << p_bool << endmsg;
  info() << "PIntArray    = " << p_intarray << endmsg;
  info() << "PDoubleArray = " << p_doublearray << endmsg;
  info() << "PStringArray = " << p_stringarray << endmsg;
  info() << "PBoolArray   = " << p_boolarray << endmsg;
  //
  // Checking units
  //
  auto& doublearrayunits = u_doublearrayunits.value();
  auto& doublearray      = u_doublearray.value();
  for ( unsigned int i = 0; i < doublearrayunits.size(); i++ ) {

    if ( doublearrayunits[i] != doublearray[i] ) {
      error() << format( "DoubleArrayWithUnits[%d] = %g and should be %g", i, doublearrayunits[i], doublearray[i] )
              << endmsg;
    } else {
      info() << format( "DoubleArrayWithUnits[%d] = %g", i, doublearrayunits[i] ) << endmsg;
    }
  }

  //
  // Checking the Property Verifier
  //
  info() << "===============Checking Property Verifier ===============" << endmsg;

  info() << "Playing with PropertyVerifiers..." << endmsg;

  p_int.verifier().setBounds( 0, 200 );
  p_int = 155;
  // info() << "PInt= " << p_int << " [should be 155, bounds are " <<
  //                     p_int.verifier().lower() << ", " <<
  //                     p_int.verifier().upper() << " ]" << endmsg;
  info() << format( "PInt= %d [should be 155, bounds are %d, %d]", (int)p_int, (int)p_int.verifier().lower(),
                    (int)p_int.verifier().upper() )
         << endmsg;
  try {
    p_int = 255;
  } catch ( ... ) { info() << "Got an exception when setting a value outside bounds" << endmsg; }
  info() << "PInt= " << p_int << " [should be 155]" << endmsg;

  //
  //  Checking the Property CallBacks
  //
  info() << "===============Checking Property CallBaks ===============" << endmsg;

  double d;
  info() << "Accessing PDouble ... " << endmsg;
  d = p_double;

  info() << "Value obtained is: " << d << endmsg;

  info() << "Updating PDouble ... " << endmsg;
  p_double = 999.;

  //
  //  Checking Accessing Properties by string
  //

  info() << "==========Checking Accesing Properties by string=========" << endmsg;

  auto appmgr = serviceLocator()->as<IProperty>();
  // StatusCode sc = serviceLocator()->service("ApplicationMgr", appmgr);
  if ( !appmgr ) {
    error() << "Unable to locate the ApplicationMgr" << endmsg;
  } else {
    std::string value( "empty" );
    appmgr->getProperty( "ExtSvc", value ).ignore();
    info() << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

    appmgr->setProperty( "ExtSvc", "[\"EvtDataSvc/EventDataSvc\", \"DetDataSvc/DetectorDataSvc\"]" ).ignore();
    info() << " Set property ApplicationMgr.ExtSvc = "
           << " [\"EvtDataSvc/EventDataSvc\", \"DetDataSvc/DetectorDataSvc\"]"
           << ";" << endmsg;
    appmgr->getProperty( "ExtSvc", value ).ignore();
    info() << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

    appmgr->setProperty( "ExtSvc", "[ 'EventDataSvc', 'DetectorDataSvc']" ).ignore();
    info() << " Set property ApplicationMgr.ExtSvc = "
           << " [ 'EventDataSvc', 'DetectorDataSvc']"
           << ";" << endmsg;
    appmgr->getProperty( "ExtSvc", value ).ignore();
    info() << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;
  }
  // Testing setting bool
  std::vector<std::tuple<std::string, bool>> cases = {
      std::make_tuple( "true", true ),   std::make_tuple( "false", false ), std::make_tuple( "True", true ),
      std::make_tuple( "False", false ), std::make_tuple( "TRUE", true ),   std::make_tuple( "FALSE", false ),
      std::make_tuple( "T", true ),  // not expected to work
      std::make_tuple( "F", false ), // not expected to work
      std::make_tuple( "10", true ), // not expected to work
  };
  StatusCode sc = StatusCode::SUCCESS;
  for ( auto& c : cases ) {
    p_bool = m_bool = !std::get<1>( c );
    try {
      sc = setProperty( "PBool", std::get<0>( c ) );
    } catch ( ... ) { sc = StatusCode::FAILURE; }
    if ( !sc || ( p_bool != std::get<1>( c ) ) ) {
      error() << "PBool can not be set to " << std::get<0>( c ) << endmsg;
    }
    try {
      sc = setProperty( "Bool", std::get<0>( c ) );
    } catch ( ... ) { sc = StatusCode::FAILURE; }
    if ( !sc || ( m_bool != std::get<1>( c ) ) ) { error() << "Bool can not be set to " << std::get<0>( c ) << endmsg; }
  }

  // Testing the control of the output level directly from MessageSvc
  MsgStream newlog( msgSvc(), "MsgTest" );
  newlog << MSG::VERBOSE << "This should be printed if threshold is VERBOSE" << endmsg;
  newlog << MSG::DEBUG << "This should be printed if threshold is DEBUG" << endmsg;
  newlog << MSG::INFO << "This should be printed if threshold is INFO" << endmsg;
  newlog << MSG::WARNING << "This should be printed if threshold is WARNING" << endmsg;
  newlog << MSG::ERROR << "This should be printed if threshold is ERROR" << endmsg;
  newlog << MSG::FATAL << "This should be printed if threshold is FATAL" << endmsg;
  newlog << MSG::ALWAYS << "This should be printed ALWAYS" << endmsg;

  // Testing access to the JobOptions catalogue
  auto jopts = service<IJobOptionsSvc>( "JobOptionsSvc" );
  if ( !jopts ) {
    error() << " Unable to access the JobOptionsSvc" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Dump of the catalogue
  info() << "=================================================" << endmsg;
  info() << "Dump of the property catalogue.... " << endmsg;
  for ( const auto& cit : jopts->getClients() ) {
    using GaudiUtils::details::ostream_joiner;
    ostream_joiner(
        info() << " Properties of " << cit << ": ", *jopts->getProperties( cit ), ", ",
        []( MsgStream& os, const Gaudi::Details::PropertyBase* p ) -> MsgStream& { return os << p->name(); } )
        << endmsg;
  }
  info() << "=================================================" << endmsg;

  // Change an option of my own....
  jopts->addPropertyToCatalogue( name(), Gaudi::Property<std::string>( "PInt", "155" ) ).ignore();
  Gaudi::Property<std::vector<std::string>> sap( "DoubleArray", {"12.12", "13.13"} );
  if ( jopts->addPropertyToCatalogue( name(), sap ).isSuccess() ) {
    info() << "Changed property DoubleArray in catalogue" << endmsg;
    jopts->setMyProperties( name(), this ).ignore();
    info() << "DoubleArray = " << m_doublearray.value() << endmsg;
  } else {
    error() << "Unable to change property in catalogue" << endmsg;
  }
  info() << "=================================================" << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode PropertyAlg::execute() {
  //------------------------------------------------------------------------------
  info() << "executing...." << endmsg;
  return StatusCode::SUCCESS;
}

//------------------------------------------------------------------------------
StatusCode PropertyAlg::finalize() {
  //------------------------------------------------------------------------------
  info() << "finalizing...." << endmsg;
  return StatusCode::SUCCESS;
}

// Static Factory declaration
DECLARE_COMPONENT( PropertyAlg )
