// Include files
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
  MsgStream log(msgSvc(),name());
  log << MSG::INFO << "Read handler called for property: " << p << endmsg ;
}

// Update Handler
//------------------------------------------------------------------------------
void PropertyAlg::updateHandler(Property& p)
{
  MsgStream log(msgSvc(),name());
  log << MSG::INFO << "Update handler called for property: " << p << endmsg;
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

  MsgStream log(msgSvc(),"PropertyAlg");

  log << MSG::INFO << "Before Initialization......" << endmsg;

  log << MSG::INFO << "Int    = " << m_int << endmsg;
  log << MSG::INFO << "Int64  = " << m_int64 << endmsg;
  log << MSG::INFO << "UInt64 = " << m_uint64 << endmsg;
  log << MSG::INFO << "Double = " << m_double << endmsg;
  log << MSG::INFO << "String = " << m_string << endmsg;
  log << MSG::INFO << "Bool   = " << m_bool << endmsg;
  log << MSG::INFO << "IntArray    = " << m_intarray << endmsg;
  log << MSG::INFO << "Int64Array  = " << m_int64array << endmsg;
  log << MSG::INFO << "UInt64Array = " << m_uint64array << endmsg;
  log << MSG::INFO << "DoubleArray = " << m_doublearray << endmsg;
  log << MSG::INFO << "StringArray = " << m_stringarray << endmsg;
  log << MSG::INFO << "BoolArray   = " << m_boolarray << endmsg;
  log << MSG::INFO << "EmptyArray  = " << m_emptyarray << endmsg;
  log << MSG::INFO << "IntPairArray  = " << u_intpairarray << endmsg;
  log << MSG::INFO << "DoublePairArray  = " << u_doublepairarray << endmsg;

  log << MSG::INFO << "PInt    = " << p_int << endmsg;
  log << MSG::INFO << "PDouble = " << p_double << endmsg;
  log << MSG::INFO << "PString = " << p_string << endmsg;
  log << MSG::INFO << "PBool   = " << p_bool << endmsg;
  log << MSG::INFO << "PIntArray    = " << p_intarray << endmsg;
  log << MSG::INFO << "PDoubleArray = " << p_doublearray << endmsg;
  log << MSG::INFO << "PStringArray = " << p_stringarray << endmsg;
  log << MSG::INFO << "PBoolArray   = " << p_boolarray << endmsg;

}

//------------------------------------------------------------------------------
StatusCode PropertyAlg::initialize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());

  //
  // Checking the JobOptions interface. Be able to set the properties
  //
  log << MSG::INFO
      << "After Initialization having read the JobOptions file..."
      << endmsg;

  log << MSG::INFO << "Int    = " << m_int << endmsg;
  log << MSG::INFO << "Int64  = " << m_int64 << endmsg;
  log << MSG::INFO << "UInt64 = " << m_uint64 << endmsg;
  log << MSG::INFO << "Double = " << m_double << endmsg;
  log << MSG::INFO << "String = " << m_string << endmsg;
  log << MSG::INFO << "Bool   = " << m_bool << endmsg;
  log << MSG::INFO << "IntArray    = " << m_intarray << endmsg;
  log << MSG::INFO << "Int64Array  = " << m_int64array << endmsg;
  log << MSG::INFO << "UInt64Array = " << m_uint64array << endmsg;
  log << MSG::INFO << "DoubleArray = " << m_doublearray << endmsg;
  log << MSG::INFO << "StringArray = " << m_stringarray << endmsg;
  log << MSG::INFO << "BoolArray   = " << m_boolarray << endmsg;
  log << MSG::INFO << "EmptyArray  = " << m_emptyarray << endmsg;
  log << MSG::INFO << "IntPairArray  = " << u_intpairarray << endmsg;
  log << MSG::INFO << "DoublePairArray  = " << u_doublepairarray << endmsg;

  log << MSG::INFO << "PInt    = " << p_int << endmsg;
  log << MSG::INFO << "PDouble = " << p_double << endmsg;
  log << MSG::INFO << "PString = " << p_string << endmsg;
  log << MSG::INFO << "PBool   = " << p_bool << endmsg;
  log << MSG::INFO << "PIntArray    = " << p_intarray << endmsg;
  log << MSG::INFO << "PDoubleArray = " << p_doublearray << endmsg;
  log << MSG::INFO << "PStringArray = " << p_stringarray << endmsg;
  log << MSG::INFO << "PBoolArray   = " << p_boolarray << endmsg;
  //
  // Checking units
  //
  for (unsigned int i = 0; i < u_doublearrayunits.size(); i++ ) {

    if( u_doublearrayunits[i] != u_doublearray[i] ) {
      log << MSG::ERROR
          << format
        ("DoubleArrayWithUnits[%d] = %g and should be %g",
         i, u_doublearrayunits[i], u_doublearray[i] ) << endmsg;
    }
    else {
      log << MSG::INFO << format("DoubleArrayWithUnits[%d] = %g",
                                   i, u_doublearrayunits[i] ) << endmsg;
    }
  }

  //
  // Checking the Property Verifier
  //
  log << MSG::INFO
      << "===============Checking Property Verifier ===============" << endmsg;

  log << MSG::INFO << "Playing with PropertyVerifiers..." << endmsg;

  p_int.verifier().setBounds( 0, 200);
  p_int = 155;
  //log << MSG::INFO << "PInt= " << p_int << " [should be 155, bounds are " <<
  //                     p_int.verifier().lower() << ", " <<
  //                     p_int.verifier().upper() << " ]" << endmsg;
  log << MSG::INFO
      << format
    ("PInt= %d [should be 155, bounds are %d, %d]",
     (int)p_int, (int)p_int.verifier().lower(),
     (int)p_int.verifier().upper() ) << endmsg;
  try {
    p_int = 255;
  } catch (...) {
    log << MSG::INFO
        << "Got an exception when setting a value outside bounds" << endmsg;
  }
  log << MSG::INFO << "PInt= " << p_int << " [should be 155]" << endmsg;

  //
  //  Checking the Property CallBacks
  //
  log << MSG::INFO
      << "===============Checking Property CallBaks ===============" << endmsg;

  double d;
  log << MSG::INFO << "Accessing PDouble ... " << endmsg;
  d = p_double;

  log << MSG::INFO << "Value obtained is: " << d << endmsg;

  log << MSG::INFO << "Updating PDouble ... "<< endmsg;
  p_double = 999.;

  //
  //  Checking Accessing Properties by string
  //

  log << MSG::INFO << "==========Checking Accesing Properties by string=========" << endmsg;

  SmartIF<IProperty> appmgr(serviceLocator());
  //StatusCode sc = serviceLocator()->service("ApplicationMgr", appmgr);
  if( !appmgr.isValid() ) {
    log << MSG::ERROR << "Unable to locate the ApplicationMgr" << endmsg;
  }
  else {
    std::string value("empty");
    appmgr->getProperty("ExtSvc", value).ignore();
    log << MSG::INFO << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

    appmgr->setProperty("ExtSvc", "[\"EvtDataSvc/EventDataSvc\", \"DetDataSvc/DetectorDataSvc\"]").ignore();
    log << MSG::INFO << " Set property ApplicationMgr.ExtSvc = " << " [\"EvtDataSvc/EventDataSvc\", \"DetDataSvc/DetectorDataSvc\"]" << ";" << endmsg;
    appmgr->getProperty("ExtSvc", value).ignore();
    log << MSG::INFO << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

    appmgr->setProperty("ExtSvc", "[ 'EventDataSvc', 'DetectorDataSvc']").ignore();
    log << MSG::INFO << " Set property ApplicationMgr.ExtSvc = " << " [ 'EventDataSvc', 'DetectorDataSvc']" << ";" << endmsg;
    appmgr->getProperty("ExtSvc", value).ignore();
    log << MSG::INFO << " Got property ApplicationMgr.ExtSvc = " << value << ";" << endmsg;

  }
  // Testing setting bool
  const char* scases[] = {"true", "false", "True", "False", "TRUE", "FALSE", "T", "F", "10" };
  bool  bcases[] = { true,   false,   true,   false,   true,   false,  true, true, true};
  for (unsigned int i = 0; i < sizeof(scases)/sizeof(char*); i++ ) {
    setProperty( "PBool", scases[i] ).ignore();
    if( p_bool  != bcases[i] ) log << MSG::ERROR << "PBool can not be set to "<< scases[i] << endmsg;
    setProperty( "Bool", scases[i] ).ignore();
    if( m_bool  != bcases[i] ) log << MSG::ERROR << "Bool can not be set to "<< scases[i] << endmsg;
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
  IJobOptionsSvc* jopts;
  if( service("JobOptionsSvc", jopts ).isSuccess() ) {
    // Dump of the catalogue
    log << MSG::INFO << "=================================================" << endmsg;
    log << MSG::INFO << "Dump of the property catalogue.... " << endmsg;
    std::vector<std::string> clients = jopts->getClients();
    std::vector<std::string>::iterator cit;
    std::vector<const Property*>::const_iterator pit;

    for( cit = clients.begin(); cit != clients.end(); cit++ ) {
      log << MSG::INFO << " Properties of " <<  *cit << ": ";
      const std::vector<const Property*>* properties = jopts->getProperties(*cit);
      for( pit = properties->begin(); pit != properties->end(); pit++ ) {
        log << (*pit)->name();
        if( (pit+1) != properties->end())  log << ", ";
      }
      log << endmsg;
    }
    log << MSG::INFO << "=================================================" << endmsg;

    // Change an option of my own....
    jopts->addPropertyToCatalogue( name(), StringProperty("PInt", "155") ).ignore();
    std::vector<std::string> values;
    values.push_back("12.12");
    values.push_back("13.13");
    StringArrayProperty sap("DoubleArray", values );
    if( jopts->addPropertyToCatalogue( name(), sap ).isSuccess() ) {
      log << "Changed property DoubleArray in catalogue" << endmsg;
      jopts->setMyProperties(name(),this).ignore();
      log << MSG::INFO << "DoubleArray = " << m_doublearray << endmsg;
    } else {
      log << MSG::ERROR  << "Unable to change property in catalogue" << endmsg;
    }
    log << MSG::INFO << "=================================================" << endmsg;

    jopts->release();
  }
  else {
    log << MSG::ERROR << " Unable to access the JobOptionsSvc" << endmsg;
  }

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode PropertyAlg::execute() {
//------------------------------------------------------------------------------
  MsgStream         log( msgSvc(), name() );
  log << MSG::INFO << "executing...." << endmsg;

  return StatusCode::SUCCESS;
}


//------------------------------------------------------------------------------
StatusCode PropertyAlg::finalize() {
//------------------------------------------------------------------------------
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "finalizing...." << endmsg;

  return StatusCode::SUCCESS;
}

// Static Factory declaration
DECLARE_COMPONENT(PropertyAlg)
