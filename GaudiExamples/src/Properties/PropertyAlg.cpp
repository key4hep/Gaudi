// Include files
#include <sstream>
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/IJobOptionsSvc.h"
#include "GaudiKernel/SmartIF.h"

#include "GaudiKernel/SerializeSTL.h"

#include "PropertyAlg.h"

#ifdef __ICC
// disable icc remark #1572: floating-point equality and inequality comparisons are unreliable
#pragma warning(disable:1572)
#endif

// Read Handler
//------------------------------------------------------------------------------
void PropertyAlg::readHandler(Property& p)
{
  info() << "Read handler called for property: " << p << endmsg ;
}

// Update Handler
//------------------------------------------------------------------------------
void PropertyAlg::updateHandler(Property& p)
{
  // avoid the readHandler (which writes to the _same_ MsgStream!) from writing
  // something in the middle of the printout of this message...
  std::ostringstream os; os << p;
  info() << "Update handler called for property: " << os.str() << endmsg;
}

// Constructor
//------------------------------------------------------------------------------
PropertyAlg::PropertyAlg(const std::string& name, ISvcLocator* ploc)
           : Algorithm(name, ploc) {
//------------------------------------------------------------------------------
  // Declare the algorithm's properties

  declareProperty( "Int",    m_int    = 100);
  declareProperty( "Int64",  m_int64  = 100);
  declareProperty( "UInt64", m_uint64 = 100);
  declareProperty( "Double", m_double = 100.);
  declareProperty( "String", m_string = "hundred");
  declareProperty( "Bool",   m_bool   = true);

  declareProperty( "IntArray",    m_intarray );
  declareProperty( "Int64Array",  m_int64array );
  declareProperty( "UInt64Array", m_uint64array );
  declareProperty( "DoubleArray", m_doublearray);
  declareProperty( "StringArray", m_stringarray);
  declareProperty( "BoolArray",   m_boolarray);
  declareProperty( "EmptyArray",  m_emptyarray);

  declareProperty( "IntPairArray", u_intpairarray);
  declareProperty( "DoublePairArray", u_doublepairarray);

  // Declare the rest of the Algorithm properties

  declareProperty( "PInt",    p_int    = 100);
  declareProperty( "PDouble", p_double = 100.);
  declareProperty( "PString", p_string = "hundred");
  declareProperty( "PBool",   p_bool   = false);

  declareProperty( "PIntArray",    p_intarray );
  declareProperty( "PDoubleArray", p_doublearray);
  declareProperty( "PStringArray", p_stringarray);
  declareProperty( "PBoolArray",   p_boolarray);

  // Units testing
  declareProperty( "DoubleArrayWithUnits",    u_doublearrayunits);
  declareProperty( "DoubleArrayWithoutUnits", u_doublearray);

  // Associate read and update handlers

  p_double.declareUpdateHandler( &PropertyAlg::updateHandler, this );
  p_double.declareReadHandler( &PropertyAlg::readHandler, this );


  info() << "Before Initialization......" << endmsg;

  info() << "Int    = " << m_int << endmsg;
  info() << "Int64  = " << m_int64 << endmsg;
  info() << "UInt64 = " << m_uint64 << endmsg;
  info() << "Double = " << m_double << endmsg;
  info() << "String = " << m_string << endmsg;
  info() << "Bool   = " << m_bool << endmsg;
  info() << "IntArray    = " << m_intarray << endmsg;
  info() << "Int64Array  = " << m_int64array << endmsg;
  info() << "UInt64Array = " << m_uint64array << endmsg;
  info() << "DoubleArray = " << m_doublearray << endmsg;
  info() << "StringArray = " << m_stringarray << endmsg;
  info() << "BoolArray   = " << m_boolarray << endmsg;
  info() << "EmptyArray  = " << m_emptyarray << endmsg;
  info() << "IntPairArray  = " << u_intpairarray << endmsg;
  info() << "DoublePairArray  = " << u_doublepairarray << endmsg;

  info() << "PInt    = " << p_int << endmsg;
  std::ostringstream os; os << p_double; // avoid read handler from printing _during_ info()!
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

  info() << "Int    = " << m_int << endmsg;
  info() << "Int64  = " << m_int64 << endmsg;
  info() << "UInt64 = " << m_uint64 << endmsg;
  info() << "Double = " << m_double << endmsg;
  info() << "String = " << m_string << endmsg;
  info() << "Bool   = " << m_bool << endmsg;
  info() << "IntArray    = " << m_intarray << endmsg;
  info() << "Int64Array  = " << m_int64array << endmsg;
  info() << "UInt64Array = " << m_uint64array << endmsg;
  info() << "DoubleArray = " << m_doublearray << endmsg;
  info() << "StringArray = " << m_stringarray << endmsg;
  info() << "BoolArray   = " << m_boolarray << endmsg;
  info() << "EmptyArray  = " << m_emptyarray << endmsg;
  info() << "IntPairArray  = " << u_intpairarray << endmsg;
  info() << "DoublePairArray  = " << u_doublepairarray << endmsg;

  info() << "PInt    = " << p_int << endmsg;
  std::ostringstream os; os << p_double; // avoid read handler from printing _during_ info()!
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
  for (unsigned int i = 0; i < u_doublearrayunits.size(); i++ ) {

    if( u_doublearrayunits[i] != u_doublearray[i] ) {
      error()
          << format
        ("DoubleArrayWithUnits[%d] = %g and should be %g",
         i, u_doublearrayunits[i], u_doublearray[i] ) << endmsg;
    }
    else {
      info() << format("DoubleArrayWithUnits[%d] = %g",
                                   i, u_doublearrayunits[i] ) << endmsg;
    }
  }

  //
  // Checking the Property Verifier
  //
  info()
      << "===============Checking Property Verifier ===============" << endmsg;

  info() << "Playing with PropertyVerifiers..." << endmsg;

  p_int.verifier().setBounds( 0, 200);
  p_int = 155;
  //info() << "PInt= " << p_int << " [should be 155, bounds are " <<
  //                     p_int.verifier().lower() << ", " <<
  //                     p_int.verifier().upper() << " ]" << endmsg;
  info()
      << format
    ("PInt= %d [should be 155, bounds are %d, %d]",
     (int)p_int, (int)p_int.verifier().lower(),
     (int)p_int.verifier().upper() ) << endmsg;
  try {
    p_int = 255;
  } catch (...) {
    info()
        << "Got an exception when setting a value outside bounds" << endmsg;
  }
  info() << "PInt= " << p_int << " [should be 155]" << endmsg;

  //
  //  Checking the Property CallBacks
  //
  info()
      << "===============Checking Property CallBaks ===============" << endmsg;

  double d;
  info() << "Accessing PDouble ... " << endmsg;
  d = p_double;

  info() << "Value obtained is: " << d << endmsg;

  info() << "Updating PDouble ... "<< endmsg;
  p_double = 999.;

  //
  //  Checking Accessing Properties by string
  //

  info() << "==========Checking Accesing Properties by string=========" << endmsg;

  auto appmgr = serviceLocator()->as<IProperty>();
  //StatusCode sc = serviceLocator()->service("ApplicationMgr", appmgr);
  if( !appmgr ) {
    error() << "Unable to locate the ApplicationMgr" << endmsg;
  }
  else {
    std::string value("empty");
    appmgr->getProperty("ExtSvc", value).ignore();
    info() << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

    appmgr->setProperty("ExtSvc", "[\"EvtDataSvc/EventDataSvc\", \"DetDataSvc/DetectorDataSvc\"]").ignore();
    info() << " Set property ApplicationMgr.ExtSvc = " << " [\"EvtDataSvc/EventDataSvc\", \"DetDataSvc/DetectorDataSvc\"]" << ";" << endmsg;
    appmgr->getProperty("ExtSvc", value).ignore();
    info() << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

    appmgr->setProperty("ExtSvc", "[ 'EventDataSvc', 'DetectorDataSvc']").ignore();
    info() << " Set property ApplicationMgr.ExtSvc = " << " [ 'EventDataSvc', 'DetectorDataSvc']" << ";" << endmsg;
    appmgr->getProperty("ExtSvc", value).ignore();
    info() << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

  }
  // Testing setting bool
  const char* scases[] = {"true", "false", "True", "False", "TRUE", "FALSE", "T", "F", "10" };
  bool  bcases[] = { true,   false,   true,   false,   true,   false,  true, true, true};
  for (unsigned int i = 0; i < sizeof(scases)/sizeof(char*); i++ ) {
    setProperty( "PBool", scases[i] ).ignore();
    if( p_bool  != bcases[i] ) error() << "PBool can not be set to "<< scases[i] << endmsg;
    setProperty( "Bool", scases[i] ).ignore();
    if( m_bool  != bcases[i] ) error() << "Bool can not be set to "<< scases[i] << endmsg;
  }

  // Testing the control of the output level directly from MessageSvc
  MsgStream newlog(msgSvc(),"MsgTest" );
  newlog << MSG::VERBOSE << "This should be printed if threshold is VERBOSE" << endmsg;
  newlog << MSG::DEBUG << "This should be printed if threshold is DEBUG" << endmsg;
  newlog << MSG::INFO << "This should be printed if threshold is INFO" << endmsg;
  newlog << MSG::WARNING << "This should be printed if threshold is WARNING" << endmsg;
  newlog << MSG::ERROR << "This should be printed if threshold is ERROR" << endmsg;
  newlog << MSG::FATAL << "This should be printed if threshold is FATAL" << endmsg;
  newlog << MSG::ALWAYS << "This should be printed ALWAYS" << endmsg;

  // Testing access to the JobOptions catalogue
  auto jopts = service<IJobOptionsSvc>("JobOptionsSvc");
  if (!jopts) {
    error() << " Unable to access the JobOptionsSvc" << endmsg;
    return StatusCode::SUCCESS;
  }

  // Dump of the catalogue
  info() << "=================================================" << endmsg;
  info() << "Dump of the property catalogue.... " << endmsg;
  for( const auto&  cit : jopts->getClients() ) {
    using GaudiUtils::details::ostream_joiner;
    ostream_joiner( info() << " Properties of " <<  cit << ": ",
                    *jopts->getProperties(cit), ", ",
                    [](MsgStream& os, const Property* p) -> MsgStream&
                    { return os << p->name(); } )
    << endmsg;
  }
  info() << "=================================================" << endmsg;

  // Change an option of my own....
  jopts->addPropertyToCatalogue( name(), StringProperty("PInt", "155") ).ignore();
  StringArrayProperty sap("DoubleArray", { "12.12", "13.13" } );
  if( jopts->addPropertyToCatalogue( name(), sap ).isSuccess() ) {
    info() << "Changed property DoubleArray in catalogue" << endmsg;
    jopts->setMyProperties(name(),this).ignore();
    info() << "DoubleArray = " << m_doublearray << endmsg;
  } else {
    error()  << "Unable to change property in catalogue" << endmsg;
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
DECLARE_COMPONENT(PropertyAlg)
