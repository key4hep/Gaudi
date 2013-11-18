#ifdef _WIN32
// Disable a warning in Boost program_options headers:
// inconsistent linkage in program_options/variables_map.hpp
#pragma warning ( disable : 4273 )

// Avoid conflicts between windows and the message service.
#define NOMSG
#define NOGDI
#endif

#ifdef __ICC
// disable icc warning #279: controlling expression is constant
// ... a lot of noise produced by the boost/filesystem/operations.hpp
#pragma warning(disable:279)
// Avoid icc remark #193: zero used for undefined preprocessing identifier "_MSC_VER"
#if !defined(_WIN32) && !defined(_MSC_VER)
#define _MSC_VER 0
#endif
#endif

#include "boost/program_options.hpp"
// the hack for remark #193 is needed only for program_options and breaks regex.
#if defined(__ICC) && !defined(_WIN32) && (_MSC_VER == 0)
#undef _MSC_VER
#endif

// Include files----------------------------------------------------------------
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/exception.hpp"
#include "boost/filesystem/convenience.hpp"
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/trim.hpp"
#include "boost/algorithm/string/case_conv.hpp"
#include "boost/format.hpp"
#include "boost/regex.hpp"

#include "GaudiKernel/System.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/IAlgorithm.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IAuditor.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/HashMap.h"
#include "GaudiKernel/GaudiHandle.h"

#include "GaudiKernel/Time.h"

#include "Reflex/PluginService.h"
#include "Reflex/Reflex.h"
#include "Reflex/SharedLibrary.h"

#include "RVersion.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <set>
#include <vector>


#include "DsoUtils.h"



namespace po = boost::program_options;
namespace fs = boost::filesystem;

using namespace std;
using namespace ROOT::Reflex;

// useful typedefs
typedef std::vector<std::string> Strings_t;
typedef std::vector<fs::path>    LibPathNames_t;

namespace {
  const std::string py_tab = "    ";

  /// Regular expression to validate the property names.
  /// @see http://docs.python.org/reference/lexical_analysis.html#identifiers
  const boost::regex pythonIdentifier("^[a-zA-Z_][a-zA-Z0-9_]*$");
}

class configGenerator
{
  /// name of the package we are processing
  string  m_pkgName;

  /// absolute path to the directory where genconf will store auto-generated
  /// files (Configurables and ConfigurableDb)
  string m_outputDirName;

  /// buffer of auto-generated configurables
  stringstream m_pyBuf;

  /// switch to decide if the generated configurables need
  /// to import GaudiHandles (ie: if one of the components has a XyzHandle<T>)
  bool    m_importGaudiHandles;

  /// buffer of generated configurables informations for the "Db" file
  /// The "Db" file is holding informations about the generated configurables
  /// This file is later one used by the PropertyProxy.py to locate
  /// Configurables and know their default values, host module,...
  stringstream m_dbBuf;

  /// Configurable customization. Contains customization for:
  ///  - Name of the module where configurable base classes are defined
  ///  - Name of the configurable base class for the Algorithm component
  ///  - Name of the configurable base class for the AlgTool component
  ///  - Name of the configurable base class for the Service component
  GaudiUtils::HashMap<std::string, std::string> m_configurable;

public:
  configGenerator( const string& pkgName,
		   const string& outputDirName ) :
    m_pkgName           ( pkgName ),
    m_outputDirName     ( outputDirName ),
    m_pyBuf             ( ),
    m_importGaudiHandles( false ),
    m_dbBuf             ( ),
    m_configurable      ( )
  {}

  /// main entry point of this class:
  ///  - iterate over all the modules (ie: library names)
  ///  - for each module extract component informations
  ///  - eventually generate the header/body/trailer python file and "Db" file
  int genConfig( const Strings_t& modules, const string& userModule );

  /// customize the Module name where configurable base classes are defined
  void setConfigurableModule( const std::string& moduleName )
  {
    m_configurable[ "Module" ] = moduleName;
  }

  /// customize the default name for configurable instances
  void setConfigurableDefaultName( const std::string& defaultName )
  {
    m_configurable[ "DefaultName" ] = defaultName;
  }

  /// customize the configurable base class for Algorithm component
  void setConfigurableAlgorithm( const std::string& cfgAlgorithm )
  {
    m_configurable[ "Algorithm" ] = cfgAlgorithm;
  }

  /// customize the configurable base class for AlgTool component
  void setConfigurableAlgTool( const std::string& cfgAlgTool )
  {
    m_configurable[ "AlgTool" ] = cfgAlgTool;
  }

  /// customize the configurable base class for AlgTool component
  void setConfigurableAuditor( const std::string& cfgAuditor )
  {
    m_configurable[ "Auditor" ] = cfgAuditor;
  }

  /// customize the configurable base class for Service component
  void setConfigurableService( const std::string& cfgService )
  {
    m_configurable[ "Service" ] = cfgService;
    m_configurable[ "ApplicationMgr" ] = cfgService;
  }

private:
  int genComponent( const std::string& libName,
                    const std::string& componentName,
                    const std::string& componentType,
                    const vector<Property*>& properties );
  void genImport( std::ostream& s, const boost::format& frmt,std::string indent);
  void genHeader( std::ostream& pyOut, std::ostream& dbOut );
  void genBody( std::ostream& pyOut,
		std::ostream& dbOut ) {
    pyOut << m_pyBuf.str() << flush;
    dbOut << m_dbBuf.str() << flush;
  }
  void genTrailer( std::ostream& pyOut,
		   std::ostream& dbOut );

  /// handle the "marshalling" of Properties
  void pythonizeValue( const Property* prop,
		       string& pvalue,
		       string& ptype );

  /// Translates a valid C++ typename into a valid python one
  void pythonizeName( string& name );
};

int createAppMgr();

//--- Command main program-----------------------------------------------------
int main ( int argc, char** argv )
//-----------------------------------------------------------------------------
{
  fs::path pwd = fs::initial_path();
  fs::path out;
  Strings_t libs;
  std::string pkgName;
  std::string userModule;

  // declare a group of options that will be allowed only on command line
  po::options_description generic("Generic options");
  generic.add_options()
    ("help,h",
     "produce this help message")
    ("package-name,p",
     po::value<string>(),
     "name of the package for which we create the configurables file")
    ("input-libraries,i",
     po::value<string>(),
     "libraries to extract the component configurables from")
    ("input-cfg,c",
     po::value<string>(),
     "path to the cfg file holding the description of the Configurable base "
     "classes, the python module holding the Configurable definitions, etc...")
    ("output-dir,o",
     po::value<string>()->default_value("../genConf"),
     "output directory for genconf files.")
    ("debug-level,d",
     po::value<int>()->default_value(0),
     "debug level")
    ("load-library,l",
     po::value< Strings_t >()->composing(),
     "preloading library")
    ("user-module,m",
     po::value<string>(),
     "user-defined module to be imported by the genConf-generated one")
    ;

  // declare a group of options that will be allowed both on command line
  // _and_ in configuration file
  po::options_description config("Configuration");
  config.add_options()
    ("configurable-module",
     po::value<string>()->default_value("AthenaCommon"),
     "Name of the module holding the configurable classes")
    ("configurable-default-name",
     po::value<string>()->default_value("Configurable.DefaultName"),
     "Default name for the configurable instance")
    ("configurable-algorithm",
     po::value<string>()->default_value("ConfigurableAlgorithm"),
     "Name of the configurable base class for Algorithm components")
    ("configurable-algtool",
     po::value<string>()->default_value("ConfigurableAlgTool"),
     "Name of the configurable base class for AlgTool components")
    ("configurable-auditor",
     po::value<string>()->default_value("ConfigurableAuditor"),
     "Name of the configurable base class for Auditor components")
    ("configurable-service",
     po::value<string>()->default_value("ConfigurableService"),
     "Name of the configurable base class for Service components")
    ;

  po::options_description cmdline_options;
  cmdline_options.add(generic).add(config);

  po::options_description config_file_options;
  config_file_options.add(config);

  po::options_description visible("Allowed options");
  visible.add(generic).add(config);

  po::variables_map vm;

  try {
    po::store( po::command_line_parser(argc, argv).
	       options(cmdline_options).run(),
	       vm );

    po::notify(vm);

    // try to read configuration from the optionally given configuration file
    if( vm.count("input-cfg") ) {
      string cfgFileName = vm["input-cfg"].as<string>();
      cfgFileName = fs::system_complete( fs::path( cfgFileName ) ).string();
      std::ifstream ifs( cfgFileName.c_str() );
      po::store( parse_config_file( ifs, config_file_options ), vm );
    }

    po::notify(vm);
  }
  catch ( po::error& err ) {
    cout << "ERR0R: error detected while parsing command options: "<< err.what() << endl;
    return EXIT_FAILURE;
  }

  //--- Process command options -----------------------------------------------
  if( vm.count("help")) {
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if( vm.count("package-name") ) {
    pkgName = vm["package-name"].as<string>();
  }
  else {
    cout << "ERROR: 'package-name' required" << endl;
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if( vm.count("user-module") ) {
    userModule = vm["user-module"].as<string>();
    cout << "INFO: will import user module " << userModule << endl; 
  }

  if( vm.count("input-libraries") ) {
    // re-shape the input arguments:
    //  - removes spurious spaces,
    //  - split into tokens.
    Strings_t inputLibs;
    {
      string tmp = vm["input-libraries"].as<string>();
      boost::trim(tmp);
      boost::split( inputLibs, tmp,
		    boost::is_any_of(" "),
		    boost::token_compress_on );
    }

    //
    libs.reserve( inputLibs.size() );
    for ( Strings_t::const_iterator iLib = inputLibs.begin();
	  iLib != inputLibs.end();
	  ++iLib ) {
      std::string lib = fs::path(*iLib).stem().string();
      if ( 0 == lib.find("lib") ) {
        lib = lib.substr(3); // For *NIX remove "lib"
      }
      // remove duplicates
      if ( !lib.empty() &&
          std::find( libs.begin(), libs.end(), lib ) == libs.end() ) {
        libs.push_back( lib );
      }
    } //> end loop over input-libraries
    if ( libs.empty() ) {
      cout << "ERROR: input component library(ies) required !\n"
           << "ERROR: 'input-libraries' argument was ["
           << vm["input-libraries"].as<string>()
           << "]"
           << endl;
      return EXIT_FAILURE;
    }
  }
  else {
    cout << "ERROR: input component library(ies) required" << endl;
    cout << visible << endl;
    return EXIT_FAILURE;
  }

  if( vm.count("output-dir") ) {
    out = fs::system_complete( fs::path( vm["output-dir"].as<string>() ) );
  }

  if ( vm.count("debug-level") ) {
    PluginService::SetDebug( vm["debug-level"].as<int>() );
  }

  if ( vm.count("load-library") ) {
    Strings_t lLib_list = vm["load-library"].as< Strings_t >();
    for (Strings_t::const_iterator lLib=lLib_list.begin();
	 lLib != lLib_list.end();
	 ++lLib) {
      // load done through ROOT::Reflex helper class
      SharedLibrary tmplib(*lLib) ;
      tmplib.Load() ;
    }
  }


  if ( !fs::exists( out ) ) {
    try {
      fs::create_directory(out);
    }
    catch ( fs::filesystem_error &err ) {
      cout << "ERR0R: error creating directory: "<< err.what() << endl;
      return EXIT_FAILURE;
    }
  }

  cout << ":::::: libraries : [ ";
  copy( libs.begin(), libs.end(), ostream_iterator<string>(cout, " ") );
  cout << "] ::::::" << endl;

  configGenerator py( pkgName, out.string() );
  py.setConfigurableModule     (vm["configurable-module"].as<string>());
  py.setConfigurableDefaultName(vm["configurable-default-name"].as<string>());
  py.setConfigurableAlgorithm  (vm["configurable-algorithm"].as<string>());
  py.setConfigurableAlgTool    (vm["configurable-algtool"].as<string>());
  py.setConfigurableAuditor    (vm["configurable-auditor"].as<string>());
  py.setConfigurableService    (vm["configurable-service"].as<string>());

  int sc = EXIT_FAILURE;
  try {
    sc = py.genConfig( libs, userModule );
  }
  catch ( exception& e ) {
    cout << "ERROR: Could not generate Configurable(s) !\n"
         << "ERROR: Got exception: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  if ( EXIT_SUCCESS == sc ) {
    // create an empty __init__.py file in the output dir
    fstream initPy( ( out / fs::path( "__init__.py" ) ).string().c_str(),
        std::ios_base::out|std::ios_base::trunc );
    initPy << "## Hook for " << pkgName << " genConf module\n" << flush;
  }

  cout << ":::::: libraries : [ ";
  copy( libs.begin(), libs.end(), ostream_iterator<string>(cout, " ") );
  cout << "] :::::: [DONE]" << endl;

  return sc;
}

/// Given a Reflex::Member object, return the id for the configurable (name or id, if it is a string).
/// non-string ids are used for the persistency (DataObjects)
inline std::string getId(const Member & m) {
      return (m.Properties().HasProperty("id") && (m.Properties().PropertyValue("id").TypeInfo() == typeid(std::string))) ?
             m.Properties().PropertyAsString("id") :
             m.Properties().PropertyAsString("name") ;
}

template <class T>
IProperty *makeInstance(const Member &member, const vector<void*> &args)
{
  Object dummy;
  T* obj;
#if ROOT_VERSION_CODE < ROOT_VERSION(5,21,6)
  obj = static_cast<T*>(member.Invoke(dummy,args).Address());
#else
  member.Invoke(dummy,obj,args);
#endif
  return dynamic_cast<IProperty*>(obj);
}


//-----------------------------------------------------------------------------
int configGenerator::genConfig( const Strings_t& libs, const string& userModule )
//-----------------------------------------------------------------------------
{
  //--- Disable checking StatusCode -------------------------------------------
  StatusCode::disableChecking();

  const Strings_t::const_iterator endLib = libs.end();

  const std::string gaudiSvc = "GaudiCoreSvc";
  const bool isGaudiSvc = ( std::find( libs.begin(), endLib, gaudiSvc ) != endLib );

  //--- Instantiate ApplicationMgr --------------------------------------------
  if ( !isGaudiSvc && createAppMgr() ) {
    cout << "ERROR: ApplicationMgr can not be created. Check environment" << endl;
    return EXIT_FAILURE;
  }

  //--- Iterate over component factories --------------------------------------
  Scope factories = Scope::ByName(PLUGINSVC_FACTORY_NS);
  if ( !factories ) {
    cout << "ERROR: No PluginSvc factory namespace could be found" << endl;
    return EXIT_FAILURE;
  }

  ISvcLocator* svcLoc = Gaudi::svcLocator();
  IInterface*  dummySvc = new Service( "DummySvc", svcLoc );
  dummySvc->addRef();

  bool allGood = true;

  // iterate over all the requested libraries
  for ( Strings_t::const_iterator iLib=libs.begin(); iLib != endLib; ++iLib ) {

    std::cout << ":::: processing library: " << *iLib << "..." << std::endl;

    // reset state
    m_importGaudiHandles = false;
    m_pyBuf.str("");
    m_dbBuf.str("");

    // Scan the pluginSvc namespace and store the "background" of already
    // alive components, so we can extract our signal later on
    set<string> bkgNames;
    if ( !isGaudiSvc ) {
      for ( Member_Iterator it = factories.FunctionMember_Begin();
            it != factories.FunctionMember_End(); ++it ) {
        string ident = getId(*it);
        if ( PluginService::Debug() > 0 ) {
          cout << "::: " << ident << endl;
        }
        bkgNames.insert( ident );
      }
    }
    const set<string>::const_iterator bkgNamesEnd = bkgNames.end();

    //--- Load component library ----------------------------------------------
    System::ImageHandle handle;
    unsigned long err = System::loadDynamicLib( *iLib, &handle );
    if ( err != 1 ) {
      cout << "ERROR: " << System::getLastErrorString() << endl;
      allGood = false;
      continue;
    }

    for ( Member_Iterator it = factories.FunctionMember_Begin();
          it != factories.FunctionMember_End();
          ++it ) {
      const string ident = getId(*it);
      if ( bkgNames.find(ident) != bkgNamesEnd ) {
        if ( PluginService::Debug() > 0 ) {
          cout << "\t==> skipping [" << ident << "]..." << endl;
        }
        continue;
      }

      // Atlas contributed code (patch #1247)
      // Skip the generation of configurables if the component does not come
      // from the same library we are processing (i.e. we found a symbol that
      // is coming from a library loaded by the linker).
      // Windows implementation is empty.
      if ( !DsoUtils::inDso( *it, DsoUtils::libNativeName(*iLib) ) ) {
        cout << "WARNING: library [" << *iLib << "] requested factory "
        << "from another library ["
        << DsoUtils::dsoName(*it) << "]"
        << " ==> [" << ident << "] !!"
        << endl;
        continue;
      }

      const string rtype = it->TypeOf().ReturnType().Name();
      string type;
      bool known = true;
      if      ( ident == "ApplicationMgr" ) type = "ApplicationMgr";
      else if ( rtype == "IInterface*" )    type = "IInterface";
      else if ( rtype == "IAlgorithm*" )    type = "Algorithm";
      else if ( rtype == "IService*" )      type = "Service";
      else if ( rtype == "IAlgTool*" )      type = "AlgTool";
      else if ( rtype == "IAuditor*" )      type = "Auditor";
      else if ( rtype == "IConverter*" )    type = "Converter";
      else if ( rtype == "DataObject*" )    type = "DataObject";
      else                                  type = "Unknown", known = false;
      string name = ident;
      // handle possible problems with templated components
      boost::trim(name);

      cout << " - component: " << name << endl;

      if ( type == "IInterface" ) {
        /// not enough information...
        /// skip it
        continue;
      }

      if ( type == "Converter" || type == "DataObject" ) {
        /// no Properties, so don't bother create Configurables...
        continue;
      }

      //if ( type == "ApplicationMgr" ) {
      ///  @FIXME: no Configurable for this component. yet...
      ///  continue;
      //}

      if ( !known ) {
        cout << "WARNING: Unknown (return) type [" << rtype << "] !!\n"
             << "WARNING: component [" << ident << "] is skipped !"
             << endl;
        allGood = false;
        continue;
      }

      string cname = "DefaultName";
      vector<void*>  args;
      args.reserve( 3 );
      if ( type == "AlgTool" ) {
        args.resize( 3 );
        args[0] = &cname;
        args[1] = &type;
        args[2] = dummySvc;
      }
      else {
        args.resize( 2 );
        args[0] = &cname;
        args[1] = svcLoc;
      }
      IProperty* prop = 0;
      try {
        if ( type == "Algorithm" ) {
          prop = makeInstance<IAlgorithm>(*it,args);
        }
        else if ( type == "Service") {
          prop = makeInstance<IService>(*it,args);
        }
        else if ( type == "AlgTool") {
          prop = makeInstance<IAlgTool>(*it,args);
        }
        else if ( type == "Auditor") {
          prop = makeInstance<IAuditor>(*it,args);
        }
        else if ( type == "ApplicationMgr") {
          //svcLoc->queryInterface(IProperty::interfaceID(), pp_cast<void>(&prop));
          svcLoc->queryInterface(IProperty::interfaceID(), (void**)(&prop));
        }
        else {
          prop = makeInstance<IInterface>(*it,args);
        }
      }
      catch ( exception& e ) {
        cout << "ERROR: Error instantiating " << name
             << " from " << *iLib << endl;
        cout << "ERROR: Got exception: " << e.what() << endl;
        allGood = false;
        continue;
      }
      catch ( ... ) {
        cout << "ERROR: Error instantiating " << name
             << " from " << *iLib << endl;
        allGood = false;
        continue;
      }
      if( prop ) {
        if (genComponent( *iLib, name, type, prop->getProperties() )) {
          allGood = false;
        }
        prop->release();
      } else {
        cout << "ERROR: could not cast IInterface* object to an IProperty* !\n"
             << "ERROR: return type from PluginSvc is [" << rtype << "]...\n"
             << "ERROR: NO Configurable will be generated for ["
             << name << "] !"
             << endl;
        allGood = false;
      }
    } //> end loop over factories

    ///
    /// write-out files for this library
    ///
    const std::string pyName = ( fs::path(m_outputDirName) /
                  fs::path(*iLib+"Conf.py") ).string();
    const std::string dbName = ( fs::path(m_outputDirName) /
                  fs::path(*iLib+"_confDb.py") ).string();

    std::fstream py( pyName.c_str(),
              std::ios_base::out|std::ios_base::trunc );
    std::fstream db( dbName.c_str(),
              std::ios_base::out|std::ios_base::trunc );

    genHeader ( py, db );
    if (!userModule.empty()) 
      py << "from " << userModule << " import *" <<endl;
    genBody   ( py, db );
    genTrailer( py, db );

  } //> end loop over libraries

  dummySvc->release();
  dummySvc = 0;

  return allGood ? EXIT_SUCCESS : EXIT_FAILURE;
}

void configGenerator::genImport( std::ostream& s,
                                 const boost::format& frmt,
                                 std::string indent = ""){

  std::string::size_type pos = 0, nxtpos = 0;
  std::string mod;

  while ( std::string::npos != pos ){
    // find end of module name
    nxtpos = m_configurable["Module"].find_first_of(',',pos);

    // Prepare import string
    mod = m_configurable["Module"].substr(pos,nxtpos-pos);
    std::ostringstream import;
    import << boost::format(frmt) % mod;

    // append a normal import or a try/except enclosed one depending
    // on availability of a fall-back module (next in the list)
    if ( std::string::npos == nxtpos ) {
      // last possible module
      s << indent << import.str() << "\n" << flush;
      pos = std::string::npos;
    } else {
      // we have a fallback for this
      s << indent << "try:\n"
        << indent << py_tab << import.str() << "\n"
        << indent << "except ImportError:\n"
        << flush;
      pos = nxtpos+1;
    }
    // increase indentation level for next iteration
    indent += py_tab;
  }
}

//-----------------------------------------------------------------------------
void configGenerator::genHeader( std::ostream& py,
				 std::ostream& db )
//-----------------------------------------------------------------------------
{
  // python file part
  std::string now = Gaudi::Time::current().format(true);
  py << "#" << now //<< "\n"
     << "\"\"\"Automatically generated. DO NOT EDIT please\"\"\"\n";
  if ( m_importGaudiHandles ) {
    py << "from GaudiKernel.GaudiHandles import *\n";
  }

  genImport(py,boost::format("from %1%.Configurable import *"));

  // db file part
  db << "##  -*- python -*-  \n"
     << "# db file automatically generated by genconf on: "
     << now; // << "\n";
  db << "## insulates outside world against anything bad that could happen\n"
     << "## also prevents global scope pollution\n"
     << "def _fillCfgDb():\n";
  genImport(db,boost::format("from %1%.ConfigurableDb import CfgDb"),py_tab);

  db << "\n"
     << py_tab << "# get a handle on the repository of Configurables\n"
     << py_tab << "cfgDb = CfgDb()\n"
     << "\n"
     << py_tab << "# populate the repository with informations on Configurables \n"
     << "\n"
     << flush;
}
//-----------------------------------------------------------------------------
void configGenerator::genTrailer( std::ostream& /*py*/,
				  std::ostream& db )
//-----------------------------------------------------------------------------
{
  // db file part
  db << py_tab << "return #_fillCfgDb\n"
     << "# fill cfgDb at module import...\n"
     << "try:\n"
     << py_tab << "_fillCfgDb()\n"
     << py_tab << "#house cleaning...\n"
     << py_tab << "del _fillCfgDb\n"
     << "except Exception,err:\n"
     << py_tab << "print \"Py:ConfigurableDb   ERROR Problem with [%s] content!"
               << "\" % __name__\n"
     << py_tab << "print \"Py:ConfigurableDb   ERROR\",err\n"
     << py_tab << "print \"Py:ConfigurableDb   ERROR   ==> culprit is package ["
               << m_pkgName << "] !\"\n"
     << std::flush;
}

//-----------------------------------------------------------------------------
int
configGenerator::genComponent( const std::string& libName,
                               const std::string& componentName,
                               const std::string& componentType,
                               const vector<Property*>& properties )
//-----------------------------------------------------------------------------
{
  string cname = componentName;
  pythonizeName(cname);

  typedef GaudiUtils::HashMap<std::string, std::string> PropertyDoc_t;
  PropertyDoc_t propDoc;

  m_pyBuf << "\n";
  m_pyBuf << "class " << cname
          << "( " << m_configurable[componentType] << " ) :"
          << "\n";
  m_pyBuf << "  __slots__ = { \n";
  for ( vector<Property*>::const_iterator it = properties.begin() ;
        it != properties.end(); ++it ) {

    const string pname  = (*it)->name();
    // Validate property name (it must be a valid Python identifier)
    if (!boost::regex_match(pname, pythonIdentifier)) {
      std::cout << "ERROR: invalid property name \"" << pname
                << "\" in component " << cname
                << " (invalid Python identifier)" << std::endl;
      // try to make the buffer at least more or less valid python code.
      m_pyBuf << " #ERROR-invalid identifier '" << pname << "'\n"
              << "  }\n";
      return 1;
    }

    string pvalue, ptype;
    pythonizeValue( (*it), pvalue, ptype );
    m_pyBuf << "    '" << pname << "' : " << pvalue <<", # " << ptype << "\n";

    if ( (*it)->documentation() != "none" ) {
      propDoc[pname] = (*it)->documentation();
    }

  }
  m_pyBuf << "  }\n";
  m_pyBuf << "  _propertyDocDct = { \n";
  for ( PropertyDoc_t::const_iterator iProp = propDoc.begin();
        iProp != propDoc.end();
        ++iProp ) {
    m_pyBuf << std::setw(5)
            << "'"       << iProp->first  << "' : "
            << "\"\"\" "  << iProp->second << " \"\"\",\n";
  }
  m_pyBuf << "  }\n";

  m_pyBuf
    << "  def __init__(self, name = " << m_configurable["DefaultName"]
                                      << ", **kwargs):\n"
    << "      super(" << cname << ", self).__init__(name)\n"
    << "      for n,v in kwargs.items():\n"
    << "         setattr(self, n, v)\n"
    << "  def getDlls( self ):\n"
    << "      return '" << libName << "'\n"
    << "  def getType( self ):\n"
    << "      return '" << componentName  << "'\n"
    << "  pass # class " << cname << "\n"
    << flush;

  // name of the auto-generated module
  const string pyName = ( fs::path(m_outputDirName) /
			  fs::path(libName+"Conf.py") ).string();
  const string modName = fs::basename( fs::path( pyName ).leaf() );

  // now the db part
  m_dbBuf
    << py_tab << "cfgDb.add( configurable = '" << cname << "',\n"
    << py_tab << "           package = '" << m_pkgName << "',\n"
    << py_tab << "           module  = '" << m_pkgName << "." << modName << "',\n"
    << py_tab << "           lib     = '" << libName << "' )\n"
    << flush;

  return 0;
}
//-----------------------------------------------------------------------------
void configGenerator::pythonizeName( string& name )
//-----------------------------------------------------------------------------
{
  static string  in("<>&*,: ().");
  static string out("__rp__s___");
  for ( string::iterator i = name.begin(); i != name.end(); ++i ) {
    if ( in.find(*i) != string::npos ) *i = out[in.find(*i)];
  }
}

//-----------------------------------------------------------------------------
void configGenerator::pythonizeValue( const Property* p,
                                      string& pvalue, string& ptype )
//-----------------------------------------------------------------------------
{
  const std::string cvalue = p->toString();
  const type_info& ti = *p->type_info();
  if ( ti == typeid(bool) ) {
    pvalue = ( cvalue == "0" || cvalue == "False" || cvalue == "false" )
           ? "False"
           : "True";
    ptype  = "bool";
  }
  else if ( ti == typeid(char)  || ti == typeid(signed char)
            || ti == typeid(unsigned char)  ||
            ti == typeid(short) || ti == typeid(unsigned short) ||
            ti == typeid(int)   || ti == typeid(unsigned int)   ||
            ti == typeid(long)  || ti == typeid(unsigned long) ) {
    pvalue = cvalue;
    ptype  = "int";
  }
  else if ( ti == typeid(long long) || ti == typeid(unsigned long long) ) {
    pvalue = cvalue + "L";
    ptype  = "long";
  }
  else if ( ti == typeid(float) || ti == typeid(double) ) {
    // forces python to handle this as a float: put a dot in there...
    pvalue = boost::to_lower_copy(cvalue);
    if ( pvalue == "nan" ) {
      pvalue = "float('nan')";
      std::cout << "WARNING: default value for ["
                << p->name() << "] is NaN !!"
                << std::endl;
    } else if ( std::string::npos == pvalue.find(".") &&
                std::string::npos == pvalue.find("e")  ) {
      pvalue = cvalue + ".0";
    }
    ptype  = "float";
  }
  else if ( ti == typeid(string) ) {

    pvalue = "'"+cvalue+"'";
    ptype  = "str";
  }
  else if ( ti == typeid(GaudiHandleBase) ) {
    m_importGaudiHandles = true;
    const GaudiHandleProperty& hdl
      = dynamic_cast<const GaudiHandleProperty&>(*p);
    const GaudiHandleBase&     base = hdl.value();

    pvalue = base.pythonRepr();
    ptype  = "GaudiHandle";
  }
  else if ( ti == typeid(GaudiHandleArrayBase) ) {
    m_importGaudiHandles = true;
    const GaudiHandleArrayProperty& hdl
      = dynamic_cast<const GaudiHandleArrayProperty&>(*p);
    const GaudiHandleArrayBase&     base = hdl.value();

    pvalue = base.pythonRepr();
    ptype  = "GaudiHandleArray";
  }
  else {
    std::ostringstream v_str;
    v_str.setf(std::ios::fixed); // to correctly display floats
    p->toStream(v_str);
    pvalue = v_str.str();
    ptype  = "list";
  }
}

//-----------------------------------------------------------------------------
int createAppMgr()
//-----------------------------------------------------------------------------
{
  IInterface* iface = Gaudi::createApplicationMgr();
  SmartIF<IProperty> propMgr ( iface );
  SmartIF<IAppMgrUI> appUI  ( iface );

  if ( propMgr.isValid() && appUI.isValid() ) {
    propMgr->setProperty( "JobOptionsType", "NONE" );  // No job options
    propMgr->setProperty( "AppName", "");              // No initial printout message
    propMgr->setProperty( "OutputLevel", "7");         // No other printout messages
    appUI->configure();
    return EXIT_SUCCESS;
  }
  else {
    return EXIT_FAILURE;
  }
}
