#include "Python.h"

// Python 3 compatibility
#if PY_MAJOR_VERSION >= 3

#  define PySys_SetArgv_Char_t wchar_t

#else

#  define PySys_SetArgv_Char_t char

#endif

// Include Files
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SmartIF.h"

#include "PythonScriptingSvc.h"

#include <fstream>
#include <sstream>

// Special for Unixes
#if defined( __linux )
#  include "dlfcn.h"
#endif

// Instantiation of a static factory class used by clients to create
//  instances of this service
DECLARE_COMPONENT( PythonScriptingSvc )

//----------------------------------------------------------------------------------
PythonScriptingSvc::PythonScriptingSvc( const std::string& name, ISvcLocator* svc )
    //----------------------------------------------------------------------------------
    : base_class( name, svc ) {
  // Declare the startup script Property
  declareProperty( "StartupScript", m_startupScript = "" );
}

//----------------------------------------------------------------------------------
PythonScriptingSvc::~PythonScriptingSvc() {}
//----------------------------------------------------------------------------------

//----------------------------------------------------------------------------------
StatusCode PythonScriptingSvc::initialize()
//----------------------------------------------------------------------------------
{
  // initialize the Service Base class
  StatusCode sc = Service::initialize();
  if ( sc.isFailure() ) return sc;

  // Setup startup script. If none is explicitly specified, then
  // use the ApplicationMgr JobOptionsPath property as long as
  // the JobOptionsType property is set to "NONE".
  if ( m_startupScript.empty() ) {
    auto prpMgr = serviceLocator()->as<IProperty>();
    if ( prpMgr ) {
      Gaudi::Property<std::string> tmp;
      tmp.assign( prpMgr->getProperty( "JobOptionsType" ) );
      if ( tmp.value() == "NONE" ) {
        tmp.assign( prpMgr->getProperty( "JobOptionsPath" ) );
        m_startupScript = tmp;
      }
    }
  }

  // Python 3 compatibility
#if PY_MAJOR_VERSION >= 3
  wchar_t* progName[] = {const_cast<wchar_t*>( L"GaudiPython" )};
#else
  char* progName[] = {const_cast<char*>( "GaudiPython" )};
#endif

  // Initialize the Python interpreter.  Required.
  Py_Initialize();
  // Set argv for Tkinter (needs program name)
  PySys_SetArgv( 1, progName );
  // Get the Python version
  std::string fullversion = Py_GetVersion();
  std::string version( fullversion, 0, fullversion.find_first_of( ' ' ) );
  std::string vers( version, 0, version.find_first_of( '.', version.find_first_of( '.' ) + 1 ) );
  info() << "Python version: [" << vers << "]" << endmsg;

#if defined( __linux )
  // This is hack to make global the python symbols
  // which are needed by the other python modules
  // (eg. readline, math, etc,) libraries.
  std::string libname = "libpython" + vers + ".so";
  dlopen( libname.c_str(), RTLD_GLOBAL | RTLD_LAZY );
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
  PyRun_SimpleString( "import rlcompleter" );
  PyRun_SimpleString( "rlcompleter.readline.parse_and_bind('tab: complete')" );
#endif
  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------------------
StatusCode PythonScriptingSvc::finalize()
//----------------------------------------------------------------------------------
{
  // Finalize this specific service
  StatusCode sc = Service::finalize();
  if ( sc.isFailure() ) return sc;

  // Shutdown the Python interpreter
  Py_Finalize();
  return StatusCode::SUCCESS;
}

//----------------------------------------------------------------------------------
StatusCode PythonScriptingSvc::run()
//----------------------------------------------------------------------------------
{
  if ( !m_startupScript.empty() ) {
    std::ifstream     file{m_startupScript};
    std::stringstream stream;
    if ( file ) {
      std::string buffer;
      file.seekg( 0, std::ios::end );
      buffer.reserve( file.tellg() );
      file.seekg( 0, std::ios::beg );
      buffer.assign( ( std::istreambuf_iterator<char>{file} ), std::istreambuf_iterator<char>{} );
      file.close();
      PyRun_SimpleString( buffer.c_str() );
    } else {
      warning() << "Python startup file " << m_startupScript << " not found" << endmsg;
    }
  }
  PyRun_InteractiveLoop( stdin, "\0" );
  return StatusCode::SUCCESS;
}
