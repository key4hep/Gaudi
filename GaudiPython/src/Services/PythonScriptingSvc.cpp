// $Id: PythonScriptingSvc.cpp,v 1.18 2008/10/27 21:12:08 marcocle Exp $

#include "Python.h"

// Include Files
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/SmartIF.h"

#include "PythonScriptingSvc.h"

#include <fstream>
#include <sstream>

// Special for Unixes
#if defined(linux)
  #include "dlfcn.h"
#endif

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_SERVICE_FACTORY(PythonScriptingSvc)

//----------------------------------------------------------------------------------
PythonScriptingSvc::PythonScriptingSvc( const std::string& name, ISvcLocator* svc )
//----------------------------------------------------------------------------------
: base_class(name, svc) {
  // Declare the startup script Property
  declareProperty( "StartupScript", m_startupScript = "" );
}

//----------------------------------------------------------------------------------
PythonScriptingSvc::~PythonScriptingSvc() { }
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
StatusCode PythonScriptingSvc::initialize()
//----------------------------------------------------------------------------------
{
  // initialize the Service Base class
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;

  MsgStream log( msgSvc(), name() );

  // Setup startup script. If none is explicitly specified, then
  // use the ApplicationMgr JobOptionsPath property as long as
  // the JobOptionsType property is set to "NONE".
  if( m_startupScript == "" ) {
    SmartIF<IProperty> prpMgr(serviceLocator());
    if ( prpMgr.isValid() )   {
      StringProperty tmp;
      tmp.assign(prpMgr->getProperty("JobOptionsType"));
      if ( tmp.value( ) == "NONE" ) {
        tmp.assign(prpMgr->getProperty("JobOptionsPath"));
        m_startupScript = tmp;
      }
    }
  }

  char* progName[] = { const_cast<char*>("GaudiPython") };

  // Initialize the Python interpreter.  Required.
  Py_Initialize();
  // Set argv for Tkinter (needs program name)
  PySys_SetArgv( 1, progName );
  // Get the Python version
  std::string fullversion = Py_GetVersion();
  std::string version( fullversion, 0, fullversion.find_first_of(' '));
  std::string vers(version, 0, version.find_first_of('.',version.find_first_of('.')+1));
  log << MSG::INFO << "Python version: [" << vers << "]" << endmsg;

#if defined(linux)
  // This is hack to make global the python symbols
  // which are needed by the other python modules
  // (eg. readline, math, etc,) libraries.
  std::string libname = "libpython" + vers + ".so";
  dlopen(libname.c_str(), RTLD_GLOBAL | RTLD_LAZY);
#endif


  // Startup commands
  PyRun_SimpleString( "from gaudimodule import *" );
  PyRun_SimpleString( "g      = AppMgr()" );
  // backward compatibility with SIPython
  PyRun_SimpleString( "theApp = g" );
  PyRun_SimpleString( "def Service(n): return g.service(n)" );
  PyRun_SimpleString( "def Algorithm(n): return g.algorithm(n)" );
  PyRun_SimpleString( "def Property(n): return g.service(n)" );
  // For command-line completion (unix only)
#if !defined( _WIN32 )
  PyRun_SimpleString( "import rlcompleter");
  PyRun_SimpleString( "rlcompleter.readline.parse_and_bind('tab: complete')");
#endif
  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------------------
StatusCode PythonScriptingSvc::finalize()
//----------------------------------------------------------------------------------
{
  // Finalize this specific service
  StatusCode sc = Service::finalize();
  if ( sc.isFailure() ) {
      return sc;
  }

  // Shutdown the Python interpreter
  Py_Finalize();
  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------------------
StatusCode PythonScriptingSvc::run()
//----------------------------------------------------------------------------------
{
  MsgStream log( msgSvc(), name() );
  if ( m_startupScript != "" ) {
    std::ifstream file(m_startupScript.c_str());
    std::stringstream stream;
    if( file ) {
      char ch;
      while( file.get(ch) ) stream.put(ch);
      PyRun_SimpleString( const_cast<char*>(stream.str().c_str()) );
      file.close();
    }
    else {
      log << MSG::WARNING << "Python startup file " << m_startupScript << " not found" << endmsg;
    }
  }
  PyRun_InteractiveLoop(stdin, "\0");
  return StatusCode::SUCCESS;
}

